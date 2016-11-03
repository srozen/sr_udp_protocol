#include <stdio.h>
#include <zconf.h>
#include "CUnit/Basic.h"
#include "functions.h"
#include "socket.h"

int init_suite(void) {
    return 0;
}
int clean_suite(void) {
    return 0;
}

void test_arguments_reading(void) {
    FILE * outFile = stdout;
    char openMode[] = "w";
    int port = 0;
    char * address = NULL;

    int argc = 5;
    char* argv[] = {"./receiver", "-f", "text.txt", "localhost", "12345"};

    int argc2 = 3;
    char* argv2[] = {"./receiver", "localhost", "12345"};

    int fullArgs = read_args(argc, argv, &address, &port, &outFile, openMode);
    fclose(outFile);
    int partialArgs = read_args(argc2, argv2, &address, &port, &outFile, openMode);

    if(fullArgs == 0 && partialArgs == 0) {
        CU_PASS("read_args succeeded.\n");
    } else
        CU_FAIL("read_args failed.\n");
}

void wrong_arguments_reading(void) {
    FILE * outFile = stdout;
    char openMode[] = "w";
    int port = 0;
    char * address = NULL;


    int argc = 2;
    char* argv[] = {"./receiver", "localhost"};

    int wrongArgs = read_args(argc, argv, &address, &port, &outFile, openMode);

    if(wrongArgs != 0) {
        CU_PASS("read_args succeeded.\n");
    } else
        CU_FAIL("read_args failed.\n");

}

void real_address_test(void) {
    char * address = "localhost";
    char * address2 = "";
    struct sockaddr_in6 addr;

    const char *err = real_address(address, &addr);
    const char *err2 = real_address(address2, &addr);

    if(err == 0 && err2 != 0) {
        CU_PASS("Real Address succeeded.\n");
    } else
        CU_FAIL("Real Address failed.\n");
}

void create_socket_test(void) {
    char * address = "localhost";
    int port = 12345;
    struct sockaddr_in6 addr;
    struct sockaddr_in6 addr2;
    real_address(address, &addr);

    int sfd_good = create_socket(NULL, -1, &addr, port);
    int sfd_bad = create_socket(&addr2, port, NULL, -1);
    fprintf(stderr, "Good : %d\n", sfd_good);
    fprintf(stderr, "bad : %d\n", sfd_bad);

    if(sfd_good > 0 && sfd_bad == -1) {
        CU_PASS("Create Socket succeeded.\n");
    } else
        CU_FAIL("Create Socket failed.\n");
}

void wait_for_client_test(void) {
    char * address = "localhost";
    int port = 12345;
    struct sockaddr_in6 rec_addr;
    struct sockaddr_in6 send_addr;
    real_address(address, &rec_addr);
    real_address(address, &send_addr);

    int send_sfd = create_socket(NULL, -1, &send_addr, port);
    int rec_sfd = create_socket(&rec_addr, port, NULL, -1);

    const char * buf = "Test";
    ssize_t written = write(send_sfd, buf, 5);
    int receive = wait_for_client(rec_sfd);

    if(receive >= 0 && written > 0) {
        CU_PASS("Create Socket succeeded.\n");
    } else
        CU_FAIL("Create Socket failed.\n");
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main(void) {
    CU_pSuite pSuite = NULL;

    /* initialize the CUnit test registry */
    if(CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    pSuite = CU_add_suite("Selective Repeat Protocol Suite", init_suite, clean_suite);
    if(NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* add the tests to the suite */
    if((NULL == CU_add_test(pSuite, "Read Args Test", test_arguments_reading)) ||
            (NULL == CU_add_test(pSuite, "Wrong Read Args Test", wrong_arguments_reading)) ||
            (NULL == CU_add_test(pSuite, "Real Address Conversion", real_address_test)) ||
            (NULL == CU_add_test(pSuite, "Create Socket", create_socket_test)) ||
            (NULL == CU_add_test(pSuite, "Create Socket", wait_for_client_test))

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
