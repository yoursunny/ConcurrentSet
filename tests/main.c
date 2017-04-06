#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>

void suite_api(void);

int main(void)
{
	CU_initialize_registry();

	suite_api();

	CU_automated_run_tests();
	CU_list_tests_to_file();
	CU_cleanup_registry();
	return 0;
}
