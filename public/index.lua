-- creating html lol

http_print("<doctype! html>")
http_print("<html>")

http_print("<head>")
http_print("<title> Hello, World! </title>")
http_print("</head>")

http_print("<body>")
http_print("<h1>Hello, World!</h1>")
http_print("<p>")
http_print("This is pretty cool, if this works that is!<br>")
http_print("fname = " .. __HTTP_REQUEST__.Body.fname .. "<br>")
http_print("lname = " .. __HTTP_REQUEST__.Body.lname .. "<br>")
http_print("</p>")
http_print("</body>")

http_print("</html>")
