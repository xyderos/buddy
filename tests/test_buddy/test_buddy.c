#include "test_buddy.h"

Suite *
buddy_suite(void)
{
	Suite *s = suite_create("buddy");
	TCase *tc_core = tcase_create("core");

	suite_add_tcase(s, tc_core);
	tcase_add_test(tc_core, test_BuddyInit_Success);
	tcase_add_test(tc_core, test_BuddyInit_Fail);
	tcase_add_test(tc_core, test_BuddyAlloc_ValidSize);
	tcase_add_test(tc_core, test_FreeBuddyMemory);
	tcase_add_test(tc_core, test_BuddyDestroy);
	return s;
}
