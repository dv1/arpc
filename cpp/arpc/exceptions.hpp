#ifndef ARPC_EXCEPTIONS_HPP
#define ARPC_EXCEPTIONS_HPP

#include <cstddef>
#include <string>
#include <stdexcept>


namespace arpc
{


class missing_call_parameters:
	public std::runtime_error
{
public:
	explicit missing_call_parameters(std::string const &function_name, std::size_t const num_parameters):
		runtime_error(function_name.c_str()),
		function_name(function_name),
		num_parameters(num_parameters)
	{
	}


	std::string get_function_name() const
	{
		return function_name;
	}


	size_t get_num_parameters() const
	{
		return num_parameters;
	}


protected:
	std::string const &function_name;
	std::size_t const num_parameters;
};


}


#endif

