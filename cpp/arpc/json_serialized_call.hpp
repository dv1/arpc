#ifndef ARPC_JSON_SERIALIZED_CALL_HPP
#define ARPC_JSON_SERIALIZED_CALL_HPP

#include <arpc_config.h>

#include <algorithm>
#include <sstream>
#include <string>
#ifdef WITH_CPP0X_VARIADIC_TEMPLATES
#include <utility>
#else
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#endif
#include <boost/optional.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/at.hpp>

#include <json/value.h>
#include <json/reader.h>
#include <json/writer.h>

#include "serialized_call_traits.hpp"


namespace arpc
{


struct json_serialized_call
{
	class json_value_iterator:
		public boost::iterator_facade <
			json_value_iterator,
			Json::Value const,
			boost::bidirectional_traversal_tag
		>
	{
	public:
		json_value_iterator()
		{
		}

		json_value_iterator(Json::Value::const_iterator json_iterator):
			json_iterator(json_iterator)
		{
		}

		json_value_iterator(json_value_iterator const &other):
			json_iterator(other.json_iterator)
		{
		}


		json_value_iterator& operator = (json_value_iterator const &other)
		{
			json_iterator = other.json_iterator;
			return *this;
		}


		Json::Value const & dereference() const
		{
			return *json_iterator;
		}
		
		bool equal(json_value_iterator const &other) const
		{
			return json_iterator == other.json_iterator;
		}

		void increment()
		{
			++json_iterator;
		}

		void decrement()
		{
			--json_iterator;
		}

	protected:
		Json::Value::const_iterator json_iterator;
	};



	Json::Value json_value_array;
	typedef boost::iterator_range < json_value_iterator > parameter_range_t;
	std::string function_name;

	json_serialized_call():
		json_value_array(Json::arrayValue)
	{
	}
};


template < >
struct serialized_call_traits < json_serialized_call >
{
	typedef json_serialized_call::parameter_range_t parameters_t;
};


template < typename InputRange >
inline void read_from(json_serialized_call &serialized_call_, InputRange const &input)
{
	std::string str;
	std::copy(input.begin(), input.end(), std::back_inserter(str));

	Json::Value input_value;
	std::stringstream sstr(str);
	sstr >> input_value;

	if (!input_value.isMember("func"))
		throw std::runtime_error("func value is missing");
	if (!input_value.isMember("params"))
		throw std::runtime_error("params value is missing");

	Json::Value array_value = input_value["params"];
	if (!array_value.isConvertibleTo(Json::arrayValue))
		throw std::runtime_error("params value is not of type array");

	Json::Value func_name = input_value["func"];
	if (!func_name.isConvertibleTo(Json::stringValue))
		throw std::runtime_error("func value is not of type string");

	serialized_call_.json_value_array = array_value;
	serialized_call_.function_name = func_name.asString();
}


template < typename OutputIterator >
inline void write_to(json_serialized_call const &serialized_call_, OutputIterator output_begin)
{
	Json::Value output_value(Json::objectValue);
	output_value["params"] = serialized_call_.json_value_array;
	output_value["func"] = serialized_call_.function_name;
	std::string str = Json::FastWriter().write(output_value);
	std::copy(str.begin(), str.end(), output_begin);
}


inline json_serialized_call::parameter_range_t get_parameters(json_serialized_call const &json_serialized_call_)
{
	return json_serialized_call::parameter_range_t(json_serialized_call_.json_value_array.begin(), json_serialized_call_.json_value_array.end());
}


inline std::string get_function_name(json_serialized_call const &json_serialized_call_)
{
	return json_serialized_call_.function_name;
}


inline void set_function_name(json_serialized_call &json_serialized_call_, std::string const &function_name)
{
	json_serialized_call_.function_name = function_name;
}



namespace detail
{

template < typename T >
struct convert_json_value;

template < > struct convert_json_value < short >          { short          operator()(Json::Value const &value) const { return value.asInt(); } };
template < > struct convert_json_value < int >            { int            operator()(Json::Value const &value) const { return value.asInt(); } };
template < > struct convert_json_value < long >           { long           operator()(Json::Value const &value) const { return value.asInt(); } };
template < > struct convert_json_value < unsigned short > { unsigned short operator()(Json::Value const &value) const { return value.asUInt(); } };
template < > struct convert_json_value < unsigned int >   { unsigned int   operator()(Json::Value const &value) const { return value.asUInt(); } };
template < > struct convert_json_value < unsigned long >  { unsigned long  operator()(Json::Value const &value) const { return value.asUInt(); } };
template < > struct convert_json_value < float >          { float          operator()(Json::Value const &value) const { return value.asDouble(); } };
template < > struct convert_json_value < double >         { double         operator()(Json::Value const &value) const { return value.asDouble(); } };
template < > struct convert_json_value < std::string >    { std::string    operator()(Json::Value const &value) const { return value.asString(); } };
template < > struct convert_json_value < bool >           { bool           operator()(Json::Value const &value) const { return value.asBool(); } };

typedef boost::mpl::map <
	boost::mpl::pair < short,          boost::mpl::integral_c < Json::ValueType, Json::intValue > >,
	boost::mpl::pair < int,            boost::mpl::integral_c < Json::ValueType, Json::intValue > >,
	boost::mpl::pair < long,           boost::mpl::integral_c < Json::ValueType, Json::intValue > >,
	boost::mpl::pair < unsigned short, boost::mpl::integral_c < Json::ValueType, Json::uintValue > >,
	boost::mpl::pair < unsigned int,   boost::mpl::integral_c < Json::ValueType, Json::uintValue > >,
	boost::mpl::pair < unsigned long,  boost::mpl::integral_c < Json::ValueType, Json::uintValue > >,
	boost::mpl::pair < float,          boost::mpl::integral_c < Json::ValueType, Json::realValue > >,
	boost::mpl::pair < double,         boost::mpl::integral_c < Json::ValueType, Json::realValue > >,
	boost::mpl::pair < std::string,    boost::mpl::integral_c < Json::ValueType, Json::stringValue > >,
	boost::mpl::pair < bool,           boost::mpl::integral_c < Json::ValueType, Json::booleanValue > >
> json_value_type_map_t;


// using SFINAE to disable this function for types with no json_value_type_map_t entry
template < typename T >
bool get_parameter_value_impl(Json::Value const &json_value, T &value, typename boost::mpl::at < detail::json_value_type_map_t, T > ::type const *dummy = 0)
{
	dummy = dummy;

	if (json_value.isNull())
		return false;

	try
	{
		Json::ValueType const type_specifier = boost::mpl::at < detail::json_value_type_map_t, T > ::type::value;
		if (!json_value.isConvertibleTo(type_specifier))
			return false;

		value = detail::convert_json_value < T > ()(json_value);
		return true;
	}
	catch (std::exception const &)
	{
		return false;
	}
}


// special overload in case a Json::Value is requested directly
bool get_parameter_value_impl(Json::Value const &json_value, Json::Value &value)
{
	value = json_value;
	return true;
}


#ifdef WITH_CPP0X_VARIADIC_TEMPLATES


template < typename T >
inline void set_parameter_values_impl(json_serialized_call &json_serialized_call_, T&& param)
{
	json_serialized_call_.json_value_array.append(Json::Value(param));
}


template < typename First, typename ... Rest >
inline void set_parameter_values_impl(json_serialized_call &json_serialized_call_, First&& first, Rest&& ... rest)
{
	set_parameter_values_impl(json_serialized_call_, std::forward < First > (first));
	set_parameter_values_impl(json_serialized_call_, std::forward < Rest > (rest)...);
}


#endif


} // namespace detail end




template < typename T >
inline bool get_parameter_value(json_serialized_call const &, Json::Value const &json_value, T &value)
{
	// sometimes, the arity of the function is important; since one of the overloads has 3 arguments, not 2, this could cause trouble
	// solution: wrap them in a function with 2 arguments, which is done here
	return detail::get_parameter_value_impl < T > (json_value, value);
}


#ifdef WITH_CPP0X_VARIADIC_TEMPLATES


template < typename ... Params >
inline void set_parameter_values(json_serialized_call &json_serialized_call_, Params&& ... params)
{
	json_serialized_call_.json_value_array = Json::Value(Json::arrayValue);
	detail::set_parameter_values_impl(json_serialized_call_, std::forward < Params > (params)...);
}


#else // no c++0x variadic templates


#ifndef ARPC_SET_PARAMETER_VALUES_MAX_NUM_INDEX_PARAMS
#define ARPC_SET_PARAMETER_VALUES_MAX_NUM_INDEX_PARAMS	10
#endif

#define ARPC_SET_PARAMETER_VALUES_ACCESS_PARAMS(z, N, u) , Param##N const &param##N
#define ARPC_SET_PARAMETER_VALUES_APPEND_VALUE(z, N, u)  json_serialized_call_.json_value_array.append(Json::Value(param##N));

#define ARPC_SET_PARAMETER_VALUES_DEFINITION(z, N, u) \
template < BOOST_PP_ENUM_PARAMS(N, typename Param) > \
inline void set_parameter_values(json_serialized_call &json_serialized_call_  BOOST_PP_REPEAT(N, ARPC_SET_PARAMETER_VALUES_ACCESS_PARAMS, ~)) \
{ \
	BOOST_PP_REPEAT(N, ARPC_SET_PARAMETER_VALUES_APPEND_VALUE, ~) \
}

#define BOOST_PP_LOCAL_MACRO(n)   ARPC_SET_PARAMETER_VALUES_DEFINITION(~, n, ~)
#define BOOST_PP_LOCAL_LIMITS     (1, ARPC_SET_PARAMETER_VALUES_MAX_NUM_INDEX_PARAMS)

#include BOOST_PP_LOCAL_ITERATE()

#undef BOOST_PP_LOCAL_MACRO
#undef BOOST_PP_LOCAL_LIMITS

#undef ARPC_SET_PARAMETER_VALUES_MAX_NUM_INDEX_PARAMS
#undef ARPC_SET_PARAMETER_VALUES_ACCESS_PARAMS
#undef ARPC_SET_PARAMETER_VALUES_DEFINITION



#endif


}


#endif

