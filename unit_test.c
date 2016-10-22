#include "CUnit/Basic.h"
#include "functions.h"

int init_suite(void) { return 0; }
int clean_suite(void) { return 0; }

void test_arguments_reading(void){
    FILE * outFile = stdout;
    char openMode[] = "w";
    int port = 0;
    char * address = NULL;
    int argc = 5;
    char* argv[] = {"./receiver", "-f", "text.txt", "localhost", "12345"};

    int argc2 = 3;
    char* argv2[] = {"./receiver", "localhost", "12345"};

    int fullArgs = read_args(argc, argv, &address, &port, &outFile, openMode);
    int partialArgs = read_args(argc2, argv2, &address, &port, &outFile, openMode);

    if (fullArgs == 0 && partialArgs == 0) {
        CU_PASS("read_args succeeded.\n");
    }
    else
        CU_FAIL("read_args failed.\n");
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main ( void ) {
    CU_pSuite pSuite = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    pSuite = CU_add_suite("Selective Repeat Protocol Suite", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "Read Args Test", test_arguments_reading))
            ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Run all tests using the basic interface
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    printf("\n");
    CU_basic_show_failures(CU_get_failure_list());
    printf("\n\n");
/*
   // Run all tests using the automated interface
   CU_automated_run_tests();
   CU_list_tests_to_file();

   // Run all tests using the console interface
   CU_console_run_tests();
*/
    /* Clean up registry and return */
    CU_cleanup_registry();
    return CU_get_error();
}