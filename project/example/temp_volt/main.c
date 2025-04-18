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


#include "common/framework/platform_init.h"
#include "net/wlan/wlan.h"
#include "common/framework/net_ctrl.h"

void temp_volt_event_cb(wlan_ext_temp_volt_event_data_t *temp_volt_data)
{
	printf("************** temp_volt_data **************\n");
	printf("ind_flags:0x%04X\n", temp_volt_data->ind_flags);
	printf("tmp_now:%.02f°C\n", (float)temp_volt_data->tmp_now / 16);
	printf("tmp_max:%.02f°C\n", (float)temp_volt_data->tmp_max / 16);
	printf("tmp_min:%.02f°C\n", (float)temp_volt_data->tmp_min / 16);
	printf("volt_now:%.02fV\n", (float)temp_volt_data->volt_now / 16);
	printf("volt_max:%.02fV\n", (float)temp_volt_data->volt_max / 16);
	printf("volt_min:%.02fV\n", (float)temp_volt_data->volt_min / 16);
	if (temp_volt_data->ind_flags & WLAN_EXT_TEMP_THRESH_HIGH_OVERFLOW)
		printf("Temperature thresh high overflow!\n");
	if (temp_volt_data->ind_flags & WLAN_EXT_TEMP_THRESH_LOW_OVERFLOW)
		printf("Temperature thresh low overflow!\n");
	if (temp_volt_data->ind_flags & WLAN_EXT_VOLT_THRESH_HIGH_OVERFLOW)
		printf("Voltage thresh high overflow!\n");
	if (temp_volt_data->ind_flags & WLAN_EXT_VOLT_THRESH_LOW_OVERFLOW)
		printf("Voltage thresh low overflow!\n");
	if (temp_volt_data->ind_flags & WLAN_EXT_TEMP_VOLT_FALLBACK_TO_THRESH)
		printf("Temperature or voltage back to thresh!\n");
	printf("********************************************\n");
}

int main(void)
{
	platform_init();
	wlan_ext_set_temp_volt_event_cb(temp_volt_event_cb);

	return 0;
}
