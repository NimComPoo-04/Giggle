http_status(200)

-- Sending out a weird ass response because I am kinda stupid lol

doc = [[<html>
	<head>
		<title> Goblin </title>
	</head>
	<body>
		<h1>
		That was amazing ]]..http_getform('name')..[[ defenitely better than Nim's version.<br>
		I would forward this story to Nim, he really needs to learn how to write stories.<br>
		Anyways bye Have a nice day<br>
		And continue to write awesome stories<br>
		<i>~Giggle</i>
		</h1>
	</body>
</html>]]

http_print(doc);

print("-------------")
print("Story: ")
print(http_getform('name'))
print(http_getform('story'))
print("-------------")
