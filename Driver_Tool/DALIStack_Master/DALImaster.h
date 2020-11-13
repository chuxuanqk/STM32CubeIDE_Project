/*
 * DALImaster.h
 *
 *  Created on: 2020年9月27日
 *      Author: Saber
 */

#ifndef DALIMASTER_H_
#define DALIMASTER_H_

#include "stdint.h"
#include "stdbool.h"

enum MASTER_FLAG
{
    MASTER_NO_ACTION = 0,
    MASTER_SENDING_DATA,
    MASTER_RECEIVING_DATA,
    MASTER_ERR
};

// Configuration commands
#define CMD32 (0x0120)
#define CMD129 (0x0181)

// Query commands
#define CMD144 (0x0190)
#define CMD157 (0x019D)
#define CMD160 (0x01A0)
#define CMD165 (0x01A5)
#define CMD176 (0x01B0)
#define CMD197 (0x01C5)
#define CMD255 (0x01FF)

// Special commands
#define INITIALISE (0xA500) // command for starting initialization mode
#define RANDOMISE (0xA700)  // command for generating a random address
#define COMPARE (0xA900)
#define VERIFY_SHORT_ADDRESS (0xB901)
#define QUERY_SHORT_ADDRESS (0xBB00)

void hw_dalimaster_init(void);
uint8_t get_masterflag(void);
void master_send_data(uint16_t byteToSend);

bool Is_Wait_Answer(void);
uint8_t dali_read_backwardframe(void);

void dalimaster_test();
void dalimaster1_test(uint16_t cmd);

#endif /* DALIMASTER_H_ */
