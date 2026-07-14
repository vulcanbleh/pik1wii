#ifndef _REVOSDK_OS_OSEXECPARAMS_H
#define _REVOSDK_OS_OSEXECPARAMS_H

#include "types.h"

BEGIN_SCOPE_EXTERN_C

typedef struct {
	BOOL valid;
	u32 restartCode;
	u32 bootDol;
	void* regionStart;
	void* regionEnd;
	BOOL argsUseDefault;
	void* argsAddr;
} OSExecParams;

void __OSGetExecParams(OSExecParams*);

//////////////////////////////////

END_SCOPE_EXTERN_C

#endif
