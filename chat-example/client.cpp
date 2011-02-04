#include <iostream>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <arpc/arpc.hpp>
#include <arpc/json_serializer.hpp>


using namespace boost::asio;


class client:
	private boost::noncopyable
{
public:
	typedef ::arpc::arpc < arpc::json_serializer > arpc_t;


	explicit client(std::string const &nickname, io_service& io, ip::tcp::resolver::iterator endpoint_iterator):
		nickname(nickname),
		socket_(io),
		input_(io, ::dup(STDIN_FILENO))
	{
		arpc_.register_function < void(std::string const &, std::string const &) > (
			"server_message", 
			boost::bind(&client::server_message, this, _1, _2)
		);

		ip::tcp::endpoint endpoint = *endpoint_iterator;
		async_read_line(input_, boost::bind(&client::user_input, this, _1));
		socket_.async_connect(endpoint, boost::bind(&client::handle_connect, this, placeholders::error, ++endpoint_iterator));
	}


protected:
	bool user_input(std::string const &line)
	{
		if (line == "quit")
		{
			socket_.close();
			return false;
		}

		std::string buf;
		arpc_.serialize_call(buf, "message", nickname, line);
		write(socket_, buffer(&buf[0], buf.length()));
		return true;
	}


	bool server_input(std::string const &line)
	{
		arpc_.invoke_serialized_call(line);
		return true;
	}


	void server_message(std::string const &nick, std::string const &line)
	{
		std::cout << nick << "> " << line << std::endl;
	}


	void handle_connect(const boost::system::error_code& error, ip::tcp::resolver::iterator endpoint_iterator)
	{
		if (!error)
		{
			std::string buf;
			arpc_.serialize_call(buf, "info", nickname);
			write(socket_, buffer(&buf[0], buf.length()));

			async_read_line(socket_, boost::bind(&client::server_input, this, _1));
		}
		else if (endpoint_iterator != ip::tcp::resolver::iterator())
		{
			// That endpoint didn't work, try the next one.
			socket_.close();
			ip::tcp::endpoint endpoint = *endpoint_iterator;
			socket_.async_connect(endpoint, boost::bind(&client::handle_connect, this, placeholders::error, ++endpoint_iterator));
		}
	}


	typedef boost::function < bool(std::string const &line) > read_line_func_t;


	template < typename Source >
	void async_read_line(Source &source, read_line_func_t read_line_func)
	{
		async_read_until(
			source, input_buffer_, '\n',
			boost::bind(
				&client::handle_read_line < Source >, this, 
				boost::ref(source),
				placeholders::error,
				placeholders::bytes_transferred,
				read_line_func
			)
		);
	}


	template < typename Source >
	void handle_read_line(Source &source, const boost::system::error_code& error, std::size_t, read_line_func_t read_line_func)
	{
		if (!error)
		{
			std::istream in(&input_buffer_);
			std::string line;
			std::getline(in, line);

			if (read_line_func(line))
				async_read_line < Source > (source, read_line_func);
		}
	}


	arpc_t arpc_;
	std::string const &nickname;
	ip::tcp::socket socket_;
	posix::stream_descriptor input_;
	boost::asio::streambuf input_buffer_;
};


int main()
{
	try
	{
		std::cout << "Type in your nickname:" << std::endl;
		std::string nickname;
		std::getline(std::cin, nickname);
		std::cout << "----------------" << std::endl;
		std::cout << "Welcome to the C++ client for the simple chat example, " << nickname << ". Type quit to end the client" << std::endl;

		io_service io;

		ip::tcp::resolver resolver(io);
		ip::tcp::resolver::query query("localhost", "51413");
		ip::tcp::resolver::iterator iterator = resolver.resolve(query);

		client c(nickname, io, iterator);

		io.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}

