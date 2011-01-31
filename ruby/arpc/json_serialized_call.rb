require 'rubygems'
require 'json'


module ARPC


class JsonSerializedCall

  attr_accessor :function_name
  attr_accessor :parameters

  def initialize
  end

  def read buffer
    in_ = JSON.parse buffer
    @function_name = in_["func"]
    @parameters = in_["params"]
  end

  def write
    out = { :func => @function_name, :params => @parameters }
    JSON.generate out
  end

end


end

