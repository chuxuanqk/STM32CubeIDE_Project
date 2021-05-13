/*
 * callback_functions.c
 *
 *  Created on: 2020年1月5日
 *      Author: Fox
 */

#include <stddef.h>
#include "common/callback_functions.h"

struct Function_Loop_Str functions_loop_list[Max_Function_Loop] = {};

uint8_t Register_To_Functions_Loop(struct Function_Loop_Str *in_func)
{
	uint8_t i;
	//禁止重复注册
	for (i = 0; i < Max_Function_Loop; i++)
	{
		if (functions_loop_list[i].callback == in_func->callback)
		{
			functions_loop_list[i].callback = NULL;
		}
	}
	//定时器插入数组
	for (i = 0; i < Max_Function_Loop; i++)
	{
		if (functions_loop_list[i].callback == NULL)
		{
			functions_loop_list[i].callback = in_func->callback;
			functions_loop_list[i].param = in_func->param;
			return i;
		}
	}
	//无法再插入新定时器
	return -1;
};

uint8_t functions_loop_not_realtime(void)
{
	uint8_t i;
	//1ms实时定时器任务
	for (i = 0; i < Max_Function_Loop; i++)
	{
		if (functions_loop_list[i].callback != NULL)
			// (*functions_loop_list[i].callback)(); //不带参回调
			(*functions_loop_list[i].callback)(functions_loop_list[i].param); //带参回调
	}

	return 0;
}

uint8_t Unregister_From_Functions_Loop(struct Function_Loop_Str *in_func)
{
	uint8_t i;
	for (i = 0; i < Max_Function_Loop; i++)
	{
		if (functions_loop_list[i].callback == in_func->callback)
		{
			functions_loop_list[i].callback = NULL;
		}
	}
	return 0;
}
