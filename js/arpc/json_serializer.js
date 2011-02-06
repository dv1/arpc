/*
json_serializer.js - serializer class using JSON

Copyright (c) 2011 Carlos Rafael Giani

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt
*/

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


exports.json_serializer = json_serializer;

