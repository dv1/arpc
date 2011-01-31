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

