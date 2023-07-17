/*
 * handle_srec.c
 *
 *  Created on: Jul 17, 2023
 *      Author: hoanpx
 */
#include "handle_srec.h"
#include <stdlib.h>
#include <string.h>

/* Convert line to SREC format
 *
 * @param
 *
 *
 *
 */
uint8_t convert_to_srec(SREC *record, char *arr, uint32_t line) {
    size_t i;
    size_t data_offset;
    uint8_t checksum_c;
    if (arr[0] != 'S') {
        printf("Error in line %d : The first character must is 'S' \n", line);
        return 1;
    }
    /* Check for valid characters */
    for (i = 1; arr[i] != '\r' && arr[i] != '\n'; i++) {
        if (digit_to_hex(arr[i]) > 0x0F) {
            printf("Error in line %d : Non hexadecimal value \n", line);
            return 1;
        }
    }
    /* Get the record type */
    record->type = (digit_to_hex(arr[1]));
    if (record->type > S9) {
        printf("Error in line %d : Record type invalid \n", line);
        return 1;
    }
    /* Get the count field */
    record->byte_count = (digit_to_hex(arr[2]) << 4) | digit_to_hex(arr[3]);
    if (i < record->byte_count * 2 + MIN_RECORD_SIZE) {
        printf("Error in line %d : Byte count invalid \n", line);
        return 1;
    }
    /* Get the record address */
    switch (record->type) {
    case S0:
        record->address = 0;
        record->data_len = record->byte_count - 3;
        break;
    case S1:
    case S5:
    case S9:
        /*Four hex digits address */
        record->address = (digit_to_hex(arr[4]) << 12)
                | (digit_to_hex(arr[5]) << 8) | (digit_to_hex(arr[6]) << 4)
                | digit_to_hex(arr[7]);
        record->data_len = record->byte_count - 3;
        break;
    case S2:
    case S6:
    case S8:
        /* Six hex digits address */
        record->address = (digit_to_hex(arr[4]) << 20)
                | (digit_to_hex(arr[5]) << 16) | (digit_to_hex(arr[6]) << 12)
                | (digit_to_hex(arr[7]) << 8) | (digit_to_hex(arr[8]) << 4)
                | digit_to_hex(arr[9]);
        record->data_len = record->byte_count - 4;
        break;
    case S3:
    case S7:
        /* Eight hex digits address*/
        record->address = (digit_to_hex(arr[4]) << 28)
                | (digit_to_hex(arr[5]) << 24) | (digit_to_hex(arr[6]) << 20)
                | (digit_to_hex(arr[7]) << 16) | (digit_to_hex(arr[8]) << 12)
                | (digit_to_hex(arr[9]) << 8) | (digit_to_hex(arr[10]) << 4)
                | digit_to_hex(arr[11]);
        record->data_len = record->byte_count - 5;
        break;
        /* S4 record ignored */
    default:
        printf("Error in line %d : Record type invalid \n", line);
        break;
    }

    /* Calculate checksum and copy data bytes */
    checksum_c = record->byte_count + (record->address >> 24)
            + (record->address >> 16) + (record->address >> 8)
            + (record->address & 0xff);

    if (record->type < S4) {
        memset(record->data, 0x00, sizeof(record->data));
        data_offset = (record->byte_count - record->data_len) * 2 + 2;
        record->data_offset = data_offset;
        for (i = 0; i < record->data_len; i++) {
            record->data[i] = (digit_to_hex(arr[data_offset + i * 2]) << 4)
                    | digit_to_hex(arr[data_offset + i * 2 + 1]);
            checksum_c += record->data[i];
        }
    }

    checksum_c = ~checksum_c; /* bu 1 */
    record->check_sum = (digit_to_hex(arr[record->byte_count * 2 + 2]) << 4)
            | digit_to_hex(arr[record->byte_count * 2 + 3]);
    if (checksum_c != record->check_sum) {
        printf("Error in line %d : Checksum fail \n", line);
        return 1;
    }
    printf("Line %d is true SREC format \n", line);
    return 0;
}

/* Convert an ascii hex character
 *
 * @param
 *
 *
 * */
uint8_t digit_to_hex(const char hex) {
    if (hex >= '0' && hex <= '9')
        return hex - '0';
    else if (hex >= 'A' && hex <= 'F')
        return hex - 'A' + 10;
    else if (hex >= 'a' && hex <= 'f')
        return hex - 'a' + 10;
    else
        return 0xff;
}

/* Write data to file
 *
 * @param
 *
 *
 * */
uint8_t write_data_to_file(FILE *file, SREC *record) {
    uint32_t i;
    file = fopen("data.txt", "a");
    for (i = 0; (i < record->data_len); i++) {
        fprintf(file, "%02X", record->data[i]);
    }
    fprintf(file, "\r\n");
    fclose(file);
    return 0;
}

