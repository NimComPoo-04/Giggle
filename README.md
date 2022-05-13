# GIGGLE

Simple Webserver for Educational Perposes

###### Dependencies

* c18 complient compiler (preffered gcc)
* make build system
* unix based os
* libpthreads for multi threading
* requires lua-5.3.6 for stuff lol

This program should be ported to windows... in the _future_ ;P

###### Build Instruction

This will do all the things that are needed to build the project
this will also start the server

```
make get_dep
make run
```

The server hosts the server at: [http://localhost:8080/index.html](Here)

#### IN DEV DON'T USE

**BUGS:**
* Sometimes the server accepts connection but does not respond with anything atall.
This happens mostly in the situations when the server exits without doing more than
one connections. This is quite unfortunate.
