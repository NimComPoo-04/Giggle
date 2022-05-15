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

The server hosts the server at: [Here](http://localhost:8080/index.html)

#### THIS VERY BAD, USE AT OWN RISK

**BUGS:**
Please Report if you see any :)

##### Server scripts

They are just simple lua scripts nothing more...

there are 5 functions to interact with the server

```lua
http_status()	-- sets the return status code (only few are actually supported)

local request_value = http_getreq('User-Agent')	  -- returns the request value for http_key if exists

local form_value = http_getform('name')    -- gets the post form value if any

http_print('<h1>hi</hi>')	-- -_- prints response

http_header('Location', '/')	-- sets header fields of response
```

the &#5f;&#5f;HTTP&#5f;&#5f; is reserved for internal use

Well thats all lol...

