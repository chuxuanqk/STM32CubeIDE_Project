/*
 * crc16.c
 *
 *  Created on: 2020年1月5日
 *      Author: Fox
 */

#include <stddef.h>
#include "crc16.h"

//modbus crc16 校验
u16 crc16(u8 *pBuff, u16 len)
{
    u16 i = 0, j = 0;
    u16 crc_result = 0xffff;
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
u8 XOR8(u8 *pBuff, u16 len)
{
    u8 xor = 0;
    u16 i;
    for (i = 0; i < len; i++)
    {
        xor ^= pBuff[i];
    }
    return xor;
}
void *memSet(void *s, int c, u16 n)
{
    char *tmpS;
    if (NULL == s || n < 0)
        return NULL;
    tmpS = (char *)s;
    while (n-- > 0)
        *tmpS++ = c;
    return s;
}
