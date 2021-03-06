=begin
json_serializer.rb - serializer class using JSON

Copyright (c) 2011 Carlos Rafael Giani

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt
=end


require 'rubygems'
require 'json'


module ARPC


class JsonSerializer

  attr_accessor :function_name
  attr_accessor :parameters

  def initialize
  end

  def read_from buffer
    in_ = JSON.parse buffer
    @function_name = in_["func"]
    @parameters = in_["params"]
  end

  def write_to
    out = { :func => @function_name, :params => @parameters }
    JSON.generate out
  end

end


end

