## Design decisions

* Based on Ian Piumarta's "Open, extensible object models"
* Methods (closures) are not late bound to allow for increased performance. Once determined the address of a method, an optimizing compiler can simply transform the message send operation into a "call" CPU instruction.
* The list of methods (closures) available on any given object is also defined at "compile time", while the implementation of methods themselves can be changed at run-time (through the use of a jump table).
* No inheritance, no `parent` link, but composition through traits.

## Corollaries

* Since method addresses can be cached, all types must be known at "compile time".
* Cannot implement NativeInteger promotion because otherwise we'd have to check if the returned type is a NativeInteger or a "big" integer (i.e. we would not be able to replace NativeInteger messages into CPU arithmetic instructions), so we default to satiurating arithmetic on NativeIntegers.
