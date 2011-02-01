=begin
/*
arpc.rb - main arpc Ruby module

arpc is a simple library for serializing asynchronous function calls and invoking said calls.
The three main functions are:
- serialize_call: serializes a function call; the first argument is the function name; the remaining arguments are arguments for the function call
  returns a String containing the serialized call
- invoke_serialized_call: given a serialized function call, invoke_serialized_call deserializes it, and calls any previously
  registered function with the name stored in the serialized call
- register_function: registers a function; the registry is used by invoke_serialized_call to find a suitable function after
  deserializing a call
  - register_instance_method: like register_function, but registering instance methods instead of functions
  - register_block_method: like register_function, but registering blocks instead of functions

Copyright (c) 2011 Carlos Rafael Giani

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt
*/
=end

module ARPC


class ARPC

  def initialize(serialized_call_class)
    @serialized_call_class = serialized_call_class
    @serialized_call_handlers = {}
  end

  def register_function(function_name)
    @serialized_call_handlers[function_name.to_s] = proc { |args| Object.send(function_name.to_s, *args) }
  end

  def register_instance_method(instance, method_name)
    @serialized_call_handlers[method_name.to_s] = proc { |args| instance.send(method_name.to_s, *args) }
  end

  def register_block(function_name, &block)
    @serialized_call_handlers[function_name.to_s] = block
  end

  def serialize_call(function_name, *args)
    serialized_call = @serialized_call_class.new
    serialized_call.function_name = function_name
    serialized_call.parameters = args
    serialized_call.write
  end

  def invoke_serialized_call(in_buffer)
    serialized_call = @serialized_call_class.new
    serialized_call.read in_buffer
    handler = @serialized_call_handlers[serialized_call.function_name().to_s]
    handler.call(serialized_call.parameters) if handler
  end

end


end

