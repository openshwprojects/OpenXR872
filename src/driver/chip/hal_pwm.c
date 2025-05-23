/**
 * @file  hal_pwm.c
 * @author  XRADIO IOT WLAN Team
 */

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

#include "hal_base.h"
#include "driver/chip/hal_pwm.h"

#define DBG_PWM    0
#define PWM_DBG(fmt, arg...)    \
	HAL_LOG(HAL_DBG_ON && DBG_PWM, "[HAL PWM] "fmt, ##arg)


#define MAX_ENTIRE_CYCLE 65536
#define MAX_PWM_PRESCALE 256
#define RISECH(ch)    (HAL_BIT(ch) * HAL_BIT(ch))
#define FALLCH(ch)    (HAL_BIT(ch) * HAL_BIT(ch) * 2)
#define PWM_IRQ_ALL_BITS    ((1 << (PWM_CH_NUM << 1)) - 1)

static uint8_t IoInitCount = 0;
static PWM_IrqParam PWM_IrqPrivate[8];

/**
 * @internal
 * @brief eanble pwm module.
 * @note This function is used to enable the pwm module source clock.
 * @param none.
 * @retval none.
 */
static void PWM_ModuleEnable(void)
{
	HAL_CCM_BusDisablePeriphClock(CCM_BUS_PERIPH_BIT_PWM);
	HAL_CCM_BusForcePeriphReset(CCM_BUS_PERIPH_BIT_PWM);
	HAL_CCM_BusReleasePeriphReset(CCM_BUS_PERIPH_BIT_PWM);
	HAL_CCM_BusEnablePeriphClock(CCM_BUS_PERIPH_BIT_PWM);
}

/**
 * @internal
 * @brief disable pwm module.
 * @note This function is used to disable the pwm module source clock.
 * @param none.
 * @retval none.
 */
static void PWM_ModuleDisable(void)
{
	HAL_CCM_BusDisablePeriphClock(CCM_BUS_PERIPH_BIT_PWM);
}

static PWM_GROUP_ID PWM_ChToGroup(PWM_CH_ID ch_id)
{
	if (ch_id >= PWM_CH_NUM)
		return PWM_GROUP_NULL;

	return ch_id / 2;
}

static PWM_CH_ID PWM_GroupToCh0(PWM_GROUP_ID group_id)
{
	if (group_id >= PWM_GROUP_NUM)
		return PWM_GROUP_NULL;

	return group_id * 2;
}

static PWM_CH_ID PWM_GroupToCh1(PWM_GROUP_ID group_id)
{
	if (group_id >= PWM_GROUP_NUM)
		return PWM_GROUP_NULL;

	return group_id * 2 + 1;
}

static uint32_t PWM_ReadGroupClkFreq(PWM_GROUP_ID group_id)
{
	if (group_id >= PWM_GROUP_NUM)
		return 0;

	uint32_t src_clk_freq = 0;
	__IO uint32_t *reg = &PWM->PCCR[group_id];
	uint32_t clk = *reg & PWM_SRC_CLK_SELECT;
	uint32_t div = *reg & PWM_SRC_CLK_DIV_V;

	if (clk == 0) {
		src_clk_freq = HAL_GetHFClock();
		PWM_DBG("%s, %d HRCLK\n", __func__, __LINE__);
	} else {
		src_clk_freq =  HAL_GetAPBClock();
		PWM_DBG("%s, %d APBCLK\n", __func__, __LINE__);
	}

	src_clk_freq /= (1 << div);

	return src_clk_freq;
}


static int PWM_ChClkDiv(PWM_CH_ID ch_id, PWM_ChInitParam *param)
{
	if (ch_id >= PWM_CH_NUM)
		return 0;

	int ch_clk_freq = 0;
	uint32_t minFreq = 0;
	uint32_t maxFreq = 0;
	uint8_t ch_div = 0;
	uint32_t temp1 = 0, temp2 = 0;
	__IO uint32_t *reg = NULL;
	uint32_t src_clk_freq = PWM_ReadGroupClkFreq(PWM_ChToGroup(ch_id));

	PWM_DBG("SRC_CLK freq = %u\n", src_clk_freq);

	if ((src_clk_freq % MAX_ENTIRE_CYCLE) > 0)
		temp1 = 1;
	if (((src_clk_freq / MAX_ENTIRE_CYCLE + temp1) % MAX_PWM_PRESCALE) > 0)
		temp2 = 1;

	minFreq = (src_clk_freq / MAX_ENTIRE_CYCLE + temp1) / MAX_PWM_PRESCALE + temp2;
	maxFreq = src_clk_freq;

	if (param->hz > maxFreq || param->hz < minFreq)
		return 0;

	if (param->hz > (src_clk_freq / MAX_ENTIRE_CYCLE + temp1))
		ch_div = 0;
	else {
		ch_div = (src_clk_freq / MAX_ENTIRE_CYCLE + temp1) % param->hz;
		if (ch_div)
			ch_div =  (src_clk_freq / MAX_ENTIRE_CYCLE + temp1) / param->hz;
		else
			ch_div =  (src_clk_freq / MAX_ENTIRE_CYCLE + temp1) / param->hz - 1;
	}

	PWM_DBG("ch div = %d\n", ch_div);

	if (ch_div > (MAX_PWM_PRESCALE - 1))
		return 0;

	reg = &PWM->CH_REG[ch_id].PCR;
	uint32_t temp = *reg;
	temp &= ~PWM_PCR_PRESCAL;
	temp |= ch_div;
	*reg = temp;      //Source clock Frequency Division

	ch_clk_freq = src_clk_freq / (ch_div + 1);

	return ch_clk_freq;
}


static void PWM_ChSetPolarity(PWM_CH_ID ch_id, PWM_ChInitParam *param)
{
	if (ch_id >= PWM_CH_NUM)
		return;

	__IO uint32_t *reg = NULL;

	reg = &PWM->CH_REG[ch_id].PCR;

	if (param->polarity == PWM_LOWLEVE)
		HAL_CLR_BIT(*reg, PWM_PCR_ACT_STA);
	else if (param->polarity == PWM_HIGHLEVE)
		HAL_SET_BIT(*reg, PWM_PCR_ACT_STA);
}

static void PWM_ChSetMode(PWM_CH_ID ch_id, PWM_ChInitParam *param)
{
	if (ch_id >= PWM_CH_NUM)
		return;

	__IO uint32_t *reg = NULL;
	reg = &PWM->CH_REG[ch_id].PCR;

	if (param->mode == PWM_PLUSE_MODE)
		HAL_SET_BIT(*reg, PWM_PCR_MODE);
	else if (param->mode == PWM_CYCLE_MODE)
		HAL_CLR_BIT(*reg, PWM_PCR_MODE);
}

static int PWM_OutPeriodRady(PWM_CH_ID ch_id)
{
	if (ch_id >= PWM_CH_NUM)
		return -1;
	__IO uint32_t *reg = NULL;
	reg = &PWM->CH_REG[ch_id].PCR;

	return HAL_GET_BIT(*reg, PWM_PCR_PERIODRDY);

}

static void PWM_OutSetCycle(PWM_CH_ID ch_id, uint16_t value)
{

	if (ch_id >= PWM_CH_NUM)
		return;

	__IO uint32_t *reg = &PWM->CH_REG[ch_id].PPR;;

	uint32_t temp = *reg;
	temp &= ~PWM_PPR_ENTIER_CYCLE;
	temp |= (value << 16);
	*reg = temp;
}

static void PWM_OutByPass(PWM_CH_ID ch_id)
{
	PWM_GROUP_ID group_id;
	group_id = PWM_ChToGroup(ch_id);
	__IO uint32_t *reg = &PWM->PCCR[group_id];

	if (ch_id % 2)
		HAL_SET_BIT(*reg, PWM_HIGH_CH_CLKBYPASS);
	else
		HAL_SET_BIT(*reg, PWM_LOW_CH_CLKBYPASS);
}

static int PWM_OutModeInit(PWM_CH_ID ch_id, PWM_ChInitParam *param)
{
	int ch_clk_freq = 0;
	int ch_entire_cycle = 0;

	ch_clk_freq = PWM_ChClkDiv(ch_id, param);

	PWM_DBG("ch freq = %d\n", ch_clk_freq);

	if (ch_clk_freq == 0)
		return -1;

	if (param->hz > ch_clk_freq)
		return -1;

	ch_entire_cycle = ch_clk_freq / param->hz;
	if (ch_clk_freq % param->hz >= param->hz / 2)
		ch_entire_cycle++;

	PWM_DBG("request freq:%uHZ, actual freq:%dHZ\n", param->hz, ch_clk_freq / ch_entire_cycle);

	if (ch_entire_cycle == 1) {
		if (param->mode == PWM_CYCLE_MODE) {
			PWM_OutByPass(ch_id);
			return ch_entire_cycle;
		} else {
			return -1;
		}
	}

	PWM_ChSetPolarity(ch_id, param);
	PWM_ChSetMode(ch_id, param);

	while (PWM_OutPeriodRady(ch_id))
		HAL_MSleep(1);

	PWM_OutSetCycle(ch_id, ch_entire_cycle - 1);

	return ch_entire_cycle;
}

static int PWM_InputInit(PWM_CH_ID ch_id, PWM_ChInitParam *param)
{
	int clk_freq =  PWM_ChClkDiv(ch_id, param);

	if (clk_freq == 0) {
		HAL_ERR("Hz out of range\n");
		return -1;
	}

	PWM_ChSetPolarity(ch_id, param);
	PWM_ChSetMode(ch_id, param);

	return 0;
}

static int PWM_EnableClock(PWM_CH_ID ch_id, uint8_t en)
{
	PWM_GROUP_ID group = PWM_ChToGroup(ch_id);

	__IO uint32_t *reg = NULL;

	if (group >= PWM_GROUP_NUM)
		return -1;

	reg = &PWM->PCCR[group];

	if (en == 1)
		HAL_SET_BIT(*reg, PWM_CH_CLK_GATING);
	else
		HAL_CLR_BIT(*reg, PWM_CH_CLK_GATING);

	return 0;
}

static void PWM_Init(PWM_CH_ID ch_id)
{
	int is_init = IoInitCount & (1 << ch_id);

	if (!is_init) {
		HAL_BoardIoctl(HAL_BIR_PINMUX_INIT, HAL_MKDEV(HAL_DEV_MAJOR_PWM, ch_id), 0);
		IoInitCount |= 1 << ch_id;
	}
}

static void PWM_DeInit(PWM_CH_ID ch_id)
{


	int is_init = IoInitCount & (1 << ch_id);
	PWM_CH_ID brother;

	if (ch_id % 2)
		brother = ch_id - 1;
	else
		brother = ch_id + 1;

	int brother_is_init = IoInitCount & (1 << brother);

	if (is_init) {
		HAL_BoardIoctl(HAL_BIR_PINMUX_DEINIT, HAL_MKDEV(HAL_DEV_MAJOR_PWM, ch_id), 0);

		if (!brother_is_init)
			PWM_EnableClock(ch_id, 0);

		IoInitCount &= ~(1 << ch_id);

		if (IoInitCount == 0) {
			PWM_DBG("pwm module disable\n");
			PWM_ModuleDisable();
		}
	}
}

__STATIC_INLINE void PWM_EnableOutputIRQ(PWM_CH_ID ch_id, uint8_t en)
{
	if (en)
		HAL_SET_BIT(PWM->PIER, HAL_BIT(ch_id));
	else
		HAL_CLR_BIT(PWM->PIER, HAL_BIT(ch_id));
}

__STATIC_INLINE void PWM_EnableInuptRiseEdgeIRQ(PWM_CH_ID ch_id, uint8_t en)
{
	if (en)
		HAL_SET_BIT(PWM->CIER,  RISECH(ch_id));
	else
		HAL_CLR_BIT(PWM->CIER,  RISECH(ch_id));
}

__STATIC_INLINE void PWM_EnableInuputFallEdgeIRQ(PWM_CH_ID ch_id, uint8_t en)
{
	if (en)
		HAL_SET_BIT(PWM->CIER, FALLCH(ch_id));
	else
		HAL_CLR_BIT(PWM->CIER, FALLCH(ch_id));
}

__STATIC_INLINE void PWM_EnableInputBothEdgeIRQ(PWM_CH_ID ch_id, uint8_t en)
{
	if (en)
		HAL_SET_BIT(PWM->CIER, (RISECH(ch_id) | FALLCH(ch_id)));
	else
		HAL_CLR_BIT(PWM->CIER, (RISECH(ch_id) | FALLCH(ch_id)));
}

static uint32_t PWM_CycleValue(PWM_CH_ID ch_id)
{
	__IO uint32_t *reg;

	reg = &PWM->CH_REG[ch_id].PPR;

	return ((*reg & PWM_PPR_ENTIER_CYCLE) >> 16) + 1;
}

static void PWM_SetActCycle(PWM_CH_ID ch_id, uint16_t cycle)
{
	__IO uint32_t *reg;
	reg = &PWM->CH_REG[ch_id].PPR;

	uint32_t p = *reg;
	p &= ~PWM_PPR_ACT_CYCLE;
	p |= cycle;
	*reg = p;
}

static void PWM_EnableDeadZone(PWM_GROUP_ID group_id, uint8_t en)
{
	__IO uint32_t *reg = NULL;

	reg = &PWM->PDZCR[group_id];

	if (en)
		HAL_SET_BIT(*reg, PWM_CH_DZ_EN);
	else
		HAL_CLR_BIT(*reg, PWM_CH_DZ_EN);
}

static int PWM_FallEdgeLock(PWM_CH_ID ch_id)
{
	__IO uint32_t *reg = &PWM->CH_REG[ch_id].CCR;;

	return *reg & PWM_CCR_CFLF;
}

static void PWM_ClearFallEdgeLock(PWM_CH_ID ch_id)
{
	__IO uint32_t *reg = &PWM->CH_REG[ch_id].CCR;

	*reg |= PWM_CCR_CFLF;
}

static int PWM_RiseEdgeLock(PWM_CH_ID ch_id)
{
	__IO uint32_t *reg = &PWM->CH_REG[ch_id].CCR;

	return *reg & PWM_CCR_CRLF;
}

static void PWM_ClearRiseEdgeLock(PWM_CH_ID ch_id)
{
	__IO uint32_t *reg = &PWM->CH_REG[ch_id].CCR;
	if (reg == NULL)
		return;
	*reg |= PWM_CCR_CRLF;
}

static uint16_t PWM_CRLRValue(PWM_CH_ID ch_id)
{
	__IO uint32_t *reg = &PWM->CH_REG[ch_id].CRLR;

	return ((uint16_t)(*reg & PWM_CRLR));
}

static uint16_t PWM_CFLRValue(PWM_CH_ID ch_id)
{
	__IO uint32_t *reg = &PWM->CH_REG[ch_id].CFLR;

	return ((uint16_t)(*reg & PWM_CFLR));
}

__nonxip_text
static void PWM_OutIRQHandle(void)
{
	uint32_t i;
	uint32_t status;
	uint32_t is_irq;
	PWM_IrqParam *irq;

	status =  PWM->PISR & PWM->PIER & PWM_IRQ_ALL_BITS; /* get pending bits */
	PWM->PISR = status; /* clear IRQ bits */
	irq = PWM_IrqPrivate;

	for (i = PWM_GROUP0_CH0; i < PWM_CH_NUM && status != 0; i++) {
		is_irq = status & HAL_BIT(0);
		if (is_irq && irq[i].callback) {
			PWM_IrqEvent event = PWM_IRQ_OUTPUT;
			irq[i].callback(irq[i].arg, event);
		}
		status >>= 1;
	}
}

__nonxip_text
static void PWM_InputIRQHandle(void)
{
	uint32_t i;
	uint32_t status;
	uint32_t rise_edge;
	uint32_t fall_edge;
	PWM_IrqParam *irq;
	status =  PWM->CISR & PWM->CIER & PWM_IRQ_ALL_BITS; /* get pending bits */
	PWM->CISR = status; /* clear IRQ bits */
	irq = PWM_IrqPrivate;

	for (i = PWM_GROUP0_CH0; i < PWM_CH_NUM && status != 0; i++) {
		rise_edge = status & HAL_BIT(0);
		if (rise_edge && irq[i].callback) {
			PWM_IrqEvent event = PWM_IRQ_RISEEDGE;
			irq[i].callback(irq[i].arg, event);
		}
		fall_edge = status & HAL_BIT(1);
		if (fall_edge && irq[i].callback) {
			PWM_IrqEvent event = PWM_IRQ_FALLEDGE;
			irq[i].callback(irq[i].arg, event);
		}
		status >>= 2;
	}
}

__nonxip_text
void PWM_ECT_IRQHandler(void)
{
	if (PWM->CIER > 0)
		PWM_InputIRQHandle();

	if (PWM->PIER > 0)
		PWM_OutIRQHandle();
}

static void PWM_EnableModuleIRQ(void)
{
	HAL_NVIC_ConfigExtIRQ(PWM_ECT_IRQn, PWM_ECT_IRQHandler, NVIC_PERIPH_PRIO_DEFAULT);
}

static void PWM_DisableModuleIRQ(void)
{
	HAL_NVIC_DisableIRQ(PWM_ECT_IRQn);
}

HAL_Status HAL_PWM_GroupClkCfg(PWM_GROUP_ID group_id, PWM_ClkParam *param)
{
	if (group_id >= PWM_GROUP_NUM)
		return HAL_ERROR;

	if (IoInitCount == 0) {
		PWM_ModuleEnable();
	}

	__IO uint32_t *reg = &PWM->PCCR[group_id];

	if (param->clk == PWM_CLK_HOSC) {
		*reg &= ~PWM_SRC_CLK_SELECT;
	} else if (param->clk == PWM_CLK_APB1) {
		HAL_MODIFY_REG(*reg, PWM_SRC_CLK_SELECT, HAL_BIT(7));
	} else
		return HAL_ERROR;

	if (param->div > PWM_SRC_CLK_DIV_256 || param->div < 0)
		return HAL_ERROR;

	uint32_t temp = *reg;
	temp &= ~PWM_SRC_CLK_DIV_V;
	temp |= param->div;
	*reg = temp;

	return HAL_OK;
}

int HAL_PWM_ChInit(PWM_CH_ID ch_id, PWM_ChInitParam *param)
{
	int duty_ratio = -1;
	if (ch_id >= PWM_CH_NUM)
		return -1;

	PWM_EnableClock(ch_id, 1);

	PWM_Init(ch_id);

	switch (param->mode) {
	case PWM_CYCLE_MODE:
	case PWM_PLUSE_MODE:
		duty_ratio = PWM_OutModeInit(ch_id, param);
		break;
	case PWM_CAPTURE_MODE:
		duty_ratio = PWM_InputInit(ch_id, param);
		break;
	default:
		return -1;
	}

	return duty_ratio;
}

HAL_Status HAL_PWM_ChDeinit(PWM_CH_ID ch_id)
{
	if (ch_id >= PWM_CH_NUM)
		return HAL_ERROR;

	PWM_DeInit(ch_id);

	return HAL_OK;
}

int HAL_PWM_ComplementaryInit(PWM_GROUP_ID group_id, PWM_CompInitParam *param)
{
	if (group_id >= PWM_GROUP_NUM)
		return -1;

	int cycle_value = -1;
	PWM_CH_ID ch_low = group_id * 2;
	PWM_CH_ID ch_high = ch_low + 1;
	PWM_ChInitParam ch_param;

	PWM_EnableClock(ch_low, 1);
	PWM_EnableClock(ch_high, 1);

	PWM_Init(ch_low);
	PWM_Init(ch_high);

	ch_param.hz = param->hz;
	ch_param.polarity = param->polarity;
	ch_param.mode = PWM_CYCLE_MODE;

	cycle_value = PWM_OutModeInit(ch_low, &ch_param);
	if (cycle_value == -1)
		return -1;

	if (ch_param.polarity == PWM_LOWLEVE)
		ch_param.polarity = PWM_HIGHLEVE;
	else if (ch_param.polarity == PWM_HIGHLEVE)
		ch_param.polarity = PWM_LOWLEVE;
	else
		return -1;

	PWM_OutModeInit(ch_high, &ch_param);

	return cycle_value;
}

HAL_Status HAL_PWM_ComplementaryDeInit(PWM_GROUP_ID group_id)
{
	if (group_id >= PWM_GROUP_NUM)
		return HAL_ERROR;

	PWM_CH_ID ch_low = group_id * 2;
	PWM_CH_ID ch_high = ch_low + 1;

	PWM_DeInit(ch_low);
	PWM_DeInit(ch_high);

	return HAL_OK;
}

HAL_Status HAL_PWM_EnableCh(PWM_CH_ID ch_id, PWM_Mode mode, uint8_t en)
{
	if (ch_id >= PWM_CH_NUM)
		return HAL_ERROR;

	PWM_EnableClock(ch_id, en);

	if (en) {
		if (mode == PWM_CAPTURE_MODE) {
			HAL_CLR_BIT(PWM->PER, HAL_BIT(ch_id));
			HAL_SET_BIT(PWM->CER, HAL_BIT(ch_id));
		} else {
			HAL_CLR_BIT(PWM->CER, HAL_BIT(ch_id));
			HAL_SET_BIT(PWM->PER, HAL_BIT(ch_id));
		}
	} else {
		HAL_CLR_BIT(PWM->CER, HAL_BIT(ch_id));
		HAL_CLR_BIT(PWM->PER, HAL_BIT(ch_id));
	}

	return HAL_OK;

}

HAL_Status HAL_PWM_OutputPluse(PWM_CH_ID ch_id)
{
	__IO uint32_t *reg = NULL;
	reg = &PWM->CH_REG[ch_id].PCR;

	if ((*reg & PWM_PCR_PLUSE_START) > 0)
		return HAL_BUSY;

	HAL_SET_BIT(*reg, PWM_PCR_PLUSE_START);

	return HAL_OK;
}

HAL_Status HAL_PWM_EnableComplementary(PWM_GROUP_ID group_id, uint8_t en)
{
	if (group_id >= PWM_GROUP_NUM)
		return HAL_ERROR;

	PWM_CH_ID ch_id_0 = PWM_GroupToCh0(group_id);
	PWM_CH_ID ch_id_1 = PWM_GroupToCh1(group_id);
	PWM_EnableClock(ch_id_0, en);
	PWM_EnableClock(ch_id_1, en);

	if (en) {
		HAL_CLR_BIT(PWM->CER, HAL_BIT(ch_id_0) | HAL_BIT(ch_id_1));
		HAL_SET_BIT(PWM->PER, HAL_BIT(ch_id_0) | HAL_BIT(ch_id_1));
	} else
		HAL_CLR_BIT(PWM->PER, HAL_BIT(ch_id_0) | HAL_BIT(ch_id_1));

	return HAL_OK;
}

HAL_Status HAL_PWM_EnableIRQ(PWM_CH_ID ch_id, const PWM_IrqParam *param)
{

	if (PWM->CIER == 0 &&  PWM->PIER == 0)
		PWM_EnableModuleIRQ();

	switch (param->event) {
	case PWM_IRQ_OUTPUT:
		PWM_EnableOutputIRQ(ch_id, 1);
		break;
	case PWM_IRQ_RISEEDGE:
		PWM_EnableInuptRiseEdgeIRQ(ch_id, 1);
		break;
	case PWM_IRQ_FALLEDGE:
		PWM_EnableInuputFallEdgeIRQ(ch_id, 1);
		break;
	case PWM_IRQ_BOTHEDGE:
		PWM_EnableInputBothEdgeIRQ(ch_id, 1);
		break;
	case PWM_IRQ_NULL:
		break;
	default:
		HAL_WRN("PWM invalid IRQ event, event: %d\n", param->event);
		return HAL_ERROR;
	}

	PWM_IrqPrivate[ch_id] = *param;

	return HAL_OK;
}

void HAL_PWM_DisableIRQ(PWM_CH_ID ch_id)
{
	PWM_EnableOutputIRQ(ch_id, 0);
	PWM_EnableInputBothEdgeIRQ(ch_id, 0);

	if (PWM->CIER == 0 &&  PWM->PIER == 0)
		PWM_DisableModuleIRQ();
}

HAL_Status HAL_PWM_ChSetDutyRatio(PWM_CH_ID ch_id, uint16_t value)
{

	if (ch_id >= PWM_CH_NUM) {
		HAL_ERR("Invalid ch_id\n");
		return HAL_ERROR;
	}

	if (value > PWM_CycleValue(ch_id)) {
		HAL_ERR("value is out of range, should <= %u\n", PWM_CycleValue(ch_id));
		return HAL_ERROR;
	}

	while (PWM_OutPeriodRady(ch_id))
		HAL_MSleep(1);

	PWM_SetActCycle(ch_id, value);

	while (PWM_OutPeriodRady(ch_id))
		HAL_MSleep(1);

	return HAL_OK;
}

HAL_Status HAL_PWM_ComplementarySetDutyRatio(PWM_GROUP_ID group_id, uint16_t value)
{
	PWM_CH_ID ch_0, ch_1;

	ch_0 = PWM_GroupToCh0(group_id);
	ch_1 = PWM_GroupToCh1(group_id);

	if (HAL_PWM_ChSetDutyRatio(ch_0, value) != HAL_OK)
		return HAL_ERROR;
	if (HAL_PWM_ChSetDutyRatio(ch_1, value) != HAL_OK)
		return HAL_ERROR;

	return HAL_OK;
}

HAL_Status HAL_PWM_SetDeadZoneTime(PWM_GROUP_ID group_id, uint8_t dead_zone_value)
{
	__IO uint32_t *reg = NULL;

	if (group_id >= PWM_GROUP_NUM) {
		HAL_ERR("Invalid group id\n");
		return HAL_ERROR;
	}

	reg = &PWM->PDZCR[group_id];
	uint32_t p = *reg;

	p &= ~PWM_CH_DZ_INV;
	p |= (dead_zone_value << 8);
	*reg = p;

	return HAL_OK;
}

HAL_Status HAL_PWM_EnableDeadZone(PWM_GROUP_ID group_id, uint8_t en)
{
	if (group_id >= PWM_GROUP_NUM) {
		HAL_ERR("Invalid group id\n");
		return HAL_ERROR;
	}
	if (en)
		PWM_EnableDeadZone(group_id, 1);
	else
		PWM_EnableDeadZone(group_id, 0);

	return HAL_OK;
}

PWM_CapResult HAL_PWM_CaptureResult(PWM_CaptureMode mode, PWM_CH_ID ch_id)
{
	PWM_CapResult result = {0, 0, 0};
	uint16_t fall_time = 0, rise_time = 0;

	switch (mode) {
	case PWM_CAP_PLUSE:
		if (PWM_FallEdgeLock(ch_id) && PWM_RiseEdgeLock(ch_id)) {
			PWM_ClearFallEdgeLock(ch_id);
			PWM_ClearRiseEdgeLock(ch_id);
			fall_time = PWM_CRLRValue(ch_id) + 1;
			rise_time = PWM_CFLRValue(ch_id) + 1;

			result.highLevelTime = rise_time;
			result.lowLevelTime = 0;
			result.periodTime = 0;
		}
		break;
	case PWM_CAP_CYCLE:
		if (PWM_RiseEdgeLock(ch_id)) {
			PWM_ClearFallEdgeLock(ch_id);
			PWM_ClearRiseEdgeLock(ch_id);
			fall_time = PWM_CRLRValue(ch_id) + 1;
			rise_time = PWM_CFLRValue(ch_id) + 1;

			result.highLevelTime = rise_time;
			result.lowLevelTime = fall_time;
			result.periodTime = rise_time + fall_time;
		}
		break;
	default:
		break;
	}

	return result;
}
