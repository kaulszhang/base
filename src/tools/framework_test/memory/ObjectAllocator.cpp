// TestObjectAllocator.cpp

#include "tools/framework_test/Common.h"

#include <framework/memory/ObjectAllocator.h>
using namespace framework::configure;
using namespace framework::memory;

struct Object
{
	Object(int i, std::string const & str) : i(i), str(str) {}
	int i;
	std::string str;
};

static void test_object_allocator(Config & conf)
{
	std::list<Object, ObjectAllocator<Object> > lst;
	lst.push_back(Object(1, "2"));
	lst.clear();
}

static TestRegister test("object_allocator", test_object_allocator);
