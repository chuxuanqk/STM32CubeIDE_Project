/*
 * callback_functions.h
 *
 *  Created on: 2020年1月5日
 *      Author: Fox
 */

#ifndef COMMON_CALLBACK_FUNCTIONS_H_
#define COMMON_CALLBACK_FUNCTIONS_H_

#include "stm32f10x.h"

struct Function_Loop_Str
{
	//	void (*callback)(void); //不带参回调函数
	void (*callback)(void *param); //带参回调函数
	void *param;				   //回调函数参数
};

#define Max_Function_Loop 10 //非实时任务最大数量

uint8_t Register_To_Functions_Loop(struct Function_Loop_Str *in_func);
uint8_t Unregister_From_Functions_Loop(struct Function_Loop_Str *in_func);

uint8_t functions_loop_not_realtime(void);

#endif /* COMMON_CALLBACK_FUNCTIONS_H_ */
