#include "ccore/c_target.h"
#include "cbase/c_runes.h"
#include "cbase/c_allocator.h"
#include "ctime/c_datetime.h"

#include "cunittest/cunittest.h"

#include "cpath/private/c_parser.h"
#include "cpath/c_path.h"

#include "cpath/test_allocator.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(pathparser)
{
    UNITTEST_FIXTURE(mac)
    {
        UNITTEST_ALLOCATOR;

        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_TEST(simple)
        {
            npath::parser_t parser;

            const char* str = "/Volumes/textfiles/docs/readme.txt";
            crunes_t    p1(str);

            parser.parse(p1, npath::parser_t::MACOS);

            CHECK_EQUAL(true, parser.has_device());
            CHECK_EQUAL(true, parser.has_path());
            CHECK_EQUAL(true, parser.has_filename());
            CHECK_EQUAL(true, parser.has_extension());
        }

        UNITTEST_TEST(folders)
        {
            crunes_t fullfilepath("/Volumes/textfiles/docs/readme.txt");

            npath::parser_t parser;
            parser.parse(fullfilepath);

            npath::string_t out_device;
            npath::node_t   out_path;
            npath::string_t out_filename;
            npath::string_t out_extension;

            out_device = 0;
            out_path   = 0;
            if (parser.has_device())
            {
                // device
            }
            else if (parser.has_path())
            {
                // directory
            }

            out_filename = 0;
            if (parser.has_filename())
            {
                // filename
            }

            out_extension = 0;
            if (parser.has_extension())
            {
                // extension
            }
        }
    }
    UNITTEST_FIXTURE(windows)
    {
        UNITTEST_ALLOCATOR;

        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        UNITTEST_TEST(simple)
        {
            npath::parser_t parser;

            const char* str = "TEST:\\textfiles\\docs\\readme.txt";
            crunes_t    p1(str);

            parser.parse(p1, npath::parser_t::WINDOWS);

            CHECK_EQUAL(true, parser.has_device());
        }
    }
}
UNITTEST_SUITE_END
