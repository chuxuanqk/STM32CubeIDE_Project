/*
 * crc16.h
 *
 *  Created on: 2020年1月5日
 *      Author: Fox
 */

#ifndef COMMON_CRC16_H_
#define COMMON_CRC16_H_

#include "stm32f10x.h"

uint16_t crc16(uint8_t *pBuff, uint16_t len);

#endif /* COMMON_CRC16_H_ */
