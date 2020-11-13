/**
  ******************************************************************************
  * @file    dali_config.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    07-Dec-2012
  * @brief   User configuration of DALI hardware and stack - header
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

#ifndef DALI_CONFIG_H
#define DALI_CONFIG_H

#include "stm32f10x.h"

//callback function type for light control
typedef void TLightControlCallback(u16 lighvalue);

/* defines if device is physically selected */
extern volatile u8 Physically_Selected;

/* --- HARDWARE definitions --- */
/* IO pins for DALI in and DALI out signals */
#define OUT_DALI_PORT GPIOB //PB1 = TX
#define OUT_DALI_PIN GPIO_Pin_1
#define INVERT_OUT_DALI 0 //0 输出电平  1 输出电平相反

#define IN_DALI_PORT GPIOB //PB0 = RX
#define IN_DALI_PIN GPIO_Pin_0
#define INVERT_IN_DALI 1 //0 输入电平  1 输入电平相反

#define DALI_EXT_LINE EXTI_Line0
#define DALI_EXT_IRQn EXTI0_IRQn
#define DALI_ITR_PORT_SOURCE GPIO_PortSourceGPIOB
#define DALI_ITR_PORT_PIN_SOURCE GPIO_PinSource0

/* pushbutton for device physical selection */
#define DALI_BUTTON_PORT GPIOB //PB8 = button to GND
#define DALI_BUTTON_PIN 8

/* pull-up Vdd control for energy saving in halt */
#define DALI_PULLUP_PORT GPIOB //PB9 = Vdd for pull-up for LED in optocoupler
#define DALI_PULLUP_PIN 9

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/