#ifndef ARPC_FROM_VARIADIC_HPP
#define ARPC_FROM_VARIADIC_HPP

/*
from_variadic.hpp - utility code to convert variadic template arguments to a Boost.MPL vector

original code from Dmytro Shandyba's blog:
http://blog.shandyba.com/2009/12/17/converting-variadic-template-arguments-pack-to-boost-mpl-sequence/

Copyright (c) 2011 Carlos Rafael Giani

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt
*/

#include <boost/mpl/vector.hpp>
#include <boost/mpl/push_front.hpp>


namespace arpc
{


template < typename ...Args > struct from_variadic;

template < class T, typename ...Args >
struct from_variadic < T, Args... >
{
	typedef typename boost::mpl::push_front < typename from_variadic < Args... > ::type, T> ::type type;
};

template <class T>
struct from_variadic < T >
{
	typedef boost::mpl::vector < T > type;
};

template < >
struct from_variadic < >
{
	typedef boost::mpl::vector < > type;
};


}


#endif

