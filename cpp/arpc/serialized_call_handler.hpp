#ifndef ARPC_SERIALIZED_CALL_HANDLER_HPP
#define ARPC_SERIALIZED_CALL_HANDLER_HPP


#include <iterator>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/fusion/include/invoke.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/fusion/sequence/intrinsic/begin.hpp>
#include <boost/fusion/sequence/intrinsic/end.hpp>
#include <boost/fusion/iterator/deref.hpp>
#include <boost/fusion/iterator/value_of.hpp>
#include <boost/fusion/iterator/next.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/iteration/local.hpp>

#include <arpc_config.h>

#include "exceptions.hpp"
#include "serialized_call_traits.hpp"


namespace arpc
{


template < typename SerializedCall >
class serialized_call_handler_base
{
public:
	typedef SerializedCall serialized_call_t;


	virtual ~serialized_call_handler_base()
	{
	}


	virtual void invoke_function(serialized_call_t const &serialized_call_) = 0;
};


namespace detail
{


/*
these functions fill a fusion vector with values from a parameter sequence
the values need to be retrieved from the sequence using the get_parameter_value < > function, for each entry in the fusion vector
so, what this code does is this:
start at the beginning of the parameter sequence, retrieve the first value (using the type of the first entry in the fusion vector type list) and store it in the fusion vector
then, move to the next element in the parameter sequence, retrieve the second value (using the type of the second entry in the fusion vector type list) and store it in the fusion vector
etc.

The parameter sequence is specified using the iter and end arguments. iter is an STL iterator. (end exists only for catching error cases.)

Iteration is done using recursion, which is common in C++ metaprogramming land. Each iteration uses an fusion meta iterator that refers to the type of the current element in the fusion
vector type list.
*/

template < typename SerializedCall, typename FusionVector, typename FusionVectorIter, typename ParamValueIter, unsigned int Index >
struct set_parameter_fusion_vector_value
{
	bool operator()(SerializedCall const &serialized_call_, FusionVector &fusion_vector, ParamValueIter iter, ParamValueIter end) const
	{
		if (iter == end)
		{
			typename serialized_call_traits < SerializedCall > ::parameters_t parameters = get_parameters(serialized_call_);
			throw missing_call_parameters(get_function_name(serialized_call_), std::distance(parameters.begin(), parameters.end()));
		}

		typedef typename boost::remove_cv < typename boost::fusion::result_of::value_of < FusionVectorIter > ::type > ::type param_type_t;

		// Retrieve the value from the current element parameter sequence. The current element is (*iter).

		param_type_t param_value;
		if (get_parameter_value(serialized_call_, *iter, param_value))
		{
			// Store the value in the fusion vector
			// TODO: use fusion iterators instead of an index (if it makes sense; run-time fusion iterators need to be used here)
			boost::fusion::at_c < Index > (fusion_vector) = param_value;

			// Move to the next element
			ParamValueIter next_iter = iter;
			std::advance(next_iter, 1);

			// Next iteration (using recursion)
			return set_parameter_fusion_vector_value < SerializedCall, FusionVector, typename boost::fusion::result_of::next < FusionVectorIter > ::type, ParamValueIter, Index + 1 > ()(serialized_call_, fusion_vector, next_iter, end);
		}
		else
			return false;
	}
};


// This specialized struct is used as a terminator. If the MPL iterator == end of MPL sequence, we are done, and this terminator kicks in.
template < typename SerializedCall, typename FusionVector, typename ParamValueIter, unsigned int Index >
struct set_parameter_fusion_vector_value < SerializedCall, FusionVector, typename boost::fusion::result_of::end < FusionVector > ::type, ParamValueIter, Index >
{
	bool operator()(SerializedCall const &, FusionVector &, ParamValueIter/* iter*/, ParamValueIter/* end*/) const
	{
		/*if (iter != end)
			std::cerr << "Warning: more parameters present than expected in the serialized call" << std::endl;*/

		return true;
	}
};


template < typename T >
struct check_type
{
	typedef typename boost::remove_reference < T > ::type without_ref_t;
	typedef typename boost::is_pointer < without_ref_t > ::type is_ptr_t;
	typedef typename boost::is_const < without_ref_t > ::type is_const_t;
	typedef typename boost::is_reference < T > ::type is_ref_t;
	typedef typename boost::mpl::and_ < typename boost::mpl::not_ < is_const_t > ::type, is_ref_t > ::type is_nonconst_ref_t;
	typedef typename boost::mpl::or_ < is_ptr_t, is_nonconst_ref_t > ::type is_invalid_t;
	typedef typename boost::mpl::not_ < is_invalid_t > ::type type;
};


template < typename Parameters >
struct validate_parameter_sequence
{
	typedef typename boost::mpl::fold <
		Parameters,
		boost::mpl::bool_ < true >,
		boost::mpl::and_ < boost::mpl::_1, check_type < boost::mpl::_2 > >
	> ::type type;
};


}


template < typename SerializedCall, typename FuncType >
class serialized_call_handler:
	public serialized_call_handler_base < SerializedCall >
{
public:
	typedef SerializedCall serialized_call_t;

	/*
	The parameters may contain cv qualifiers, references ..
	for example, a function may have parameters like "std::string const &".
	Such a parameter is converted to "std::string" with the typedef lines below. The "input" is the raw function_parameters_with_cv_qualifiers_and_refs_t
	MPL sequence, the "output" is the function_parameters_t sequence, stripped of cv qualifiers and const references.

	Non-const references are a problem; they may be intended to be used as return values (return values ARE DELIBERATELY NOT SUPPORTED in the arpc system).
	This is the reason why a sanity check is done first; if a non-const reference is found within the function parameter list, a boost MPL assert is produced.

	Pointer types are also considered to be invalid; even const pointers are not accepted. The parameter values might be sent across hosts, so a pointer value loses any meaning.
	Const references are OK because they are converted to value types; the same cannot be applied to const pointers.

	As for const char * : C-strings are not supported, use something like STL strings instead.

	If the pointer value REALLY needs to be transmitted, use intptr_t or uintptr_t.
	*/

	typedef boost::function_types::parameter_types < FuncType > function_parameters_with_cv_qualifiers_and_refs_t;

private:
	typedef detail::validate_parameter_sequence < function_parameters_with_cv_qualifiers_and_refs_t > validation_result_t;
#ifdef WITH_CPP0X_STATIC_ASSERT
	static_assert(validation_result_t::type::value, "function parameters may not be pointers or non-const references");
#else
	BOOST_MPL_ASSERT_MSG(
		validation_result_t::type::value,
		FUNCTION_PARAMETERS_MAY_NOT_BE_POINTERS_OR_NONCONST_REFS,
		(function_parameters_with_cv_qualifiers_and_refs_t)
	);
#endif

public:
	typedef typename boost::mpl::transform < function_parameters_with_cv_qualifiers_and_refs_t, boost::remove_reference < boost::mpl::_1 > > ::type function_parameters_with_cv_qualifiers_t;
	typedef typename boost::mpl::transform < function_parameters_with_cv_qualifiers_t, boost::remove_cv < boost::mpl::_1 > > ::type function_parameters_t;

	typedef boost::function < FuncType > function_object_t;


	explicit serialized_call_handler(function_object_t const &function_):
		function_object(function_)
	{
	}


	virtual void invoke_function(serialized_call_t const &serialized_call_)
	{
		typedef typename boost::fusion::result_of::as_vector < function_parameters_t > ::type fusion_vector_t;
		fusion_vector_t fusion_vector;
		typename serialized_call_traits < serialized_call_t > ::parameters_t parameters = get_parameters(serialized_call_);


		detail::set_parameter_fusion_vector_value <
			SerializedCall,
			fusion_vector_t,
			typename boost::fusion::result_of::begin < fusion_vector_t > ::type,
			typename serialized_call_traits < serialized_call_t > ::parameters_t::const_iterator,
			0
		> ()(serialized_call_, fusion_vector, parameters.begin(), parameters.end());

		boost::fusion::invoke(function_object, fusion_vector);		
	}


protected:
	function_object_t function_object;
};


}


#endif


