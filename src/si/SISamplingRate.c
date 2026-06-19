#include "RevoSDK/OS.h"
#include "RevoSDK/vi.h"
#include "types.h"

extern u32 SISetXY(int x, int y);

struct tvformat {
	u16 mTvShort;
	u8 mTvByte;
};

u16 _viRegList : 0xCC00206C;

static u32 SamplingRate;
static struct tvformat XYNTSC[12] = {
	246, 2, 14, 19, 30, 9, 44, 6, 52, 5, 65, 4, 87, 3, 87, 3, 87, 3, 131, 2, 131, 2, 131, 2,
};

static struct tvformat XYPAL[12] = {
	296, 2, 15, 21, 29, 11, 45, 7, 52, 6, 63, 5, 78, 4, 104, 3, 104, 3, 104, 3, 104, 3, 156, 2,
};

/**
 * @brief Sets the controller port sampling rate in milliseconds.
 *
 * All the attached controller devices share the same sampling rate setting.
 * Since the sampling rate is controlled by the Flipper's video clock, the actual sampling rate set differs from the one specified.
 *
 * @param msec The desired sampling rate in milliseconds. If the value is greater than 11, it will be set to 11.
 *
 */
void SISetSamplingRate(u32 msec)
{
	BOOL interruptState;
	u16 viRegList;
	int xScale;
	struct tvformat* format; // r4

	if (msec > 11) {
		msec = 11;
	}

	interruptState = OSDisableInterrupts();
	SamplingRate   = msec;

	switch (VIGetTvFormat()) {
	case VI_NTSC:
	case VI_MPAL:
	case VI_EURGB60:
		format = XYNTSC;
		break;
	case VI_PAL:
		format = XYPAL;
		break;
	default:
		OSReport("SISetSamplingRate: unknown TV format. Use default.");
		msec   = 0;
		format = XYNTSC;
		break;
	}

	viRegList = (_viRegList);
	if ((viRegList & 1) != 0) {
		xScale = 2;
	} else {
		xScale = 1;
	}

	SISetXY(xScale * format[msec].mTvShort, format[msec].mTvByte);
	OSRestoreInterrupts(interruptState);
}

/**
 * @TODO: Documentation
 */
void SIRefreshSamplingRate(void)
{
	SISetSamplingRate(SamplingRate);
}
