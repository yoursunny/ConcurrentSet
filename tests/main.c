#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

void
suite_api(void);

void
suite_hashtable(void);

int
main(void)
{
	CU_initialize_registry();

	suite_api();
	suite_hashtable();

	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	//CU_list_tests_to_file();
	CU_cleanup_registry();
	return 0;
}
