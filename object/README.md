## Design decisions

* Based on [Ian Piumarta's "Open, extensible object models"](https://www.piumarta.com/software/id-objmodel/objmodel2.pdf) with some differences listed below.
* Methods (closures) are not late bound to allow for increased performance. Once determined the address of a method, an optimizing compiler can simply transform the message send operation into a CPU "call" instruction.
* The list of methods (closures) available on any given object is also defined at "compile time", while the implementation of methods themselves can be changed at run-time (through the use of a jump table).
* No inheritance, no `parent` link, but composition through traits.

### Memory model

* All objects in memory use the least-significant bit as a tag: if 0, it's a pointer to an object; if 1, it's a native integer (63-bits on a 64-bit architecture). The word `0` represented `null`, i.e. a pointer to no object.
* NativeInteger cannot automatically be promoted to a class of arbitrary-size integers because otherwise we'd have to check if the returned type of any arithmetic operation is a NativeInteger or a "big" integer. For this reason, NativeIntegers default to saturating arithmetic.

### Typing

* Since method addresses can be cached, all types must be known at "compile time".
* Objects *are* types, and viceversa.
* `x is y` if `x.meta == y.meta`
* Actors are objects with 2 methods: call and cast
* The caller is responsible for doing type checking, so it can be optimized out by a sufficiently advanced language that can guarantee a method is being called with the correct arguments.

### Other ideas

* Not everything has to be defined "within" the language. For example, take Smalltalk's implementation of control flow with the ifTrue:ifFalse: message. In DAS//, some concepts are allowed to be part of the "meta environment" to allow for more advanced optimizations. For example, Booleans and control structures map natively to how CPUs work, so if we don't try to pigeonhole these into messages, it is much easier to optimize them out into native CPU instructions (`test`, `jnz`, etc.)
