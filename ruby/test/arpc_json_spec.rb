#!/usr/bin/env ruby

require 'rubygems'
require 'spec'
require File.expand_path('../../arpc/arpc', __FILE__)
require File.expand_path('../../arpc/json_serializer', __FILE__)


class MyIO
  def initialize
    @bufs = []
  end

  def receive_serialized_call do_block
    if @bufs.empty?
      nil
    else
      @bufs.shift
    end
  end

  def send_serialized_call out_buffer
    @bufs << out_buffer
  end
end


@@func_call_params = []
def foobar i, f, text
  puts "foobar called: #{i} #{f} #{text}"
  @@func_call_params = [i, f, text]
end


rpc = nil
@@block_call_params = []
describe ARPC::ARPC do
  before(:all) do
    rpc = ARPC::ARPC.new ARPC::JsonSerializer
    rpc.register_function :foobar
    rpc.register_block(:abcd) { |a, b, c, d|
      puts "block called: #{a} #{b} #{c} #{d}"
      @@block_call_params = [a, b, c, d]
    }
  end

  before(:each) do
    @@func_call_params = []
    @@block_call_params = []
  end

  it "should call the foobar function" do
    expected_params = [1, 0.5, "hello"]
    t = rpc.serialize.foobar *expected_params
    rpc.invoke_serialized_call t
    @@func_call_params.should == expected_params
  end

  it "should call the abc block" do
    expected_params = ["X", "Y", 185.24, false]
    t = rpc.serialize_call "abcd", *expected_params
    rpc.invoke_serialized_call t
    @@block_call_params.should == expected_params
  end
end

