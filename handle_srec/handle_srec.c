/*
 * handle_srec.c
 *
 *  Created on: Jul 17, 2023
 *      Author: hoanpx
 */
#include "handle_srec.h"
#include <stdlib.h>
#include <string.h>
/***************Variable******************/
uint8_t line_s0 = 0;
uint32_t line_s1 = 0;
uint32_t line_s2 = 0;
uint32_t line_s3 = 0;
uint8_t line_s4 = 0;
uint8_t line_s5 = 0;
uint8_t line_s6 = 0;
uint8_t line_s7 = 0;
uint8_t line_s8 = 0;
uint8_t line_s9 = 0;
uint32_t s_line_of_s1s2s3 = 0;
/***************Decalre Static Functon******************/
static uint8_t count_line_type(SREC *rec);
static uint8_t digit_to_hex(const char hex);

/********Define Function******************/
/* Convert line to SREC format
 *
 * @param record: line SREC type
 *        arr: arr contain one line of file SREC
 *        line: line of file present
 * @retval 1: ERROR
 *         0: NOT ERROR
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
            printf("Error in line %d : Non hexadecimal value. \n", line);
            return 1;
        }
    }
    /* Get the record type */
    record->type = (digit_to_hex(arr[1]));
    if (line == 1 && record->type != S0) {
        printf("Error in line %d: Record type must be S0 in first.\n", line);
        return 1;
    }
    if (record->type > S9) {
        printf("Error in line %d : Record type invalid. \n", line);
        return 1;
    } else {
        if (count_line_type(record))
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
        if (record->type == S9 && record->byte_count != 3){
            printf("Error: Byte count S9 error\n");
            return 1;
        }
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
        if (record->type == S8 && record->byte_count != 4){
            printf("Error: Byte count S8 error\n");
            return 1;
        }
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
        if (record->type == S7 && record->byte_count != 5){
            printf("Error: Byte count S7 error\n");
            return 1;
        }
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
    } else if (record->type < S7) {
        if (record->type == S5 || record->type == S6) {
            /*Check line of s1 s2 s3*/
            if (record->address != s_line_of_s1s2s3) {
                printf("ERROR: Address of S%d not match in real line\n", record->type);
                return 1;
            }
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
 * @param hex: digit type char
 *
 * @retval hexa value
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
 * @param file: pointer point to file
 *        record: pointer to SREC type
 * @retval 0
 * */
uint8_t write_data_to_file(FILE *file, SREC *record) {
    uint32_t i;
    file = fopen("data.txt", "a");
    /*Wrire data of S1 or S2 or S3 */
    if (record->type == S1 || record->type == S2 || record->type == S3) {
        for (i = 0; (i < record->data_len); i++) {
            fprintf(file, "%02X", record->data[i]);
        }
        fprintf(file, "\r\n");
    }
    fclose(file);
    return 0;
}
/* Count line of type in file: Count and check error each line of file
 *
 * @param rec: line of SREC file
 *
 * @retval 1 if error
 *         0 if success
 * */
static uint8_t count_line_type(SREC *rec) {
    switch (rec->type) {
    case S0:
        line_s0++;
        if (line_s0 > 1) {
            printf("Error : Too many S0\n");
            return 1;
        }
        break;
    case S1:
        line_s1++;
        if (line_s2 != 0 || line_s3 != 0) {
            printf("S1 S2 S3 cannot exist at the same time\n");
            return 1;
        }
        break;
    case S2:
        line_s2++;
        if (line_s1 != 0 || line_s3 != 0) {
            printf("S1 S2 S3 cannot exist at the same time\n");
            return 1;
        }
        break;
    case S3:
        line_s3++;
        if (line_s2 != 0 || line_s1 != 0) {
            printf("S1 S2 S3 cannot exist at the same time\n");
            return 1;
        }
        break;
    case S4:
        line_s4++;
        printf("Error : S4 not use in SREC format\n");
        return 1;
        break;
    case S5:
        line_s5++;
        if (line_s5 > 1) {
            printf("Error : Too many S5\n");
            return 1;
        }
        if (line_s6 > 0) {
            printf("S5 S6 cannot exist at the same time\n");
            return 1;
        }
        if (s_line_of_s1s2s3 > 0xFFFF) {
            printf("Over range address of S5\n");
            return 1;
        }
        break;
    case S6:
        line_s6++;
        if (line_s6 > 1) {
            printf("Error : Too many S6\n");
            return 1;
        }
        if (line_s5 > 0) {
            printf("S5 S6 cannot exist at the same time\n");
            return 1;
        }
        if (s_line_of_s1s2s3 > 0xFFFFFF) {
            printf("Over range address of S6\n");
            return 1;
        }
        break;
    case S7:
        line_s7++;
        if (line_s7 > 1) {
            printf("Error : Too many S7\n");
            return 1;
        }
        if (line_s8 != 0 || line_s9 != 0) {
            printf("S7 S8 S9 cannot exist at the same time\n");
            return 1;
        }
        break;
    case S8:
        line_s8++;
        if (line_s8 > 1) {
            printf("Error : Too many S8\n");
            return 1;
        }
        if (line_s7 != 0 || line_s9 != 0) {
            printf("S7 S8 S9 cannot exist at the same time\n");
            return 1;
        }
        break;
    case S9:
        line_s9++;
        if (line_s9 > 1) {
            printf("Error : Too many S9\n");
            return 1;
        }
        if (line_s8 != 0 || line_s7 != 0) {
            printf("S7 S8 S9 cannot exist at the same time\n");
            return 1;
        }
        break;
    }
    s_line_of_s1s2s3 = line_s1 + line_s2 + line_s3;
    return 0;
}
