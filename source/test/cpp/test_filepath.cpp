#include "ccore/c_target.h"
#include "cbase/c_runes.h"
#include "cbase/c_allocator.h"
#include "ctime/c_datetime.h"

#include "cunittest/cunittest.h"

#include "cpath/c_filepath.h"
#include "cpath/private/c_root.h"

#include "cpath/test_allocator.h"

using namespace ncore;

UNITTEST_SUITE_BEGIN(filepath)
{
	UNITTEST_FIXTURE(main)
	{
        UNITTEST_ALLOCATOR;

		UNITTEST_FIXTURE_SETUP()
		{
		}

		UNITTEST_FIXTURE_TEARDOWN()
		{}

		UNITTEST_TEST(constructor1)
		{
			filepath_t p1;

			CHECK_TRUE(p1.isEmpty());
		}

		UNITTEST_TEST(constructor2)
		{
            npath::root_t reg;
            reg.init(Allocator);

			const char* str = "TEST:\\textfiles\\docs\\readme.txt";
			filepath_t p;
            reg.register_fullfilepath(str, p);

			CHECK_FALSE(p.isEmpty());

            reg.exit(Allocator);
		}

	}
}
UNITTEST_SUITE_END
