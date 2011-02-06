#ifndef ARPC_ARPC_HPP
#define ARPC_ARPC_HPP

/*
arpc.hpp - main arpc header

arpc is a simple library for serializing asynchronous function calls and invoking said calls.
The three main functions are:
- serialize_call: serializes a function call; the first argument is the output where the serialized call shall be written to;
  the second argument is the function name; the remaining arguments are arguments for the function call
- invoke_serialized_call: given a serialized function call, invoke_serialized_call deserializes it, and calls any previously
  registered function with the name stored in the serialized call
- register_function: registers a function; the registry is used by invoke_serialized_call to find a suitable function after
  deserializing a call
  (register_lambda does the same, but for C++0x lambda expressions)

Copyright (c) 2011 Carlos Rafael Giani

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt
*/

#include <map>
#include <string>
#include <vector>
#include <arpc_config.h>
#include <boost/shared_ptr.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_function.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include "serialized_call_handler.hpp"
#include "functor_traits.hpp"
#ifndef WITH_CPP0X_VARIADIC_TEMPLATES
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#endif


namespace arpc
{


template < typename SerializedCall >
class arpc
{
protected:
	template < typename Function >
	struct is_function_pointer
	{
		typedef typename boost::mpl::and_ <
			typename boost::is_pointer < Function > ::type,
			typename boost::is_function < typename boost::remove_pointer < Function > ::type > ::type
		> ::type type;
	};


public:
	typedef SerializedCall serialized_call_t;
	typedef boost::shared_ptr < serialized_call_handler_base < SerializedCall > > serialized_call_handler_ptr_t;
	typedef std::map < std::string, serialized_call_handler_ptr_t > serialized_call_handlers_t;


	// FuncType is the type signature of the function, for example void(int)
	// must be specified explicitely (for now; see below)
	template < typename FuncType, typename Function >
	void register_function(std::string const &function_name, Function const &function)
	{
		// it would be useful to be able to deduce FuncType from any kind of Function,
		// however, while this is easy to do with the function_types library for function pointers,
		// function objects and functors do not work with it, since they can be polymorphic;
		// for instance, a functor with a templated call operator, like
		//   template < typename T, typename U > void operator()(T t, U u) { ... }
		// cannot have a FuncType deduced, since T and U are free variables

		typedef serialized_call_handler < SerializedCall, FuncType > serialized_call_handler_t;

		serialized_call_handler_t *serialized_call_handler_ = new serialized_call_handler_t(function);
		serialized_call_handlers[function_name] = serialized_call_handler_ptr_t(serialized_call_handler_);
	}


	// Specialized variant of register_function() for function pointers; the function type can be deduced automatically from these
	// For example, this works: void foo(int) {}    ...  register_function("foo", &foo);
	template < typename Function >
	void register_function(
		std::string const &function_name,
		Function const &function,
		typename boost::enable_if < typename is_function_pointer < Function > ::type > ::type const *dummy = 0
	)
	{
		typedef typename boost::function_types::function_type < boost::function_types::components < Function > > ::type synthesized_function_type_t;
		dummy = dummy;
		register_function < synthesized_function_type_t, Function > (function_name, function);
	}


	// Specialized variant of register_function() for functors with non-templated call operators; the function type can be deduced automatically from these
	// For example, this works: struct foo { void operator() {} };    ...  register_function("foo", foo());
	// Does NOT work properly for functors with templated call operators, and requires the C++0x decltype() operator (used inside functor_traits)
	template < typename Function >
	void register_function(
		std::string const &function_name,
		Function const &function,
		typename boost::disable_if < typename is_function_pointer < Function > ::type > ::type const *dummy = 0
	)
	{
		dummy = dummy;
		typedef functor_traits < Function > traits_t;
		typedef typename boost::function_types::function_type < typename traits_t::components_t > ::type synthesized_function_type_t;
		register_function < synthesized_function_type_t, Function > (function_name, function);
	}


#ifdef WITH_CPP0X_VARIADIC_TEMPLATES


	template < typename Output, typename FirstParam, typename ... Params >
	void serialize_call(Output &output, std::string const &function_name, FirstParam && first_param, Params&& ... params)
	{
		serialized_call_t serialized_call_;
		set_function_name(serialized_call_, function_name);
		set_parameter_values(serialized_call_, std::forward < FirstParam > (first_param), std::forward < Params > (params)...);

		write_to(serialized_call_, std::back_inserter(output));
	}


#else // no c++0x variadic templates


#ifndef ARPC_SERIALIZE_CALL_MAX_NUM_INDEX_PARAMS
#define ARPC_SERIALIZE_CALL_MAX_NUM_INDEX_PARAMS	10
#endif

#define ARPC_SERIALIZE_CALL_ACCESS_PARAMS(z, N, u) , Param##N const &param##N

#define ARPC_SERIALIZE_CALL_DEFINITION(z, N, u) \
	template < typename Output, BOOST_PP_ENUM_PARAMS(N, typename Param) > \
	void serialize_call(Output &output, std::string const &function_name BOOST_PP_REPEAT(N, ARPC_SERIALIZE_CALL_ACCESS_PARAMS, ~)) \
	{ \
		serialized_call_t serialized_call_; \
		set_function_name(serialized_call_, function_name); \
		set_parameter_values(serialized_call_, BOOST_PP_ENUM_PARAMS(N, param)); \
 \
		write_to(serialized_call_, std::back_inserter(output)); \
	}

#define BOOST_PP_LOCAL_MACRO(n)   ARPC_SERIALIZE_CALL_DEFINITION(~, n, ~)
#define BOOST_PP_LOCAL_LIMITS     (1, ARPC_SERIALIZE_CALL_MAX_NUM_INDEX_PARAMS)

#include BOOST_PP_LOCAL_ITERATE()

#undef BOOST_PP_LOCAL_MACRO
#undef BOOST_PP_LOCAL_LIMITS

#undef ARPC_SERIALIZE_CALL_MAX_NUM_INDEX_PARAMS
#undef ARPC_SERIALIZE_CALL_ACCESS_PARAMS
#undef ARPC_SERIALIZE_CALL_DEFINITION


#endif


	template < typename Input >
	void invoke_serialized_call(Input const &input)
	{
		serialized_call_t serialized_call_;
		read_from(serialized_call_, input);
		typename serialized_call_handlers_t::iterator iter = serialized_call_handlers.find(get_function_name(serialized_call_));
		if (iter != serialized_call_handlers.end())
			iter->second->invoke_function(serialized_call_);
	}


protected:
	serialized_call_handlers_t serialized_call_handlers;
};


}


#endif


