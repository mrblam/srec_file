/*
 * handle_srec.h
 *
 *  Created on: Jul 17, 2023
 *      Author: hoanpx
 */

#ifndef HANDLE_SREC_HANDLE_SREC_H_
#define HANDLE_SREC_HANDLE_SREC_H_
#include <stdint.h>
#include <stdio.h>
/*************************/
#define MIN_RECORD_SIZE 4
#define ROW_SIZE    16
/*************************/
typedef enum {
    S0 = 0,
    S1,
    S2,
    S3,
    S4,
    S5,
    S6,
    S7,
    S8,
    S9
} srec_type;

typedef struct SREC_t SREC;
struct SREC_t {
    char header;
    srec_type type;
    uint8_t byte_count;
    uint8_t data_len;
    uint8_t data_offset;
    uint32_t address;
    uint8_t data[32];
    uint8_t check_sum;
};
/*******Declare**********/
uint8_t convert_to_srec(SREC *record, char *arr, uint32_t line);
uint8_t write_data_to_file(FILE *file, SREC *record);

#endif /* HANDLE_SREC_HANDLE_SREC_H_ */
