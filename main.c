/*
 * main.c
 *
 *  Created on: Jul 17, 2023
 *      Author: hoanpx
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "handle_srec/handle_srec.h"
#define MAX_RECORD_LENGTH 256
extern uint8_t line_s0;
extern uint32_t line_s1;
extern uint32_t line_s2;
extern uint32_t line_s3;
extern uint8_t line_s5;
extern uint8_t line_s6;
extern uint8_t line_s7;
extern uint8_t line_s8;
extern uint8_t line_s9;
extern uint32_t s_line_of_s1s2s3;
int main() {
    SREC rec;
    uint32_t line = 0;
    char line_data[MAX_RECORD_LENGTH];
    FILE *file = NULL;
    FILE *data = NULL;
    file = fopen("srecord_example", "r");
    if (file == NULL) {
        printf("ERROR: Can not open file.\n");
        return 1;
    }
    /* Read each line in the file */
    while (fgets(line_data, MAX_RECORD_LENGTH, file) != NULL) {
        line++;
        if (!convert_to_srec(&rec, line_data, line)) {
            write_data_to_file(data, &rec);
        } else {
            printf("SREC file is corrupted!!!\nStop check.\n");
            fclose(file);
            return 1;
        }
    }
    /*check at least of s1 s2 s3 s4 s5 s6 s7 s8 s9*/
    if (line_s0 == 0) {
        printf("Error: Missing S0 line in file.\n");
        fclose(file);
        return 1;
    }
    if (s_line_of_s1s2s3 == 0) {
        printf("Error: Missing S1 S2 S3 line in file.\n");
        fclose(file);
        return 1;
    }
    if (line_s5 == 0 && line_s6 == 0) {
        printf("Error: Missing S5 S6 line in file.\n");
        fclose(file);
        return 1;
    }
    if (line_s7 == 0 && line_s8 == 0 && line_s9 == 0) {
        printf("Error: Missing S7 S8 S9 line in file.\n");
        fclose(file);
        return 1;
    }
    if (line_s1 != 0 && line_s9 == 0) {
        printf("Error: S1 must accompany S9\n");
        fclose(file);
        return 1;
    }
    if (line_s2 != 0 && line_s8 == 0) {
        printf("Error: S2 must accompany S8\n");
        fclose(file);
        return 1;
    }
    if (line_s3 != 0 && line_s7 == 0) {
        printf("Error: S3 must accompany S7\n");
        fclose(file);
        return 1;
    }
    printf("Check successfully. File SREC is valid.\n");
    fclose(file);
    return 0;
}

