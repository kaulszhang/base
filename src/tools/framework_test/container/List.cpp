// List.cpp

#include "tools/framework_test/Common.h"

#include "framework/container/List.h"
using namespace framework::configure;
using namespace framework::container;
using namespace framework::generic;

struct ListItem
    : ListHook<ListItem, NativePointerTraits<ListItem> >::type
{
    ListItem(
        size_t n)
        : n(n)
    {
        List<ListItem>::reverse_iterator i = list.rbegin();
        while (i != list.rend() && i->n < n)
            ++i;
        list.insert(i, this);
    }

    bool operator < (
        ListItem const & r) const
    {
        return n < r.n;
    }

    size_t n;

    static List<ListItem> list;
};

List<ListItem> ListItem::list;

void test_list(Config & conf)
{
    new ListItem(4);
    new ListItem(1);
    ListItem * p1 = new ListItem(3);
    ListItem * p2 = new ListItem(5);
    new ListItem(2);

    std::cout << ListItem::list.size() << std::endl;
    for (List<ListItem>::const_iterator p = ListItem::list.cbegin(); p != ListItem::list.cend(); ++p) {
        std::cout << p->n << ' ';
    }
    std::cout << std::endl;
    ListItem::list.rotate(ListItem::list.begin(), ListItem::list.begin(), ListItem::list.end());
    for (ListItem::pointer p = ListItem::list.first(); p; p = ListItem::list.next(p)) {
        std::cout << p->n << ' ';
    }
    std::cout << std::endl;
    for (ListItem::const_pointer p = ListItem::list.last(); p; p = ListItem::list.prev(p)) {
        std::cout << p->n << ' ';
    }
    std::cout << std::endl;

    //p1->n = 7;
    //ListItem::list.vacidate_order(p1);

    //for (list<ListItem>::const_iterator p = ListItem::list.begin(); p != ListItem::list.end(); ++p) {
    //    std::cout << p->n << ' ';
    //}
    //std::cout << std::endl;

    //p2->n = 3;
    //ListItem::list.vacidate_order(p2);

    //for (list<ListItem>::const_iterator p = ListItem::list.begin(); p != ListItem::list.end(); ++p) {
    //    std::cout << p->n << ' ';
    //}
    //std::cout << std::endl;

    //ListItem::list.vacidate_order(p2);

    //for (list<ListItem>::const_iterator p = ListItem::list.begin(); p != ListItem::list.end(); ++p) {
    //    std::cout << p->n << ' ';
    //}
    //std::cout << std::endl;
    p1 = ListItem::list.erase(p1);
    std::cout << p1->n;
    std::cout << std::endl;

    std::cout << ListItem::list.size() << std::endl;
    for (ListItem::const_pointer p = ListItem::list.last(); p; p = ListItem::list.prev(p)) {
        std::cout << p->n << ' ';
    }
    std::cout << std::endl;

    new ListItem(6);
    new ListItem(9);
    new ListItem(8);

    std::cout << ListItem::list.size() << std::endl;
    for (ListItem::const_pointer p = ListItem::list.last(); p; p = ListItem::list.prev(p)) {
        std::cout << p->n << ' ';
    }
    std::cout << std::endl;

    ListItem::list.clear();

    std::cout << ListItem::list.size() << std::endl;
    for (ListItem::const_pointer p = ListItem::list.last(); p; p = ListItem::list.prev(p)) {
        std::cout << p->n << ' ';
    }
    std::cout << std::endl;

    delete p1;
    delete p2;
}

static TestRegister test("list", test_list);
