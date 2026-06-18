#include "RevoSDK/dsp.h"

#include "RevoSDK/hw_regs.h"
#include "RevoSDK/os.h"

static BOOL __DSP_init_flag = 0;
extern DSPTaskInfo* __DSP_curr_task;
extern DSPTaskInfo* __DSP_first_task;
extern DSPTaskInfo* __DSP_last_task;
extern DSPTaskInfo* __DSP_tmp_task;

const char* __DSPVersion = "<< RVL_SDK - DSP \trelease build: Aug  8 2007 01:59:06 (0x4199_60831) >>";

/**
 * @TODO: Documentation
 */
u32 DSPCheckMailToDSP()
{
	return __DSPRegs[DSP_MAILBOX_IN_HI] >> 0xF & 1;
}

/**
 * @TODO: Documentation
 */
u32 DSPCheckMailFromDSP(void)
{
	return __DSPRegs[DSP_MAILBOX_OUT_HI] >> 0xF & 1;
}

/**
 * @TODO: Documentation
 * @note UNUSED Size: 000014
 */
void DSPReadCPUToDSPMbox(void)
{
	// UNUSED FUNCTION
}

/**
 * @TODO: Documentation
 */
u32 DSPReadMailFromDSP()
{
	return (__DSPRegs[DSP_MAILBOX_OUT_HI] << 0x10) | __DSPRegs[DSP_MAILBOX_OUT_LO];
}

/**
 * @TODO: Documentation
 */
void DSPSendMailToDSP(u32 mail)
{
	__DSPRegs[DSP_MAILBOX_IN_HI] = mail >> 0x10;
	__DSPRegs[DSP_MAILBOX_IN_LO] = mail;
}

/**
 * @TODO: Documentation
 */
void DSPAssertInt(void)
{
	BOOL interrupts = OSDisableInterrupts();
	u16 tmp;

	tmp                           = __DSPRegs[DSP_CONTROL_STATUS];
	tmp                           = (tmp & ~0xA8 | 0x2);
	__DSPRegs[DSP_CONTROL_STATUS] = tmp;
	OSRestoreInterrupts(interrupts);
}

/**
 * @TODO: Documentation
 */
void DSPInit(void)
{
	BOOL intr;
	u16 reg;

	__DSP_debug_printf("DSPInit(): Build Date: %s %s\n", "Aug  8 2007", "01:59:06");

	if (__DSP_init_flag == 1) {
		return;
	}

	OSRegisterVersion(__DSPVersion);
	intr = OSDisableInterrupts();
	__OSSetInterruptHandler(7, __DSPHandler);
	__OSUnmaskInterrupts(0x1000000);

	reg                           = __DSPRegs[DSP_CONTROL_STATUS];
	reg                           = ((reg & ~(0x20 | 0x8 | 0x80)) | 0x800);
	__DSPRegs[DSP_CONTROL_STATUS] = reg;

	reg                           = __DSPRegs[DSP_CONTROL_STATUS];
	reg                           = (reg & ~(0x20 | 0x8 | 0x80 | 0x4));
	__DSPRegs[DSP_CONTROL_STATUS] = reg;

	__DSP_first_task = __DSP_last_task = __DSP_curr_task = __DSP_tmp_task = NULL;
	__DSP_init_flag                                                       = 1;
	OSRestoreInterrupts(intr);
}

/**
 * @TODO: Documentation
 */
BOOL DSPCheckInit(void)
{
	return __DSP_init_flag;
}

/**
 * @TODO: Documentation
 */
DSPTaskInfo* DSPAssertTask(DSPTaskInfo* task)
{
	BOOL enabled;

	enabled = OSDisableInterrupts();

	if (__DSP_curr_task == task) {
		__DSP_rude_task_pending = TRUE;
		__DSP_rude_task         = task;
		OSRestoreInterrupts(enabled);
		return task;
	}

	if (task->priority < __DSP_curr_task->priority) {
		__DSP_rude_task_pending = TRUE;
		__DSP_rude_task         = task;

		if (__DSP_curr_task->state == DSP_TASK_STATE_RUN) {
			DSPAssertInt();
		}

		OSRestoreInterrupts(enabled);
		return task;
	}

	OSRestoreInterrupts(enabled);
	return NULL;
}
