def factorial(n: int) -> int:
  if n == 0:
    return 1
  else:
    return n * factorial(n - 1)


def is_prime(n: int) -> bool:
  if n <= 1:
    return False
  i: int = 2
  for i in range(2, int(n ** 0.5) + 1):
    if n % i == 0:
      return False
  return True


def fibonacci(n: int) -> list[int]:
  sequence: list[int] = [0, 1]
  while len(sequence) < n:
    next_num: int = sequence[-1] + sequence[-2]
    sequence.append(next_num)
  return sequence


def count_words(string: str) -> int:
  words: list[str] = string.split()
  return len(words)


def main():
  num: int = 5
  print("Factorial of", num, "is", factorial(num))

  if is_prime(num):
    print(num, "is a prime number")
  else:
    print(num, "is not a prime number")

  print("Fibonacci sequence up to", num, "is", fibonacci(num))

  string: str = "Hello, World!"
  print("String:", string)
  print("Number of words in the string:", count_words(string))


if __name__ == "__main__":
  main()
