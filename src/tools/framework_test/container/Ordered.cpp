// Ordered.cpp

#include "tools/framework_test/Common.h"

#include "framework/container/Ordered.h"
using namespace framework::configure;
using namespace framework::container;
using namespace framework::generic;

struct OrderedItem
    : OrderedHook<OrderedItem, NativePointerTraits<OrderedItem> >::type
{
    OrderedItem(
        size_t m, 
        size_t n = 0)
        : m(m)
        , n(n)
    {
        orders.insert(this);
    }

    bool operator < (
        OrderedItem const & r) const
    {
        return n < r.n;
    }

    size_t m;
    size_t n;

    typedef Ordered<
        OrderedItem, 
        identity<OrderedItem>, 
        std::less<OrderedItem>, 
        ordered_non_unique_tag
    > OrderedItemOrdered;

    static OrderedItemOrdered orders;
};

OrderedItem::OrderedItemOrdered OrderedItem::orders;

void test_ordered(Config & conf)
{
    std::cout << "add 7:3 2:3 3:3 4:3 5:3" << std::endl;
    new OrderedItem(7, 3);
    new OrderedItem(2, 3);
    OrderedItem * p1 = new OrderedItem(3, 3);
    OrderedItem * p2 = new OrderedItem(4, 3);
    new OrderedItem(5, 3);

    std::cout << "visit through iterator" << std::endl;
    std::cout << "size = " << OrderedItem::orders.size() << std::endl;
    for (Ordered<OrderedItem>::const_iterator p = OrderedItem::orders.cbegin(); p != OrderedItem::orders.cend(); ++p) {
        std::cout << p->m << ':' << p->n << ' ';
    }
    std::cout << std::endl;

    std::cout << "erase 4:3, print next item" << std::endl;
    OrderedItem::orders.erase(p2);

    std::cout << "visit through pointer" << std::endl;
    std::cout << "size = " << OrderedItem::orders.size() << std::endl;
    for (OrderedItem::pointer p = OrderedItem::orders.first(); p; p = OrderedItem::orders.next(p)) {
        std::cout << p->m << ':' << p->n << ' ';
    }
    std::cout << std::endl;

    std::cout << "reverse visit through pointer" << std::endl;
    std::cout << "size = " << OrderedItem::orders.size() << std::endl;
    for (OrderedItem::const_pointer p = OrderedItem::orders.last(); p; p = OrderedItem::orders.prev(p)) {
        std::cout << p->m << ':' << p->n << ' ';
    }
    std::cout << std::endl;

    std::cout << "erase 3:3, print next item" << std::endl;
    p1 = OrderedItem::orders.erase(p1);
    std::cout << p1->m << ':' << p1->n;
    std::cout << std::endl;

    std::cout << "reverse visit through pointer" << std::endl;
    std::cout << "size = " << OrderedItem::orders.size() << std::endl;
    for (OrderedItem::const_pointer p = OrderedItem::orders.last(); p; p = OrderedItem::orders.prev(p)) {
        std::cout << p->m << ':' << p->n << ' ';
    }
    std::cout << std::endl;

    std::cout << "add 6:0 9:0 8:0" << std::endl;
    new OrderedItem(6);
    new OrderedItem(9);
    new OrderedItem(8);

    std::cout << "reverse visit through pointer" << std::endl;
    std::cout << OrderedItem::orders.size() << std::endl;
    for (OrderedItem::const_pointer p = OrderedItem::orders.last(); p; p = OrderedItem::orders.prev(p)) {
        std::cout << p->m << ':' << p->n << ' ';
    }
    std::cout << std::endl;

    std::cout << "erase all, print before erase" << std::endl;
    std::cout << OrderedItem::orders.size() << std::endl;
    for (OrderedItem * p = OrderedItem::orders.first(); p; p = OrderedItem::orders.erase(p)) {
        std::cout << p->m << ':' << p->n << ' ';
    }

    delete p1;
    delete p2;

    std::cout << "add 7:0" << std::endl;
    OrderedItem a(7);

    std::cout << "reverse visit through pointer" << std::endl;
    std::cout << OrderedItem::orders.size() << std::endl;
    for (OrderedItem::const_pointer p = OrderedItem::orders.last(); p; p = OrderedItem::orders.prev(p)) {
        std::cout << p->m << ':' << p->n << ' ';
    }
    std::cout << std::endl;

    std::cout << "erase 7:0, re-add 7:0" << std::endl;
    OrderedItem::orders.erase(a);
    OrderedItem::orders.insert(&a);

    std::cout << "reverse visit through pointer" << std::endl;
    std::cout << OrderedItem::orders.size() << std::endl;
    for (OrderedItem::const_pointer p = OrderedItem::orders.last(); p; p = OrderedItem::orders.prev(p)) {
        std::cout << p->m << ':' << p->n << ' ';
    }
    std::cout << std::endl;

    OrderedItem::orders.erase(a);
}

static TestRegister test("ordered", test_ordered);
