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


int main() {
    SREC rec;
    uint32_t line = 0;
    char record[MAX_RECORD_LENGTH];
    FILE* file = NULL;
    FILE* data = NULL;
    file = fopen("srecord_example","r");
    if (file == NULL) {
        printf("ERROR: Can not open file.\n");
        return 1;
    }
    /* Read each line in the file */
    while (fgets(record, MAX_RECORD_LENGTH, file) != NULL) {
        line++;
        if(!convert_to_srec(&rec,record,line)){
            write_data_to_file(data, &rec);
        }else{
            printf("SREC file is corrupted.\n");
            return 1;
        }
    }
    printf("Check successfully. File SREC is valid.\n");
    fclose(file);
    return 0;
}
