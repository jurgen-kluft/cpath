#include "ccore/c_target.h"
#include "cbase/c_allocator.h"
#include "cbase/c_runes.h"
#include "ctime/c_datetime.h"

#include "cunittest/cunittest.h"

#include "cpath/c_dirpath.h"
#include "cpath/private/c_pathreg.h"

#include "cpath/test_allocator.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(dirpath)
{
	UNITTEST_FIXTURE(main)
	{
        UNITTEST_ALLOCATOR;

		UNITTEST_FIXTURE_SETUP()
		{
		}

		UNITTEST_FIXTURE_TEARDOWN()
		{
		}

		static const char*		sFolders[] = {
			"the",
			"name",
			"is",
			"johhnywalker",
		};

		UNITTEST_TEST(constructor1)
		{
			dirpath_t dirpath;
			CHECK_EQUAL(true,dirpath.isEmpty());
		}

		UNITTEST_TEST(constructor2)
		{
            pathreg_t reg;
            reg.init(Allocator);

			dirpath_t dirpath;
            reg.dirpath("c:\\the\\name\\is\\johhnywalker\\", dirpath);

			CHECK_EQUAL(false, dirpath.isEmpty());

            reg.exit(Allocator);
		}

		UNITTEST_TEST(to_string)
		{
            pathreg_t reg;
            reg.init(Allocator);

			const char* asciidirstr = "c:\\the\\name\\is\\johhnywalker\\";

			dirpath_t dirpath;
            reg.dirpath(asciidirstr, dirpath);
			nrunes::runestr_t<ascii::rune, 128> dirstr;
			dirpath.to_string(dirstr);
			CHECK_EQUAL(0, compare(dirstr, asciidirstr));

            reg.exit(Allocator);
		}
	}
}
UNITTEST_SUITE_END
