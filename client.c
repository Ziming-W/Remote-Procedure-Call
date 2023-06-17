#include "rpc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int exit_code = 0;

    rpc_client *state = rpc_init_client("::1", 3000);
    if (state == NULL) {
        exit(EXIT_FAILURE);
    }

    char name[] = "add3"; 
    rpc_handle *handle_add3 = rpc_find(state, name);
    if (handle_add3 == NULL) {
        fprintf(stderr, "ERROR: Function add3 does not exist\n");
    }
    else{
        printf("successfully found add3 on server\n"); 
    }

    rpc_handle *handle_multi2 = rpc_find(state, "multi2");
    if (handle_multi2 == NULL) {
        fprintf(stderr, "ERROR: Function multi2 does not exist\n");
    }
    else{
        printf("successfully found multi2 on server\n"); 
    }

    rpc_handle *handle_add2 = rpc_find(state, "add2");
    if (handle_add2 == NULL) {
        fprintf(stderr, "ERROR: Function add2 does not exist\n");
        exit_code = 1;
        goto cleanup;
    }
    else{
        printf("successfully found add2 on server\n"); 
    }

    for (int i = 127; i < 128; i++) {
        /* Prepare request */
        char left_operand = i;
        char right_operand = 127;
        rpc_data request_data = {
            .data1 = left_operand, .data2_len = 1, .data2 = &right_operand};

        /* Call and receive response */
        rpc_data *response_data = rpc_call(state, handle_add2, &request_data);
        if (response_data == NULL) {
            fprintf(stderr, "Function call of add2 failed \n");
            exit_code = 1;
            goto cleanup;
        }

        /* Interpret response */
        assert(response_data->data2_len == 0);
        assert(response_data->data2 == NULL);
        printf("Result of adding %d and %d: %d\n", left_operand, right_operand,
               response_data->data1);
        rpc_data_free(response_data);
    }

    for (int i = 0; i < 2; i++) {
        /* Prepare request */
        char left_operand = i;
        char right_operand = 100 ;
        rpc_data request_data = {
            .data1 = left_operand, .data2_len = 1, .data2 = &right_operand};

        /* Call and receive response */
        rpc_data *response_data = rpc_call(state, handle_multi2, &request_data);
        if (response_data == NULL) {
            fprintf(stderr, "Function call of multi2 failed\n");
            exit_code = 1;
            goto cleanup;
        }

        /* Interpret response */
        assert(response_data->data2_len == 0);
        assert(response_data->data2 == NULL);
        printf("Result of multipling %d and %d: %d\n", left_operand, right_operand,
               response_data->data1);
        rpc_data_free(response_data);
    }

cleanup:
    if (handle_add2 != NULL) {
        free(handle_add2);
    }

    rpc_close_client(state);
    state = NULL;

    return exit_code;
}
