fact := func(n) {
  if n == 1 {
    1
  } else {
    n * fact(n - 1)
  }
}

fact(6).inspect.println
