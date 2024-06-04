fizzbuzz := func (n) {
  if n % 15 == 0 {
    "fizzbuzz".println()
  } else if n % 3 == 0 {
    "fizz".println()
  } else if n % 5 == 0 {
    "buzz".println()
  } else {
    n.inspect().println()
  }
}

n := 1
loop n <= 100 {
  scope.fizzbuzz(n)
  n = n + 1
}
