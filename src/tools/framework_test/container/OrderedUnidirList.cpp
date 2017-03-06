// OrderedUnidirList.cpp

#include "tools/framework_test/Common.h"

#include "framework/container/OrderedUnidirList.h"
using namespace framework::configure;
using namespace framework::container;
using namespace framework::generic;

struct OrderedUnidirListItem
    : OrderedUnidirListHook<OrderedUnidirListItem, NativePointerTraits<OrderedUnidirListItem> >::type
{
    OrderedUnidirListItem(
        size_t n)
        : n(n)
    {
        list.push(this);
    }

    bool operator < (
        OrderedUnidirListItem const & r) const
    {
        return n < r.n;
    }

    size_t n;

    static OrderedUnidirList<OrderedUnidirListItem> list;
};

OrderedUnidirList<OrderedUnidirListItem> OrderedUnidirListItem::list;

void test_ordered_unidir_list(Config & conf)
{
    new OrderedUnidirListItem(4);
    new OrderedUnidirListItem(1);
    OrderedUnidirListItem * p1 = new OrderedUnidirListItem(3);
    OrderedUnidirListItem * p2 = new OrderedUnidirListItem(5);
    new OrderedUnidirListItem(2);

    std::cout << OrderedUnidirListItem::list.size() << std::endl;
    for (OrderedUnidirList<OrderedUnidirListItem>::const_iterator p = OrderedUnidirListItem::list.cbegin(); p != OrderedUnidirListItem::list.cend(); ++p) {
        std::cout << p->n << ' ';
    }
    std::cout << std::endl;
    for (OrderedUnidirListItem::pointer p = OrderedUnidirListItem::list.first(); p; p = OrderedUnidirListItem::list.next(p)) {
        std::cout << p->n << ' ';
    }
    std::cout << std::endl;

    //p1->n = 7;
    //OrderedUnidirListItem::list.vacidate_order(p1);

    //for (list<OrderedUnidirListItem>::const_iterator p = OrderedUnidirListItem::list.begin(); p != OrderedUnidirListItem::list.end(); ++p) {
    //    std::cout << p->n << ' ';
    //}
    //std::cout << std::endl;

    //p2->n = 3;
    //OrderedUnidirListItem::list.vacidate_order(p2);

    //for (list<OrderedUnidirListItem>::const_iterator p = OrderedUnidirListItem::list.begin(); p != OrderedUnidirListItem::list.end(); ++p) {
    //    std::cout << p->n << ' ';
    //}
    //std::cout << std::endl;

    //OrderedUnidirListItem::list.vacidate_order(p2);

    //for (list<OrderedUnidirListItem>::const_iterator p = OrderedUnidirListItem::list.begin(); p != OrderedUnidirListItem::list.end(); ++p) {
    //    std::cout << p->n << ' ';
    //}
    //std::cout << std::endl;

    new OrderedUnidirListItem(6);
    new OrderedUnidirListItem(9);
    new OrderedUnidirListItem(8);

    OrderedUnidirListItem::list.clear();

    delete p1;
    delete p2;
}

static TestRegister test("ordered_unidir_list", test_ordered_unidir_list);
