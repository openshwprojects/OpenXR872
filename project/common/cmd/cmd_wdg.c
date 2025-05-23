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

#include "cmd_util.h"
#include "cmd_wdg.h"
#include "driver/chip/hal_wdg.h"

#if HAL_WDG_INTERRUPT_SUPPORT
static void cmd_wdg_callback(void *arg)
{
	cmd_write_event(CMD_EVENT_WDG_TIMEOUT, "wdg timeout @ %d ms", OS_GetTicks());
}
#endif

/*
 * drv wdg config m=<mode> t=<timeout>
 */
static enum cmd_status cmd_wdg_config_exec(char *cmd)
{
	char mode_str[9];
	uint32_t timeout;
	WDG_InitParam param;
	int32_t cnt;

	cnt = cmd_sscanf(cmd, "m=%8s t=%u", mode_str, &timeout);
	if (cnt != 2) {
		return CMD_STATUS_INVALID_ARG;
	}

	cmd_memset(&param, 0, sizeof(param));

	if (cmd_strcmp(mode_str, "reboot") == 0) {
		param.hw.event = WDG_EVT_RESET;
		param.hw.resetCycle = WDG_DEFAULT_RESET_CYCLE;
#if HAL_WDG_INTERRUPT_SUPPORT
	} else if (cmd_strcmp(mode_str, "it") == 0) {
		param.hw.event = WDG_EVT_INTERRUPT;
		param.callback = cmd_wdg_callback;
		param.arg = NULL;
#endif
#if (__CONFIG_CHIP_ARCH_VER == 2)
	} else if (cmd_strcmp(mode_str, "resetcpu") == 0) {
		param.hw.event = WDG_EVT_RESET_CPU;
		param.hw.resetCpuMode = PRJCONF_WDG_RESET_CPU_MODE;
		param.hw.resetCycle = WDG_DEFAULT_RESET_CYCLE;
#endif
	} else {
		CMD_ERR("invalid mode %s\n", mode_str);
		return CMD_STATUS_INVALID_ARG;
	}

	if (timeout > 11) {
		CMD_ERR("invalid timeout %u\n", timeout);
		return CMD_STATUS_INVALID_ARG;
	} else {
		param.hw.timeout = (timeout << WDG_TIMEOUT_SHIFT) & WDG_TIMEOUT_MASK;
	}

	if (HAL_WDG_Init(&param) != HAL_OK) {
		return CMD_STATUS_FAIL;
	}

	return CMD_STATUS_OK;
}

/*
 * drv wdg deconfig
 */
static enum cmd_status cmd_wdg_deconfig_exec(char *cmd)
{
	HAL_WDG_DeInit();
	return CMD_STATUS_OK;
}

/*
 * drv wdg start
 */
static enum cmd_status cmd_wdg_start_exec(char *cmd)
{
	HAL_WDG_Start();
	return CMD_STATUS_OK;
}

/*
 * drv wdg stop
 */
static enum cmd_status cmd_wdg_stop_exec(char *cmd)
{
	HAL_WDG_Stop();
	return CMD_STATUS_OK;
}

/*
 * drv wdg feed
 */
static enum cmd_status cmd_wdg_feed_exec(char *cmd)
{
	HAL_WDG_Feed();
	return CMD_STATUS_OK;
}

/*
 * drv wdg reboot
 */
static enum cmd_status cmd_wdg_reboot_exec(char *cmd)
{
	HAL_WDG_Reboot();
	return CMD_STATUS_OK;
}

#if (__CONFIG_CHIP_ARCH_VER == 2)
static enum cmd_status cmd_wdg_reset_cpu_exec(char *cmd)
{
	HAL_WDG_ResetCpu(WDG_RESET_CPU_CORE);
	return CMD_STATUS_OK;
}

/*
 * drv wdg noreset module
 */
const static char *module_name[] = {"", "spi1", "", "flash", "sdc0", "csi", "", "", "", \
                                    "sdc1", "wlan", "psram", "", "", "", "", "", "uart1", "twi0", \
                                    "twi1", "", "pwm", "daudio", "irtx", "irrx", "gpadc", "", "", \
                                    "uart2", "codec", ""};

static enum cmd_status cmd_wdg_noreset_exec(char *cmd)
{
	int i;
	char module_str[8];

	cmd_sscanf(cmd, "%s", module_str);
	for (i = 0; i < cmd_nitems(module_name); i++) {
		if (cmd_strcmp(module_str, module_name[i]) == 0) {
			HAL_WDG_SetNoResetPeriph(1 << i, 1);
			break;
		}
	}
	if (i >= cmd_nitems(module_name)) {
		printf("invalid module name: %s", module_str);
		return CMD_STATUS_INVALID_ARG;
	}
	return CMD_STATUS_OK;
}
#endif

static const struct cmd_data g_wdg_cmds[] = {
	{ "config",     cmd_wdg_config_exec },
	{ "deconfig",   cmd_wdg_deconfig_exec },
	{ "start",      cmd_wdg_start_exec },
	{ "stop",       cmd_wdg_stop_exec },
	{ "feed",       cmd_wdg_feed_exec },
	{ "reboot",     cmd_wdg_reboot_exec },
#if (__CONFIG_CHIP_ARCH_VER == 2)
	{ "reset_cpu",  cmd_wdg_reset_cpu_exec },
	{ "noreset",    cmd_wdg_noreset_exec },
#endif
};

enum cmd_status cmd_wdg_exec(char *cmd)
{
	return cmd_exec(cmd, g_wdg_cmds, cmd_nitems(g_wdg_cmds));
}
