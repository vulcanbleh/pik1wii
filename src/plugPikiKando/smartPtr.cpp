#include "DebugLog.h"
#include "RefCountable.h"

/**
 * @todo: Documentation
 * @note UNUSED Size: 000098
 */
DEFINE_ERROR(__LINE__) // Never used in the DLL

/**
 * @todo: Documentation
 * @note UNUSED Size: 0000F0
 */
DEFINE_PRINT("smartPtr");

/**
 * @todo: Documentation
 */
RefCountable::RefCountable()
{
	mCount = 0;
}

/**
 * @todo: Documentation
 */
void RefCountable::clearCnt()
{
	mCount = 0;
}

/**
 * @todo: Documentation
 */
void RefCountable::addCnt()
{
	mCount++;
}

/**
 * @todo: Documentation
 */
void RefCountable::subCnt()
{
	mCount--;
	if (mCount >= 0) {
		return;
	}
	mCount = 0;
	return;
}
