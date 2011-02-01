/*
json_serializer.cpp - serializer call class using JSON

Copyright (c) 2011 Carlos Rafael Giani

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt
*/

#include "arpc/json_serializer.hpp"


namespace arpc
{


json_serializer::json_value_iterator::json_value_iterator()
{
}

json_serializer::json_value_iterator::json_value_iterator(Json::Value::const_iterator json_iterator):
	json_iterator(json_iterator)
{
}

json_serializer::json_value_iterator::json_value_iterator(json_value_iterator const &other):
	json_iterator(other.json_iterator)
{
}


json_serializer::json_value_iterator& json_serializer::json_value_iterator::operator = (json_value_iterator const &other)
{
	json_iterator = other.json_iterator;
	return *this;
}


Json::Value const & json_serializer::json_value_iterator::dereference() const
{
	return *json_iterator;
}

bool json_serializer::json_value_iterator::equal(json_value_iterator const &other) const
{
	return json_iterator == other.json_iterator;
}

void json_serializer::json_value_iterator::increment()
{
	++json_iterator;
}

void json_serializer::json_value_iterator::decrement()
{
	--json_iterator;
}



json_serializer::json_serializer():
	json_value_array(Json::arrayValue)
{
}


}

