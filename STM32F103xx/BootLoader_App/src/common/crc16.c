/*
 * crc16.c
 *
 *  Created on: 2020年1月5日
 *      Author: Fox
 */

#include "common/crc16.h"
#include <stddef.h>

//modbus crc16 校验
uint16_t crc16(uint8_t *pBuff, uint16_t len)
{
    uint16_t i = 0, j = 0;
    uint16_t crc_result = 0xffff;
    for (i = 0; i < len; i++)
    {
        crc_result = crc_result ^ pBuff[i];
        for (j = 0; j < 8; j++)
        {
            if (crc_result & 0x01)
                crc_result = (crc_result >> 1) ^ 0xA001;
            else
                crc_result = crc_result >> 1;
        }
    }
    return crc_result;
}
//异或和校验
uint8_t XOR8(uint8_t *pBuff, uint16_t len)
{
    uint8_t xor = 0;
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        xor ^= pBuff[i];
    }
    return xor;
}
void *memSet(void *s, int c, uint16_t n)
{
    char *tmpS;
    if (NULL == s || n < 0)
        return NULL;
    tmpS = (char *)s;
    while (n-- > 0)
        *tmpS++ = c;
    return s;
}
