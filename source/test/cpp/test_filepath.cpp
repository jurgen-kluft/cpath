#include "ccore/c_target.h"
#include "cbase/c_runes.h"
#include "cbase/c_allocator.h"
#include "ctime/c_datetime.h"
#include "cvmem/c_virtual_memory.h"

#include "cunittest/cunittest.h"

#include "cpath/c_path.h"
#include "cpath/c_filepath.h"
#include "cpath/c_device.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(filepath)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_ALLOCATOR;

        UNITTEST_FIXTURE_SETUP()
        {
            nvmem::initialize();
        }
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_TEST(constructor1)
        {
            npath::paths_t* paths = npath::g_construct_paths(Allocator);

            filepath_t p1(paths->m_devices->get_default_device());

            CHECK_TRUE(p1.isEmpty());

            npath::g_destruct_paths(Allocator, paths);
        }

        UNITTEST_TEST(constructor2)
        {
            npath::paths_t* paths = npath::g_construct_paths(Allocator);

            // const char* str = "TEST:\\textfiles\\docs\\readme.txt";
            // filepath_t  p;

            // npath::string_t out_device;
            // npath::node_t   out_path;
            // npath::string_t out_filename;
            // npath::string_t out_extension;
            // paths.register_fullfilepath(str, out_device, out_path, out_filename, out_extension);

            // CHECK_FALSE(p.isEmpty());

            npath::g_destruct_paths(Allocator, paths);
        }
    }
}
UNITTEST_SUITE_END
