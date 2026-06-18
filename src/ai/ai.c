#include "RevoSDK/ai.h"
#include "RevoSDK/hw_regs.h"
#include "RevoSDK/os.h"
#include <stddef.h>

static AIDCallback __AID_Callback;
static u8* __CallbackStack;
static u8* __OldStack;
static vs32 __AI_init_flag = FALSE;
static vs32 __AID_Active = FALSE;

static OSTime bound_32KHz;
static OSTime bound_48KHz;
static OSTime min_wait;
static OSTime max_wait;
static OSTime buffer;

static void __AI_set_stream_sample_rate(u32 rate);
static void __AISHandler(s16 interrupt, OSContext* context);
static void __AIDHandler(s16 interrupt, OSContext* context);
static void __AICallbackStackSwitch(register AIDCallback cb);
static void __AI_SRC_INIT(void);

const char* __AIVersion = "<< RVL_SDK - AI \trelease build: Aug  8 2007 01:58:12 (0x4199_60831) >>";

/**
 * @TODO: Documentation
 */
AIDCallback AIRegisterDMACallback(AIDCallback callback)
{
	s32 oldInts;
	AIDCallback ret;

	ret            = __AID_Callback;
	oldInts        = OSDisableInterrupts();
	__AID_Callback = callback;
	OSRestoreInterrupts(oldInts);
	return ret;
}

/**
 * @TODO: Documentation
 */
void AIInitDMA(u32 address, u32 length)
{
	s32 previousInterruptState;

	previousInterruptState = OSDisableInterrupts();

	__DSPRegs[DSP_DMA_START_HI]    = (u16)((__DSPRegs[DSP_DMA_START_HI] & ~0x1FFF) | (address >> 16));
	__DSPRegs[DSP_DMA_START_LO]    = (u16)((__DSPRegs[DSP_DMA_START_LO] & ~0xFFE0) | (address & 0xFFFF));
	__DSPRegs[DSP_DMA_CONTROL_LEN] = (u16)((__DSPRegs[DSP_DMA_CONTROL_LEN] & ~0x7FFF) | ((length >> 5) & 0xFFFF));

	OSRestoreInterrupts(previousInterruptState);
}

/**
 * @TODO: Documentation
 */
void AIStartDMA(void)
{
	__DSPRegs[DSP_DMA_CONTROL_LEN] |= DSP_DMA_START_FLAG;
}

/**
 * @TODO: Documentation
 */
void AIStopDMA(void) {
    __DSPRegs[DSP_DMA_CONTROL_LEN] &= ~DSP_DMA_START_FLAG;
}

/**
 * @TODO: Documentation
 */
u32 AIGetDMABytesLeft(void) {
    return (__DSPRegs[DSP_DMA_BYTES_LEFT] & 0x7FFF) << 5;
}

/**
 * @TODO: Documentation
 */
u32 AIGetDMAStartAddr(void) {
    return (((__DSPRegs[DSP_DMA_START_HI] & 0x1FFF) << 16) | (__DSPRegs[0x19] & 0xFFE0));
}

/**
 * @TODO: Documentation
 */
u32 AIGetDMALength(void) {
    return ((__DSPRegs[DSP_DMA_CONTROL_LEN] & 0x7FFF) << 5);
}

/**
 * @TODO: Documentation
 */
BOOL AICheckInit(void) {
    return __AI_init_flag;
}

/**
 * @TODO: Documentation
 */
u32 AIGetDSPSampleRate(void)
{
	return ((__AIRegs[AI_CONTROL] >> 6) & 1) ^ 1;
}

/**
 * @TODO: Documentation
 */
void AISetDSPSampleRate(u32 rate)
{
	BOOL previousInterruptState;

	// If the requested rate is the same as the current rate, do nothing
	if (rate == AIGetDSPSampleRate()) {
		return;
	}

	// Clear the DSP sample rate bit in the control register
	__AIRegs[AI_CONTROL] &= ~AI_CONTROL_DSP_SAMPLE_RATE;

	if (rate == 0) {
		// Disable interrupts and initialize the sample rate converter
		previousInterruptState = OSDisableInterrupts();
		__AI_SRC_INIT();
		// Set the DSP sample rate bit in the control register
		__AIRegs[AI_CONTROL] |= AI_CONTROL_DSP_SAMPLE_RATE;

		// Restore the previous interrupt state
		OSRestoreInterrupts(previousInterruptState);
	}
}




/**
 * @TODO: Documentation
 */
void AIInit(u8* stack)
{
	u32 reg;

	// If AI is already initialized, do nothing
	if (__AI_init_flag == TRUE) {
		return;
	}

	OSRegisterVersion(__AIVersion);

	// Set bounds and buffer sizes in ticks
	bound_32KHz = OSNanosecondsToTicks(31524);
	bound_48KHz = OSNanosecondsToTicks(42024);
	min_wait    = OSNanosecondsToTicks(42000);
	max_wait    = OSNanosecondsToTicks(63000);
	buffer      = OSNanosecondsToTicks(3000);

	reg = __AIRegs[AI_CONTROL];
	reg &= ~(0x1 | 0x4 | 0x10);
	__AIRegs[AI_CONTROL]       = reg;
	__AIRegs[AI_VOLUME]        = 0;
	__AIRegs[AI_INTRPT_TIMING] = 0;
	__AIRegs[AI_CONTROL]       = (__AIRegs[AI_CONTROL] & ~0x20) | (0x1 << 5);

	AISetDSPSampleRate(0);

	// Clear callback and set callback stack
	__AID_Callback  = 0;
	__CallbackStack = stack;

	// Set interrupt handlers and unmask interrupts
	__OSSetInterruptHandler(5, __AIDHandler);
	__OSUnmaskInterrupts(OS_INTERRUPTMASK_DSP_AI);

	// Set AI initialisation flag to TRUE
	__AI_init_flag = TRUE;
}

/**
 * @TODO: Documentation
 */
static void __AIDHandler(s16 interrupt, OSContext* context)
{
	OSContext tempContext;
	u16 reg;

	reg          = __DSPRegs[5];
	reg          = (reg & ~(0x80 | 0x20) | 0x8);
	__DSPRegs[5] = reg;

	OSClearContext(&tempContext);
	OSSetCurrentContext(&tempContext);

	if (__AID_Callback) {
		if (!__AID_Active) {
			__AID_Active = TRUE;

			if (__CallbackStack) {
				__AICallbackStackSwitch(__AID_Callback);
			} else {
				__AID_Callback();
			}

			__AID_Active = FALSE;
		}
	}

	OSClearContext(&tempContext);
	OSSetCurrentContext(context);
}

/**
 * @TODO: Documentation
 */
ASM static void __AICallbackStackSwitch(register AIDCallback cb)
{
#ifdef __MWERKS__ // clang-format off
	// Allocate stack frame
	fralloc

	// Store current stack
	lis r5, __OldStack@ha
	addi r5, r5, __OldStack@l
	stw r1, 0(r5)

	// Load stack for callback
	lis r5, __CallbackStack@ha
	addi r5, r5, __CallbackStack@l
	lwz r1,0(r5)

	// Move stack down 8 bytes
	subi r1, r1, 8
	// Call callback
	mtlr cb
	blrl

	// Restore old stack
	lis r5, __OldStack @ha
	addi r5, r5, __OldStack@l
	lwz r1,0(r5)

	// Free stack frame
	frfree

	blr
#endif // clang-format on
}

/**
 * @TODO: Documentation
 */
static void __AI_SRC_INIT(void)
{
	OSTime rising_32khz = 0;
	OSTime rising_48khz = 0;
	OSTime diff         = 0;
	OSTime t1           = 0;
	OSTime temp         = 0;
	u32 temp0           = 0;
	u32 temp1           = 0;
	u32 done            = 0;
	u32 volume          = 0;
	u32 Init_Cnt        = 0;
	u32 walking         = 0;

	walking  = 0;
	Init_Cnt = 0;
	temp     = 0;

	while (!done) {
		__AIRegs[AI_CONTROL] = (__AIRegs[AI_CONTROL] & ~0x20) | 0x20;
		__AIRegs[AI_CONTROL] &= ~2;
		__AIRegs[AI_CONTROL] = (__AIRegs[AI_CONTROL] & ~1) | 1;

		temp0 = (__AIRegs[AI_SAMPLE_COUNTER] & 0x7FFFFFFF) >> 0;

		while (temp0 == (__AIRegs[AI_SAMPLE_COUNTER] & 0x7FFFFFFF) >> 0)
			;
		rising_32khz = OSGetTime();

		__AIRegs[AI_CONTROL] = (__AIRegs[AI_CONTROL] & ~2) | 2;
		__AIRegs[AI_CONTROL] = (__AIRegs[AI_CONTROL] & ~1) | 1;

		temp1 = (__AIRegs[AI_SAMPLE_COUNTER] & 0x7FFFFFFF) >> 0;
		while (temp1 == (__AIRegs[AI_SAMPLE_COUNTER] & 0x7FFFFFFF) >> 0)
			;

		rising_48khz = OSGetTime();

		diff = rising_48khz - rising_32khz;
		__AIRegs[AI_CONTROL] &= ~2;
		__AIRegs[AI_CONTROL] &= ~1;

		if (diff < (bound_32KHz - buffer)) {
			temp = min_wait;
			done = 1;
			++Init_Cnt;
		} else if (diff >= (bound_32KHz + buffer) && diff < (bound_48KHz - buffer)) {
			temp = max_wait;
			done = 1;
			++Init_Cnt;
		} else {
			done    = 0;
			walking = 1;
			++Init_Cnt;
		}
	}

	while ((rising_48khz + temp) > OSGetTime())
		;
}
