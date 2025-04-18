/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "driver/chip/hal_prcm.h"
#include "driver/chip/hal_adc.h"
#include <stdio.h>

__xip_text
void HAL_ADC_Set_VrefMode(ADC_VrefMode mode)
{
	uint8_t ext_ldo = !!HAL_GET_BIT(PRCM->SYS_LDO_SW_CTRL, PRCM_EXT_LDO_VOLT_BIT);
	if (mode < 2) {
		HAL_MODIFY_REG(ADC->CTRL, ADC_VREF_MODE_SEL_MASK, mode << ADC_VREF_MODE_SEL_SHIFT);
	} else if (ext_ldo) {
		HAL_MODIFY_REG(ADC->CTRL, ADC_VREF_MODE_SEL_MASK, ADC_VREF_MODE_3 << ADC_VREF_MODE_SEL_SHIFT);
	} else {
		HAL_MODIFY_REG(ADC->CTRL, ADC_VREF_MODE_SEL_MASK, ADC_VREF_MODE_2 << ADC_VREF_MODE_SEL_SHIFT);
	}
}

#ifdef __CONFIG_ROM

extern HAL_Status __HAL_ADC_Init(ADC_InitParam *initParam);

__xip_text
HAL_Status HAL_ADC_Init(ADC_InitParam *initParam)
{
	HAL_Status ret;

	ret = __HAL_ADC_Init(initParam);
	HAL_ADC_Set_VrefMode(initParam->vref_mode);
	HAL_CCM_GPADC_SetMClock(CCM_APB_PERIPH_CLK_SRC_HFCLK, CCM_PERIPH_CLK_DIV_N_1, CCM_PERIPH_CLK_DIV_M_1);

	return ret;
}

#endif /*__CONFIG_ROM*/
