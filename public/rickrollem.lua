http_status(200)
http_header('Refresh', '6;url=https://www.youtube.com/watch?v=dQw4w9WgXcQ&t=0s')

doc = [[<html>
<head>
<title>Get Rick Rolled ;)!!</title>
<style>
#RICK {
	font-size: 100px;
	margin: 10%;
	color: seagreen;
}
#ROLLED {
	font-size: 72px;
	margin: 10%;
	color: darkred;
}
</style>
</head>
<body>
<div id='RICK'>GET RICKROLLED ;)</div>
<div id='ROLLED'> It was clear that those were dead links you still decided to click on them!</div>
</body>
</html>]]

http_print(doc)
