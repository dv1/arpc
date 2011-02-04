require 'rubygems'
require 'eventmachine'
require File.expand_path('../../ruby/arpc/arpc', __FILE__)
require File.expand_path('../../ruby/arpc/json_serializer', __FILE__)

module KeyboardInput
  include EM::Protocols::LineText2

  def initialize protocol, arpc, nick
    @protocol = protocol
    @arpc = arpc
    @nick = nick
  end

  def receive_line data
    if data == "quit"
      puts "Closing connection"
      @protocol.close_connection_after_writing
    else
      msg = @arpc.serialize.message @nick, data
      @protocol.send_line msg
    end
  end
end

class MyProtocol < EventMachine::Protocols::LineAndTextProtocol

  def initialize nick
    super
    @arpc = ARPC::ARPC.new ARPC::JsonSerializer
    @arpc.register_block(:server_message) { |nickname, line|
      puts "#{nickname}> #{line}"
    }

    @nick = nick
    msg = @arpc.serialize.info @nick
    send_line msg

    EM.open_keyboard(KeyboardInput, self, @arpc, nick)
  end

  def unbind
    super
    EM.stop_event_loop
  end

  def receive_line(line)
    @arpc.invoke_serialized_call line
  end

  def send_line(line)
    line += "\n" unless line[-1] == ?\n  # add EOL if not present
    send_data(line)
  end

end


puts "Type in your nickname:"
nick = gets.chomp

puts "----------------"
puts "Welcome to the Ruby client for the simple chat example, #{nick}. Type quit to end the client"

begin
  EM.run do
    EventMachine::connect 'localhost', 51413, MyProtocol, nick
  end
rescue Interrupt
  warn "User interrupt caught, exiting"
end

