/*
arpc.js - JavaScript implementation of arpc

arpc is a simple library for serializing asynchronous function calls and invoking said calls.
The three main functions are:
- serialize_call: serializes a function call; the first argument is the function name; the remaining arguments are arguments for the function call
  returns the serialized call as a string
- invoke_serialized_call: given a serialized function call, invoke_serialized_call deserializes it, and calls any previously
  registered function with the name stored in the serialized call
- register_function: registers a function; the registry is used by invoke_serialized_call to find a suitable function after
  deserializing a call

Copyright (c) 2011 Carlos Rafael Giani

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt
*/

function arpc(serializer_type)
{
	this.serializer_type = serializer_type;
	this.serialized_call_handlers = {};
}


arpc.prototype = {
	register_function : function(name, func)
	{
		if (foo == null) throw "foo is null";
		if (func == null) throw "func is null";
		var handler = function(params) { func.apply(this, params); };
		this.serialized_call_handlers[name] = handler;
	},

	serialize_call : function(function_name)
	{
		var serialized_call = new this.serializer_type;
		serialized_call.function_name = function_name;
		var args = [];
		for (var i = 1; i < arguments.length; i++)
			args[i - 1] = arguments[i];
		serialized_call.parameters = args;
		return serialized_call.write_to();
	},

	invoke_serialized_call : function(in_buffer)
	{
		var serialized_call = new this.serializer_type;
		serialized_call.read_from(in_buffer);
		var handler = this.serialized_call_handlers[serialized_call.function_name];
		if (handler)
			handler(serialized_call.parameters);
	}
}


function json_serializer()
{
	this.function_name = null;
	this.parameters = null;
}


json_serializer.prototype = {
	read_from : function(buffer)
	{
		in_ = JSON.parse(buffer);
		this.function_name = in_.func;
		this.parameters = in_.params;
	},

	write_to : function()
	{
		return JSON.stringify({ func: this.function_name, params : this.parameters });
	}
}

