require 'rubygems'
require 'eventmachine'
require File.expand_path('../../ruby/arpc/arpc', __FILE__)
require File.expand_path('../../ruby/arpc/json_serializer', __FILE__)


class MyProtocol < EventMachine::Protocols::LineAndTextProtocol

  @@connections = []

  def initialize
    super
    @@connections << self
    @nickname = ""
    @arpc = ARPC::ARPC.new ARPC::JsonSerializer
    @arpc.register_block(:message) { |nickname, line|
      @@connections.each do |connection|
        msg = @arpc.serialize.server_message nickname, line
        connection.send_line msg
      end
    }
    @arpc.register_block(:info) { |nickname|
      @nickname = nickname
      puts "Chatter #{@nickname} entered"
    }
  end

  def unbind
    puts "Chatter #{@nickname} left"
    @@connections.delete self
    super
  end

  def receive_line(line)
    @arpc.invoke_serialized_call line
  end

  def send_line(line)
    line += "\n" unless line[-1] == ?\n  # add EOL if not present
    send_data(line)
  end

end


begin
  EM.run do
    EventMachine::start_server '0.0.0.0', 51413, MyProtocol
  end
rescue Interrupt
  warn "User interrupt caught, exiting"
end

