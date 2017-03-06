// Module.cpp

#include "tools/util_test/Common.h"

#include <util/daemon/Module.h>
using namespace framework::configure;

struct DependModule
    : util::daemon::ModuleBase<DependModule>
{
    DependModule(
        util::daemon::Daemon & daemon)
        : util::daemon::ModuleBase<DependModule>(daemon)
    {
        config();
        get_daemon();
        logger();
        io_svc();
    }

    virtual boost::system::error_code startup()
    {
        return boost::system::error_code();
    }

    virtual void shutdown()
    {
    }
};

struct MainModule
    : util::daemon::ModuleBase<MainModule>
{
    MainModule(
        util::daemon::Daemon & daemon)
        : util::daemon::ModuleBase<MainModule>(daemon)
        , depend_(util::daemon::use_module<DependModule>(daemon))
    {
        assert(util::daemon::has_module<DependModule>(daemon));
        assert(&depend_ == &util::daemon::use_module<DependModule>(daemon));

        config();
        get_daemon();
        logger();
        io_svc();
    }

    virtual boost::system::error_code startup()
    {
        return boost::system::error_code();
    }

    virtual void shutdown()
    {
    }

private:
    DependModule & depend_;
};

static void test_daemon_module(Config & conf)
{
    util::daemon::Daemon daemon;
    util::daemon::use_module<MainModule>(daemon);
    daemon.start();
}

static TestRegister test("daemon_module", test_daemon_module);
