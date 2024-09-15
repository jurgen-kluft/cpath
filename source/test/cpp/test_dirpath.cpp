#include "ccore/c_target.h"
#include "cbase/c_allocator.h"
#include "cbase/c_runes.h"
#include "ctime/c_datetime.h"

#include "cunittest/cunittest.h"

#include "cpath/c_dirpath.h"
#include "cpath/private/c_root.h"

#include "cpath/test_allocator.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(dirpath)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_ALLOCATOR;

        UNITTEST_FIXTURE_SETUP() {}

        UNITTEST_FIXTURE_TEARDOWN() {}

        static const char* sFolders[] = {
            "the",
            "name",
            "is",
            "johhnywalker",
        };

        UNITTEST_TEST(constructor1)
        {
            dirpath_t dirpath;
            CHECK_EQUAL(true, dirpath.isEmpty());
        }

        UNITTEST_TEST(constructor2)
        {
            npath::root_t reg;
            reg.init(Allocator);

            dirpath_t       dirpath;
            npath::string_t outdevicename;
            npath::node_t   outnode;
            reg.register_fulldirpath("c:\\the\\name\\is\\johhnywalker\\", outdevicename, outnode);

            CHECK_EQUAL(false, dirpath.isEmpty());

            reg.exit(Allocator);
        }

        UNITTEST_TEST(to_string)
        {
            npath::root_t reg;
            reg.init(Allocator);

            const char* asciidirstr = "c:\\the\\name\\is\\johhnywalker\\";

            npath::string_t outdevicename;
            npath::node_t   outnode;
            reg.register_fulldirpath(asciidirstr, outdevicename, outnode);
            dirpath_t dirpath(reg.get_device(outdevicename), outnode);

            utf32::rune dst_runes[256];
            dst_runes[0] = {0};
            dst_runes[1] = {0};
            runes_t dst(dst_runes, 0, 0, 256);

            dirpath.to_string(dst);
            CHECK_EQUAL(0, nrunes::compare(dst, asciidirstr));

            reg.exit(Allocator);
        }
    }
}
UNITTEST_SUITE_END
