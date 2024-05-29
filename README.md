# DAS//compute

This repository is the forge vat into which *DAS//compute*, a second-generation computing system, is being grown.

It is based upon three main concepts:
- An architecture-independent prototype-based language into which most of the system is written in.
- A message-passing, typed object model to represent the semantics of the system.
- An actor-based kernel running hosted on bare metal, or on top of an existing operating system, to run and exchange messages between actors.

## Language design brainstorming

* Inspired by [Ian Piumarta's "Open, extensible object models"](https://www.piumarta.com/software/id-objmodel/objmodel2.pdf) but with a focus on prototypes.

### Memory model

* All words in memory use the least-significant bit as a tag: if LSB==0, the word points to an object; if LSB==1, the word refers to a native integer (i.e. 63-bits on a 64-bit architecture). The word `0` represents `null`, i.e. a pointer to no object.

### Other ideas

* Not everything has to be defined from "within" the language. For example, take Smalltalk's implementation of control flow with the ifTrue:ifFalse: message. In DAS//, some concepts are allowed to be part of the "meta environment" to allow for more advanced optimizations. For example, Booleans and control structures map natively to how CPUs work, so if we don't try to pigeonhole these into messages, it is much easier to optimize them out into native CPU instructions (`test`, `jnz`, etc.)

## Gotchas

* There is no memory management at all, just yet. Things are allocated and never freed.
