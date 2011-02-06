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

As an alternative to serialize_call, serialize can be used. Example:
   rpc.serialize_call("foo", 'a', 'b', 'c')  # method 1
   rpc.serialize.foo('a', 'b', 'c')  # method 2

but please note that serialize makes use of method_missing, and respond_to? will always return true. This is a consequence
of the fact that these "function" calls actually just generate messages.

Copyright (c) 2011 Carlos Rafael Giani

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt
*/
=end

module ARPC


class ARPC

  class Serialize
    def initialize arpc_instance
      @arpc_instance = arpc_instance
    end

    def method_missing(sym, *args, &block)
      @arpc_instance.serialize_call sym, *args
    end

    def respond_to?(sym)
      true
    end
  end


  attr_reader :serialize

  def initialize(serializer_class)
    @serializer_class = serializer_class
    @serialized_call_handlers = {}
    @serialize = Serialize.new self
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
    serialized_call = @serializer_class.new
    serialized_call.function_name = function_name
    serialized_call.parameters = args
    serialized_call.write_to
  end

  def invoke_serialized_call(in_buffer)
    serialized_call = @serializer_class.new
    serialized_call.read_from in_buffer
    handler = @serialized_call_handlers[serialized_call.function_name().to_s]
    handler.call(serialized_call.parameters) if handler
  end

end


end

