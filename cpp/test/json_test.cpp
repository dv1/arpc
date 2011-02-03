#include <stdint.h>
#include <vector>
#include <string>
#include <arpc/arpc.hpp>
#include <arpc/json_serializer.hpp>


bool ok = false;

void foo(int i, std::string const &s)
{
	ok = (i == 5) && (s == "hello");
	if (!ok)
		std::cerr << "foo(): expected: (5, \"hello\"), got: (" << i << ", " << s << ")\n";
}


struct foofunctor
{
	void operator()(bool b, long l)
	{
		ok = b && (l == 42);
		if (!ok)
			std::cerr << "foofunctor: expected: (1, 42), got: (" << b << ", " << l << ")\n";
	}
};


int main()
{
	typedef std::vector < uint8_t > buf_t;
	using namespace arpc;
	buf_t buf;

	::arpc::arpc < json_serializer > rpc;
	rpc.register_function("foo", &foo);
	rpc.register_function < void(bool, long) > ("foofunctor", foofunctor());
#ifdef WITH_CPP0X_LAMBDA
	rpc.register_lambda("bar", [&](std::string const &s, bool b) {
		ok = (s == "hello") && !b;
		if (!ok)
			std::cerr << "lambda: expected: (\"hello\", false), got: (" << s << ", " << b << ")\n";
	});
#endif

	buf.clear();
	rpc.serialize_call(buf, "foo", 5, "hello");
	rpc.invoke_serialized_call(buf);
	if (!ok) return -1;

	buf.clear();
	rpc.serialize_call(buf, "foofunctor", true, 42);
	rpc.invoke_serialized_call(buf);
	if (!ok) return -1;

#ifdef WITH_CPP0X_LAMBDA
	buf.clear();
	rpc.serialize_call(buf, "bar", "hello", false);
	rpc.invoke_serialized_call(buf);
	if (!ok) return -1;
#endif

	return 0;
}

