// Self-test

// Assertions
Runtime.assert(false == false, true)
Runtime.assert(1 == 1, true)
Runtime.assert(1 == 2, false)

// Arithmetic and operator precedence
Runtime.assert(5 * 4 + 3, 23)
Runtime.assert(5 + 4 * 3, 17)

// Assignments and block scoping
selfTest__x := 137
{
    selfTest__y := 82
    Runtime.assert(selfTest__x + selfTest__y, 137 + 82)
}


// Strings
Runtime.assert("foo".concat("bar"), "foobar")
Runtime.assert("foo".concat("bar") === "foobar", false)


// Closures (WIP)
add1 := func(n) { n + 1 }
Runtime.assert(scope.add1(5), 6)

"Self-test passed.".println()
