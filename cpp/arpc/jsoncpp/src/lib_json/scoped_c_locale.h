#ifndef JSONCPP_DETAIL_SCOPED_C_LOCALE_H_INCLUDED
# define JSONCPP_DETAIL_SCOPED_C_LOCALE_H_INCLUDED

namespace Json {
namespace detail {


// Forces the usage of '.' for decimal points
struct scoped_c_locale
{
	explicit scoped_c_locale();
	~scoped_c_locale();

	char *old_locale;
};


}
}

#endif

