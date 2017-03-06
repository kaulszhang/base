// Cycle.cpp

#include "tools/framework_test/Common.h"

#include "framework/container/Cycle.h"
using namespace framework::configure;
using namespace framework::container;

struct CycleItem
{
    CycleItem(
        size_t n = 0)
        : n(n)
    {
    }

    size_t n;

    static Cycle<CycleItem> cycle;
};

Cycle<CycleItem> CycleItem::cycle(10);

void test_cycle(Config & conf)
{
    CycleItem::cycle.push(4);
    CycleItem::cycle.push(1);
    CycleItem::cycle.push(3);
    CycleItem::cycle.push(5);
    CycleItem::cycle.push(2);

    std::cout << CycleItem::cycle.size() << std::endl;
    for (Cycle<CycleItem>::const_iterator p = CycleItem::cycle.begin(); p != CycleItem::cycle.end(); ++p) {
        std::cout << p->n << ' ';
    }
    std::cout << std::endl;

    CycleItem p1 = CycleItem::cycle.front();
    std::cout << p1.n;
    std::cout << std::endl;
    CycleItem::cycle.pop();

    std::cout << CycleItem::cycle.size() << std::endl;
    for (Cycle<CycleItem>::const_iterator p = CycleItem::cycle.begin(); p != CycleItem::cycle.end(); ++p) {
        std::cout << p->n << ' ';
    }
    std::cout << std::endl;
}

static TestRegister test("cycle", test_cycle);
