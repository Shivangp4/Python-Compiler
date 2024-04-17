class A:

  def __init__(self):
    self.x: int = 1
    self.y: int = 3


class B(A):

  def __init__(self):
    self.x = 1
    self.y = 3
    self.z: int = 5


def main():
  a: A = A()
  b: B = B()

  print("a:")
  print(a.x)
  print(a.y)
  print("")
  
  print("b:")
  print(b.x)
  print(b.y)
  print(b.z)


if __name__ == '__main__':
  main()
