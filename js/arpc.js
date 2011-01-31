function arpc(serialized_call_type)
{
	this.serialized_call_type = serialized_call_type;
	this.serialized_call_handlers = {};
}


arpc.prototype = {
	register_function : function(name, func)
	{
		var handler = function(params) { func.apply(this, params); };
		this.serialized_call_handlers[name] = handler;
	},

	serialize_call : function(function_name)
	{
		var serialized_call = new this.serialized_call_type;
		serialized_call.function_name = function_name;
		var args = [];
		for (var i = 1; i < arguments.length; i++)
			args[i - 1] = arguments[i];
		serialized_call.parameters = args;
		return serialized_call.write_to();
	},

	invoke_serialized_call : function(in_buffer)
	{
		var serialized_call = new this.serialized_call_type;
		serialized_call.read_from(in_buffer);
		var handler = this.serialized_call_handlers[serialized_call.function_name];
		if (handler)
			handler(serialized_call.parameters);
	}
}


function json_serialized_call()
{
	this.function_name = null;
	this.parameters = null;
}


json_serialized_call.prototype = {
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




// TEST/EXAMPLE START


xxx = {
	val : 9143,
	foo : function(i,j,k) { print("xxxfoo" + ' ' + i + ' ' + j + ' ' + k + '   ' + xxx.val); }
}


var r = new arpc(json_serialized_call);
r.register_function("foobar", function(i,j) { print("foobar " + i + ' ' + j); });
r.register_function("xxx", xxx.foo);

t = r.serialize_call("foobar", 4,1);
r.invoke_serialized_call(t);
r.invoke_serialized_call('{"func":"foobar", "params":[5,1]}')
r.invoke_serialized_call('{"func":"xxx", "params":[5,1,9]}')


// TEST/EXAMPLE END

