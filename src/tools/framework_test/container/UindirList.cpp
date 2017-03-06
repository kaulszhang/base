// UnidirList.cpp

#include "tools/framework_test/Common.h"

#include "framework/container/UnidirList.h"
using namespace framework::configure;
using namespace framework::container;
using namespace framework::generic;

struct UnidirListItem
    : UnidirListHook<UnidirListItem, NativePointerTraits<UnidirListItem> >::type
{
    UnidirListItem(
        size_t n)
        : n(n)
    {
        list.push_front(this);
    }

    bool operator < (
        UnidirListItem const & r) const
    {
        return n < r.n;
    }

    size_t n;

    static UnidirList<UnidirListItem> list;
};

UnidirList<UnidirListItem> UnidirListItem::list;

void test_unidir_list(Config & conf)
{
    new UnidirListItem(4);
    new UnidirListItem(1);
    UnidirListItem * p1 = new UnidirListItem(3);
    UnidirListItem * p2 = new UnidirListItem(5);
    new UnidirListItem(2);

    std::cout << UnidirListItem::list.size() << std::endl;
    for (UnidirList<UnidirListItem>::const_iterator p = UnidirListItem::list.cbegin(); p != UnidirListItem::list.cend(); ++p) {
        std::cout << p->n << ' ';
    }
    std::cout << std::endl;
    for (UnidirListItem::pointer p = UnidirListItem::list.first(); p; p = UnidirListItem::list.next(p)) {
        std::cout << p->n << ' ';
    }
    std::cout << std::endl;

    new UnidirListItem(6);
    new UnidirListItem(9);
    new UnidirListItem(8);

    while(!UnidirListItem::list.empty()) {
        std::cout << "size = " << UnidirListItem::list.size() << std::endl;
        UnidirListItem::list.pop_front();
    }

    delete p1;
    delete p2;
}

static TestRegister test("unidir_list", test_unidir_list);
