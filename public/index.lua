-- creating html lol

http_status(200)	-- don't forget to tell the server we did this perfectly ;)

body = [[
<doctype! html>
<html>
	<head>
		<title>Hello, World!</title>
	</head>
	<body>
		<h1> Hello, World </h1>
		<i>ahem ahem...</i>
		<p>
		Sever is a very bad name for a program like this<br>
		sounds fancy and cool<br>
		calling a minimum wage dude executive kind of thig<br>
		This is waiter lol<br><br>
		First Name: ]]..http_getform('fname')..[[ <br>
		Last Name: ]]..http_getform('lname')..[[ <br>
		Description: ]]..http_getform('desp')..[[ <br>
		</p>
	</body>
</html>
]]

http_print(body);
