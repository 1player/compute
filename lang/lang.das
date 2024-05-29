// Self-test

// Assertions
Runtime.assertEquals(false == false, true)
Runtime.assertEquals(1 == 1, true)
Runtime.assertEquals(1 == 2, false)

// Arithmetic and operator precedence
Runtime.assertEquals(5 * 4 + 3, 23)
Runtime.assertEquals(5 + 4 * 3, 17)

// Assignments and block scoping
selfTest__x := 137
{
    selfTest__y := 82
    Runtime.assertEquals(selfTest__x + selfTest__y, 137 + 82)
}


// Strings
Runtime.assertEquals("foo".concat("bar"), "foobar")
Runtime.assertEquals("foo".concat("bar") === "foobar", false)


"Self-test passed.".println()
