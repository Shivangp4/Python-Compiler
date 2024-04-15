#include <string>
#include <vector>
#include <stack>
#include "codegen.h"
#include "symtable.h"
#include "utils.h"

extern std::vector<std::vector<std::string>> ac3_code; // 3AC instructions (op, arg1, arg2, result)

extern std::string get_3ac_str(const std::vector<std::string> &ac3_line);

std::vector<std::string> x86_code;
std::vector<std::string> arg_regs = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};

std::string cur_label;

std::stack<std::string> arg_stack;

void gen_x86_code() {
  for (const auto &ac3_line : ac3_code) {
    gen_x86_line_code(ac3_line);
  }
}

void gen_x86_line_code(const std::vector<std::string> &ac3_line) {
  std::string op = ac3_line[0];
  std::string arg1 = ac3_line[1];
  std::string arg2 = ac3_line[2];
  std::string result = ac3_line[3];

  if (op == "=") {
    // result = arg1
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    std::string arg1_addr = get_addr(arg1);
    std::string result_addr = get_addr(result);

    x86_code.push_back("\tmovl\t" + arg1_addr + ", " + "%eax");
    x86_code.push_back("\tmovl\t%eax, " + result_addr);
    x86_code.push_back("");
    return;
  }

  if (result == "goto") {
    // goto arg1
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tjmp\t" + arg1);
    x86_code.push_back("");
    return;
  }

  if (arg1 == "goto") {
    // if op goto arg2
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    int operator_pos = op.find("<");
    if(operator_pos != std::string::npos){
      std::string lhs = op.substr(0, operator_pos);
      std::string rhs = op.substr(operator_pos + 1);
      x86_code.push_back("\tmovl\t" + get_addr(lhs) + ", %eax");
      x86_code.push_back("\tcmpl\t" + get_addr(rhs) + ", %eax");
      x86_code.push_back("\tjl\t" + arg2);
      x86_code.push_back("");
      return;
    }
    x86_code.push_back("\tmovl\t" + get_addr(op) + ", %eax");
    x86_code.push_back("\tcmpl\t$0, %eax");
    x86_code.push_back("\tjg\t" + arg2);
    x86_code.push_back("");
    return;
  }
  
  if (arg1 == ":" && op.empty()) {
    // result:
    cur_label = result;

    x86_code.push_back(result + ":");
    return;
  }
  
  if (result == "beginfunc") {
    // beginfunc
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    func_scope = true;
    std::string func_name = cur_label;
    cur_func_symtable_ptr = lookup_func(func_name);

    x86_code.push_back("\tpushq\t%rbp");
    x86_code.push_back("\tmovq\t%rsp, %rbp");

    int offset = align_offset(cur_func_symtable_ptr->offset);
    if (offset) {
      x86_code.push_back("\tsubq\t$" + std::to_string(offset) + ", %rsp");
    }

    store_args(func_name);
    x86_code.push_back("");
    return;
  }
  
  if (result == "endfunc") {
    // endfunc
    func_scope = false;
    return;
  }
  
  if (op == "push") {
    // push arg1
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    std::string arg1_addr = get_addr(arg1);

    x86_code.push_back("\tmovl\t" + arg1_addr + ", " + "%eax");
    x86_code.push_back("");
    return;
  }
  
  if (op == "return") {
    // return
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tleave");
    x86_code.push_back("\tret");
    x86_code.push_back("");
    return;
  }
  
  if (op == "popparam" && arg1 == "return_val") {
    // result = popparam
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  
  if (op == "param") {
    // param arg1
    arg_stack.push(arg1);
    return;
  }
  
  if (result == "call") {
    // call arg1, op
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    int num_args = std::stoi(op);
    pass_args(num_args);

    x86_code.push_back("\tcall\t" + arg1);

    int num_regs = arg_regs.size();
    if (num_args > num_regs) {
      int size = 8 * (num_args - num_regs);
      x86_code.push_back("\taddq\t$" + std::to_string(size) + ", %rsp");
    }

    x86_code.push_back("");
    return;
  }
  
  if (arg2.empty()) {
    // result = op arg1
    if(op == "-"){
      x86_code.push_back("\t# " + get_3ac_str(ac3_line));
      x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
      x86_code.push_back("\tnegl\t%eax");
      x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
      x86_code.push_back("");
      return;
    }
    if(op == "not"){
      x86_code.push_back("\t# " + get_3ac_str(ac3_line));
      x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
      x86_code.push_back("\tcmpl\t$0, %eax");
      x86_code.push_back("\tsete\t%al");
      x86_code.push_back("\tmovzbl\t%al, %eax");
      x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
      x86_code.push_back("");
      return;
    }
    if(op == "~"){
      x86_code.push_back("\t# " + get_3ac_str(ac3_line));
      x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
      x86_code.push_back("\tnotl\t%eax");
      x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
      x86_code.push_back("");
      return;
    }
    return;
  }

  // result = arg1 op arg2
  if(op == "+"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
    x86_code.push_back("\taddl\t" + get_addr(arg2) + ", %eax");
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == "-"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
    x86_code.push_back("\tsubl\t" + get_addr(arg2) + ", %eax");
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == "*"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
    x86_code.push_back("\timull\t" + get_addr(arg2) + ", %eax");
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == "/"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
    x86_code.push_back("\tcltd");
    x86_code.push_back("\tidivl\t" + get_addr(arg2));
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == "**"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t$1 , %eax");
    x86_code.push_back("\tmovl\t" + get_addr(arg2) + ", %edx");
    std::string newlabel1 = new_label();
    x86_code.push_back(newlabel1 + ":");
    x86_code.push_back("\tcmpl\t$0, %edx");
    std::string newlabel2 = new_label();
    x86_code.push_back("\tje\t" + newlabel2);
    x86_code.push_back("\timull\t" + get_addr(arg1) + ", %eax");
    x86_code.push_back("\tdecl\t%edx");
    x86_code.push_back("\tjmp\t" + newlabel1);
    x86_code.push_back(newlabel2 + ":");
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == "%"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
    x86_code.push_back("\tcltd");
    x86_code.push_back("\tidivl\t" + get_addr(arg2));
    x86_code.push_back("\tmovl\t%edx, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == "&"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
    x86_code.push_back("\tandl\t" + get_addr(arg2) + ", %eax");
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == "|"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
    x86_code.push_back("\torl\t" + get_addr(arg2) + ", %eax");
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == "^"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
    x86_code.push_back("\txorl\t" + get_addr(arg2) + ", %eax");
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == "and"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    std::string newlabel1 = new_label();
    x86_code.push_back("\tcmpl\t$0, " + get_addr(arg1));
    x86_code.push_back("\tje\t" + newlabel1);
    x86_code.push_back("\tcmpl\t$0, " + get_addr(arg2)); 
    x86_code.push_back("\tje\t" + newlabel1);
    x86_code.push_back("\tmovl\t$1, %eax");
    std::string newlabel2 = new_label();
    x86_code.push_back("\tjmp\t" + newlabel2);
    x86_code.push_back(newlabel1 + ":");
    x86_code.push_back("\tmovl\t$0, %eax");
    x86_code.push_back(newlabel2 + ":");
    x86_code.push_back("\tmovzbl\t%al, %eax");
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == "or"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    std::string newlabel1 = new_label();
    x86_code.push_back("\tcmpl\t$0, " + get_addr(arg1));
    x86_code.push_back("\tjne\t" + newlabel1);
    x86_code.push_back("\tcmpl\t$0, " + get_addr(arg2)); 
    std::string newlabel2 = new_label();
    x86_code.push_back("\tje\t" + newlabel2);
    x86_code.push_back(newlabel1 + ":");
    x86_code.push_back("\tmovl\t$1, %eax");
    std::string newlabel3 = new_label();
    x86_code.push_back("\tjmp\t" + newlabel3);
    x86_code.push_back(newlabel2 + ":");
    x86_code.push_back("\tmovl\t$0, %eax");
    x86_code.push_back(newlabel3 + ":");
    x86_code.push_back("\tmovzbl\t%al, %eax");
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == "=="){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
    x86_code.push_back("\tcmpl\t" + get_addr(arg2) + ", %eax");
    x86_code.push_back("\tsete\t%al");
    x86_code.push_back("\tmovzbl\t%al, %eax");
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == "!="){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
    x86_code.push_back("\tcmpl\t" + get_addr(arg2) + ", %eax");
    x86_code.push_back("\tsetne\t%al");
    x86_code.push_back("\tmovzbl\t%al, %eax");
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == "<"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
    x86_code.push_back("\tcmpl\t" + get_addr(arg2) + ", %eax");
    x86_code.push_back("\tsetl\t%al");
    x86_code.push_back("\tmovzbl\t%al, %eax");
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == ">"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
    x86_code.push_back("\tcmpl\t" + get_addr(arg2) + ", %eax");
    x86_code.push_back("\tsetg\t%al");
    x86_code.push_back("\tmovzbl\t%al, %eax");
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == "<<"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t" + get_addr(arg2) + ", %eax"); 
    x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %edx");
    x86_code.push_back("\tmovl\t%eax, %ecx");
    x86_code.push_back("\tsall\t%cl, %edx");
    x86_code.push_back("\tmovl\t%edx, %eax");
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == ">>"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t" + get_addr(arg2) + ", %eax"); 
    x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %edx");
    x86_code.push_back("\tmovl\t%eax, %ecx");
    x86_code.push_back("\tsarl\t%cl, %edx");
    x86_code.push_back("\tmovl\t%edx, %eax");
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
  if(op == "//"){
    x86_code.push_back("\t# " + get_3ac_str(ac3_line));
    x86_code.push_back("\tmovl\t" + get_addr(arg1) + ", %eax");
    x86_code.push_back("\tcltd");
    x86_code.push_back("\tidivl\t" + get_addr(arg2));
    x86_code.push_back("\tmovl\t%eax, " + get_addr(result));
    x86_code.push_back("");
    return;
  }
}

std::string get_addr(const std::string &name) {
  if (is_int_literal(name)) {
    return "$" + name;
  }

  if (name == "True") {
    return "$1";
  }

  if (name == "False") {
    return "$0";
  }

  int brace = name.find('[');
  if (brace != std::string::npos) {
    // array access
    std::string arr = name.substr(0, brace);
    std::string offset = name.substr(brace + 1, name.size() - brace - 2);
    std::string arr_addr = get_addr(arr);
    if (is_int_literal(offset)) {
      x86_code.push_back("\tmovq\t" + arr_addr + ", %rax");
      x86_code.push_back("\tleaq\t" + offset + "(%rax), %rdx");
      return "(%rdx)";
    }

    std::string offset_addr = get_addr(offset);
    x86_code.push_back("\tmovl\t" + offset_addr + ", %eax");
    x86_code.push_back("\tcltq");
    x86_code.push_back("\tmovq\t%rax, %rdx");
    x86_code.push_back("\tmovq\t" + arr_addr + ", %rax");
    x86_code.push_back("\taddq\t%rax, %rdx");
    return "(%rdx)";
  }

  if (name == "\"__main__\"") {
    // TODO
    return name;
  }
  
  symtable_entry entry = lookup_var(name);
  int offset = entry.offset + entry.size;
  return "-" + std::to_string(offset) + "(%rbp)";
}

int align_offset(int offset) {
  int rem = offset % 16;
  if (rem) {
    offset += 16 - rem;
  }

  return offset;
}

void store_args(const std::string &func_name) {
  local_symtable *func_symtable_ptr = lookup_func(func_name);
  int num_args = func_symtable_ptr->params.size();
  int num_regs = arg_regs.size();
  for (int i = 0; i < num_args; i++) {
    std::string param = func_symtable_ptr->params[i].first;
    if (i < num_regs) {
      x86_code.push_back("\tmovl\t" + arg_regs[i] + ", " + get_addr(param));
      continue;
    }

    int offset = 16 + 8 * (i - num_regs);
    x86_code.push_back("\tmovl\t" + std::to_string(offset) + "(%rbp), " + get_addr(param));
  }
}

void pass_args(int num_args) {
  int num_regs = arg_regs.size();
  if (num_args > num_regs) {
    for (int i = 0; i < num_args - num_regs; i++) {
      std::string arg = arg_stack.top();
      x86_code.push_back("\tpushq\t" + get_addr(arg));
      arg_stack.pop();
    }

    num_args = num_regs;
  }
  
  for (int i = 0; i < num_args; i++) {
    std::string arg = arg_stack.top();
    std::string arg_reg = arg_regs[num_regs - i - 1];
    x86_code.push_back("\tmovl\t" + get_addr(arg) + ", " + arg_reg);
    arg_stack.pop();
  }
}
