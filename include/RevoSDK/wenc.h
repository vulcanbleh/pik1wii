#ifndef REVOSDK_WENC_H
#define REVOSDK_WENC_H

#include <types.h>

BEGIN_SCOPE_EXTERN_C

typedef enum {
	WENC_FLAG_USER_INFO = (1 << 0),
} WENCFlag;

typedef struct WENCInfo {
	s32 xn;        // _00
	s32 dl;        // _04
	s32 qn;        // _08
	s32 dn;        // _0C
	s32 dlh;       // _10
	s32 dlq;       // _14
	u8 padding[8]; // _18
} WENCInfo;

s32 WENCGetEncodeData(WENCInfo*, u32, const s16*, s32, u8*);

END_SCOPE_EXTERN_C

#endif
