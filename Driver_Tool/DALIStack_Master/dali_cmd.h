/*
 * dali_cmd.h
 *
 *  Created on: 2020年10月17日
 *      Author: Saber
 */

#ifndef DALI_CMD_H_
#define DALI_CMD_H_

#include <stdint.h>

#define DALI_BORADCAST_ADDR 0xFF
#define DIMLEVEL_MAX 0xFE
#define DIMLEVEL_MIN 0x00

void Cmd_SetDimLevel(uint8_t short_addr, uint8_t dim_level);
void Cmd_SetTimeRate(uint8_t short_addr, uint8_t time_rate);
void Cmd_SetFateRate(uint8_t short_addr, uint8_t fate_rate);
void Cmd_SetTemperatureColour(uint8_t short_addr);
void Cmd_SetPowerOnLevel(uint8_t short_addr, uint8_t power_on_level);
void Cmd_SetSystemFailureLevel(uint8_t short_addr, uint8_t dim_level);

void Cmd_QueryDimLevel(uint8_t short_addr);
void Cmd_QueryFateAndTimeRate(uint8_t short_addr);
void Cmd_QueryPowerOnLevel(uint8_t short_addr);
void Cmd_QueryDeviceType(uint8_t short_addr);
void Cmd_QueryContentDTR1(uint8_t short_addr);
void Cmd_QueryContentDTR2(uint8_t short_addr);

/* Special Cmd */
void Cmd_InitialiseAddr(uint8_t short_addr);
void Cmd_DelShortAddr(uint8_t short_addr);
void Update_ShortAddr(uint8_t old_addr, uint8_t new_addr);

#endif /* DALI_CMD_H_ */
