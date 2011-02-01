#ifndef ARPC_EXCEPTIONS_HPP
#define ARPC_EXCEPTIONS_HPP

/*
exceptions.hpp - various exception classes deriving from standard C++ exception classes

Copyright (c) 2011 Carlos Rafael Giani

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt
*/

#include <cstddef>
#include <string>
#include <stdexcept>


namespace arpc
{


class missing_call_parameters:
	public std::runtime_error
{
public:
	explicit missing_call_parameters(std::string const &function_name, std::size_t const num_parameters);

	std::string get_function_name() const;
	size_t get_num_parameters() const;


protected:
	std::string const &function_name;
	std::size_t const num_parameters;
};


}


#endif

