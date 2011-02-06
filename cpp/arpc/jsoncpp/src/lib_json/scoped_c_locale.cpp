#include <clocale>
#include "scoped_c_locale.h"

namespace Json {
namespace detail {


scoped_c_locale::scoped_c_locale()
{
	old_locale = std::setlocale(LC_NUMERIC, 0);
	std::setlocale(LC_NUMERIC, "C");
}

scoped_c_locale::~scoped_c_locale()
{
	std::setlocale(LC_NUMERIC, old_locale);
}


}
}


