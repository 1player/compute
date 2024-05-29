// Self-test

// Arithmetic and operator precedence
Runtime.assertEquals(5 * 4 + 3, 23)
Runtime.assertEquals(5 + 4 * 3, 17)

// Assignments and block scoping
selfTest__x := 137
{
    selfTest__y := 82
    Runtime.assertEquals(selfTest__x + selfTest__y, 137 + 82)
}


"Self-test passed.".println()
