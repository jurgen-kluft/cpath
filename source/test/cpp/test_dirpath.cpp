#include "ccore/c_target.h"
#include "cbase/c_allocator.h"
#include "cbase/c_runes.h"
#include "ctime/c_datetime.h"
#include "cvmem/c_virtual_memory.h"

#include "cunittest/cunittest.h"

#include "cpath/c_path.h"
#include "cpath/c_dirpath.h"
#include "cpath/c_device.h"

#include "cpath/test_allocator.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(dirpath)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_ALLOCATOR;

        UNITTEST_FIXTURE_SETUP()
        {
            // Initialize virtual memory
            nvmem::initialize();
        }
        UNITTEST_FIXTURE_TEARDOWN() {}

        static const char* sFolders[] = {
            "the",
            "name",
            "is",
            "johhnywalker",
        };

        UNITTEST_TEST(constructor1)
        {
            npath::paths_t* paths = npath::g_construct_paths(Allocator, 1024 * 1024 * 1024);

            dirpath_t dirpath(paths->m_devices->get_default_device());
            CHECK_EQUAL(true, dirpath.isEmpty());

            npath::g_destruct_paths(Allocator, paths);
        }

        UNITTEST_TEST(register_device)
        {
            npath::paths_t* paths = npath::g_construct_paths(Allocator, 1024 * 1024 * 1024);

            npath::device_t* device = paths->register_device(make_crunes("c:"));

            npath::g_destruct_paths(Allocator, paths);
        }

        UNITTEST_TEST(constructor2)
        {
            npath::paths_t* paths = npath::g_construct_paths(Allocator, 1024 * 1024 * 1024);

#ifdef TARGET_PC
            crunes_t fullpath = make_crunes("c:\\the\\name\\is\\johhnywalker\\");
#elif defined(TARGET_MAC)
            crunes_t fullpath = make_crunes("/volume/the/name/is/johhnywalker/");
#endif

            dirpath_t dirpath = paths->register_fulldirpath(fullpath);
            CHECK_FALSE(dirpath.isEmpty());

            npath::g_destruct_paths(Allocator, paths);
        }

        UNITTEST_TEST(to_string)
        {
            npath::paths_t* paths = npath::g_construct_paths(Allocator, 1024 * 1024 * 1024);

            // const char* asciidirstr = "c:\\the\\name\\is\\johhnywalker\\";

            // npath::string_t outdevicename;
            // npath::node_t   outnode;
            // paths->register_fulldirpath(asciidirstr, outdevicename, outnode);
            // dirpath_t dirpath(paths->get_device(outdevicename), outnode);

            // utf32::rune dst_runes[256];
            // dst_runes[0] = 0;
            // dst_runes[1] = 0;
            // runes_t dst(dst_runes, 0, 0, 256);

            // dirpath.to_string(dst);
            // CHECK_EQUAL(0, nrunes::compare(dst, asciidirstr));

            npath::g_destruct_paths(Allocator, paths);
        }
    }
}
UNITTEST_SUITE_END
