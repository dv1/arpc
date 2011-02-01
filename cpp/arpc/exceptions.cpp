/*
exceptions.cpp - various exception classes deriving from standard C++ exception classes

Copyright (c) 2011 Carlos Rafael Giani

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt
*/

#include "arpc/exceptions.hpp"


namespace arpc
{


missing_call_parameters::missing_call_parameters(std::string const &function_name, std::size_t const num_parameters):
	runtime_error(function_name.c_str()),
	function_name(function_name),
	num_parameters(num_parameters)
{
}


std::string missing_call_parameters::get_function_name() const
{
	return function_name;
}


size_t missing_call_parameters::get_num_parameters() const
{
	return num_parameters;
}


}

