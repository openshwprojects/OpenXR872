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
#include <stdio.h>
#include <stdarg.h>
#include "kernel/os/os.h"
#include "shared/src/new_cfg.h"
#include "shared/src/new_pins.h"
#include "shared/src/mqtt/new_mqtt.h"
#include "shared/src/new_common.h"



OSStatus rtos_create_thread( beken_thread_t* thread, 
							uint8_t priority, const char* name, 
							beken_thread_function_t function,
							uint32_t stack_size, beken_thread_arg_t arg ) {
    OSStatus err = kNoErr;
	int new_priority;
	// TODO: translate
	new_priority = OS_THREAD_PRIO_CONSOLE;

		printf("rtos_create_thread %s stack %i\n", name, stack_size);
	stack_size = (2 * 1024);
	err = OS_ThreadCreate(thread,
		                name,
		                function,
		                arg,
		                new_priority,
		                stack_size);
		printf("rtos_create_thread %s  gives %i\n", name,  err);

	return err;
}

OSStatus rtos_delete_thread( beken_thread_t* thread ) {
    return OS_ThreadDelete( thread );
}
#define MAX_DUMP_BUFF_SIZE 256
char dump_buffer[MAX_DUMP_BUFF_SIZE];
void bk_printf(char *format, ...){
    va_list vp;

    va_start(vp, format);
    vsnprintf(dump_buffer, MAX_DUMP_BUFF_SIZE, format, vp);
    va_end(vp);

	printf("%s\r\n",dump_buffer);
}
int main(void)
{
	platform_init();
	int x = 0;
	while (x++<10) {
		OS_Sleep(1);
		printf("Hello wordsald! @ %u sec\n", OS_GetTicks());
	}
	Main_Init();
	user_main();
	return 0;
}
