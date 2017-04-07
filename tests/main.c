#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>

void suite_api(void);
void suite_hashtable(void);

int main(void)
{
	CU_initialize_registry();

	suite_api();
	suite_hashtable();

	CU_automated_run_tests();
	CU_list_tests_to_file();
	CU_cleanup_registry();
	return 0;
}
