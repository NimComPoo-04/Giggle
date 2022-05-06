-- The reason for calling this glue is that it glues lua scripts to
-- the internal server code lol

__KEEP_ALIVE__ = 0

__HTTP_RESPONSE__ =
"HTTP/1.1 200 OK\r\n"..
"Server: Giggle\r\n"..
"Connection: close\r\n"..
"content-length:21\r\n\r\n"..
"<h1>Hello, World</h1>"

print(__HTTP_REQUEST__)
