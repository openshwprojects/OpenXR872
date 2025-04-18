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

#ifndef __PM_I_H
#define __PM_I_H

#include "pm/pm.h"

#define MAX_DEV_NAME 12
struct suspend_stats {
	int     success;
	int     fail;
	int     failed_suspend_noirq;
	int     failed_suspend;
	int     failed_resume;
	int     failed_resume_noirq;
	char    failed_devs[MAX_DEV_NAME];
	int     last_failed_step;
};

typedef enum {
	PM_SHUTDOWN = 0,
	PM_REBOOT = 1,
} pm_operate_t;

#define PM_EARLY_SUSPEND           (1 << 16)
#define PM_SUSPEND_BEGIN           (2 << 16)
#define PM_SUSPEND_DEVICES         (3 << 16)
#define PM_SUSPEND_ENTER           (4 << 16)
#define PM_RESUME_SYSTEM           (6 << 16)
#define PM_RESUME_DEVICES          (7 << 16)
#define PM_RESUME_END              (8 << 16)
#define PM_RESUME_COMPLETE         (9 << 16)
#define PM_RESUME_ERROR            (0x0e << 16)
#define PM_SUSPEND_FAIL_FLAG       (0xFFFF)
#define PM_FIRST_BOOT_FLAG         (0x0000)

#define PM_HIBERNATION             (0x0a << 16)

#define PM_SYNC_MAGIC              (0x7FF2DCCD)

extern uint16_t pm_debug_mask;


/**
 * @brief Data constructs for implementation in assembly.
 * @note systick saved by timer subsys.
 */
struct arm_CMX_core_regs {
	unsigned int msp;
	unsigned int psp;
	unsigned int psr;
	unsigned int primask;
	unsigned int faultmask;
	unsigned int basepri;
	unsigned int control;
	unsigned int reg12[16]; /* used only for debug */
};

/**
 * @brief Callbacks for managing platform dependent system sleep states.
 *
 * @begin: Initialise a transition to given system sleep state.
 *      @begin() is executed right prior to suspending devices. The information
 *      conveyed to the platform code by @begin() should be disregarded by it as
 *    soon as @end() is executed. If @begin() fails (ie. returns nonzero),
 *    @prepare(), @enter() and @finish() will not be called.
 *
 * @prepare: Prepare the platform for entering the system sleep state indicated.
 *      @prepare() is called right after devices have been suspended (ie. the
 *      appropriate .suspend() method has been executed for each device) and
 *      before device drivers' late suspend callbacks are executed. It returns
 *      0 on success or a negative error code otherwise, in which case the
 *      system cannot enter the desired sleep state.
 *
 * @enter: Enter the system sleep state indicated.
 *      It returns 0 on success or a negative error code otherwise, in which
 *      case the system cannot enter the desired sleep state.
 *
 * @wake: Called when the system has just left a sleep state, right after
 *      the CPU have been enabled and before device drivers' early resume
 *      callbacks are executed.
 *      It is always called after @enter().
 *
 * @end: Called after resuming devices, to indicate to the platform that the
 *      system has returned to the working state or the transition to the sleep
 *      state has been aborted.
 *    Platforms implementing @begin() should also provide a @end() which
 *    cleans up transitions aborted before @enter().
 */
struct platform_suspend_ops {
	int (*begin)(enum suspend_state_t state);
	int (*prepare)(enum suspend_state_t state);
	void (*enter)(enum suspend_state_t state);
	void (*wake)(enum suspend_state_t state);
	void (*end)(enum suspend_state_t state);
};

#define to_device(ptr_module, idx)            \
	__containerof(ptr_module, struct soc_device, node[idx])
#define get_device(d) (d->ref++;)
#define put_device(d) (d->ref--;)

extern struct suspend_stats suspend_stats;
extern const char *const pm_states[];

void suspend_ops_init(struct platform_suspend_ops *suspend_ops);

extern void __cpu_sleep(enum suspend_state_t state);
extern void __cpu_suspend(enum suspend_state_t state);

extern void suspend_test_start(void);
extern void suspend_test_finish(const char *label);
extern int suspend_test(int level);
extern void pm_set_debug_delay_ms(unsigned int ms);
extern int check_wakeup_irqs(void);
extern void parse_dpm_list(unsigned int idx);
extern int dpm_suspend_noirq(enum suspend_state_t state);
extern int dpm_suspend(enum suspend_state_t state);
extern int suspend_devices_and_enter(enum suspend_state_t state);
extern void pm_select_mode(enum suspend_state_t state);
extern void dpm_resume_noirq(enum suspend_state_t state);
extern void dpm_resume(enum suspend_state_t state);

#endif /* __PM_I_H */
