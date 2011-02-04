require.paths.unshift('../js/arpc');
var net = require('net');
var arpc = require('arpc');


function readline(one_shot, callback)
{
	var this_ = this;
	this.callback = callback;
	this.stdin = process.openStdin();
	var handler = function(c) 
	{
		var line = c + "";
		line = line.slice(0, -1);
		this_.callback(line);
		if (one_shot)
			this_.stdin.removeListener("data", handler);
	};
	this.stdin.on("data", handler);
};



console.log("Type in your nickname:");

new readline(true, function(nick) {

	console.log("----------------");
	console.log("Welcome to the JavaScript client for the simple chat example, " + nick + ". Type quit to end the client");


	var rpc = new arpc.arpc(arpc.json_serializer);
	rpc.register_function("server_message", function(nickname, line) {
		console.log(nickname + "> " + line);
	});


	var connection = net.createConnection(51413, 'localhost');
	connection.on("connect", function()
	{
		var buf = rpc.serialize_call("info", nick);
		connection.write(buf + "\n");
		connection.flush();
	});
	connection.on("data", function(data)
	{
		rpc.invoke_serialized_call(data);
	});


	new readline(false, function(line) {
		if (line == "quit")
			process.exit(0);
		var buf = rpc.serialize_call("message", nick, line);
		connection.write(buf + "\n");
		connection.flush();
	});


});

