#ifndef ARPC_LAMBDA_TRAITS_HPP
#define ARPC_LAMBDA_TRAITS_HPP

/*
lambda_traits.hpp - traits class for C++0x lambda expressions; determines return type and argument types of a lambda expression

Copyright (c) 2011 Carlos Rafael Giani

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt
*/

#include <arpc_config.h>

#ifdef WITH_CPP0X_VARIADIC_TEMPLATES
#include "from_variadic.hpp"
#else
#include <boost/mpl/vector.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#endif


namespace arpc
{



namespace detail
{


template < typename T >
struct lambda_traits_impl;


#ifdef WITH_CPP0X_VARIADIC_TEMPLATES

template < class C, class R, class ... A >
struct lambda_traits_impl < R (C::*)(A ...) >
{
	typedef typename from_variadic < A... > ::type arguments_t;
	typedef R result_t;
};

template < class C, class R, class ... A >
struct lambda_traits_impl < R (C::*)(A ...) const >
{
	typedef typename from_variadic < A... > ::type arguments_t;
	typedef R result_t;
};

template < class R, class ... A >
struct lambda_traits_impl < R (*)(A ...) >
{
	typedef typename from_variadic < A... > ::type arguments_t;
	typedef R result_t;
};


#else // no c++0x variadic templates


#ifndef ARPC_LAMBDA_TRAITS_IMPL_MAX_NUM_ARGUMENTS
#define ARPC_LAMBDA_TRAITS_IMPL_MAX_NUM_ARGUMENTS	10
#endif

#define ARPC_LAMBDA_TRAITS_IMPL_TEMPLATE_PARAM(z, N, u) , typename Arg##N

#define ARPC_LAMBDA_TRAITS_IMPL_DEFINE_STRUCTS(z, N, u) \
template < class C, class R  BOOST_PP_REPEAT(N, ARPC_LAMBDA_TRAITS_IMPL_TEMPLATE_PARAM, ~) > \
struct lambda_traits_impl < R (C::*)(BOOST_PP_ENUM_PARAMS(N, Arg)) > \
{ \
	typedef boost::mpl::vector < BOOST_PP_ENUM_PARAMS(N, Arg) > arguments_t; \
	typedef R result_t; \
}; \
\
template < class C, class R  BOOST_PP_REPEAT(N, ARPC_LAMBDA_TRAITS_IMPL_TEMPLATE_PARAM, ~) > \
struct lambda_traits_impl < R (C::*)(BOOST_PP_ENUM_PARAMS(N, Arg)) const > \
{ \
	typedef boost::mpl::vector < BOOST_PP_ENUM_PARAMS(N,  Arg) > arguments_t; \
	typedef R result_t; \
}; \
\
template < class R  BOOST_PP_REPEAT(N, ARPC_LAMBDA_TRAITS_IMPL_TEMPLATE_PARAM, ~) > \
struct lambda_traits_impl < R (*)(BOOST_PP_ENUM_PARAMS(N, Arg)) > \
{ \
	typedef boost::mpl::vector < BOOST_PP_ENUM_PARAMS(N, Arg) > arguments_t; \
	typedef R result_t; \
};

#define BOOST_PP_LOCAL_MACRO(n)   ARPC_LAMBDA_TRAITS_IMPL_DEFINE_STRUCTS(~, n, ~)
#define BOOST_PP_LOCAL_LIMITS     (0, ARPC_LAMBDA_TRAITS_IMPL_MAX_NUM_ARGUMENTS)

#include BOOST_PP_LOCAL_ITERATE()

#undef BOOST_PP_LOCAL_MACRO
#undef BOOST_PP_LOCAL_LIMITS

#undef ARPC_LAMBDA_TRAITS_IMPL_MAX_NUM_ARGUMENTS
#undef ARPC_LAMBDA_TRAITS_IMPL_DEFINE_STRUCTS



#endif


} // namespace detail end



template < typename T >
struct lambda_traits
{
	typedef decltype(&T::operator()) lambda_t;
	typedef detail::lambda_traits_impl < lambda_t > impl_t;
	typedef typename impl_t::arguments_t arguments_t;
	typedef typename impl_t::result_t result_t;

	// This concatenation of result and argument types is for convenience;
	// for example, the boost.function_types library expects such a sequence for function type synthesis
	typedef typename boost::mpl::push_front < arguments_t, result_t > ::type components_t;
};


}


#endif

