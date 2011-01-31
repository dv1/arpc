#ifndef ARPC_FROM_VARIADIC_HPP
#define ARPC_FROM_VARIADIC_HPP

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

