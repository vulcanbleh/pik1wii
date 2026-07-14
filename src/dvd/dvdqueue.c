#include "RevoSDK/dvd.h"
#include "RevoSDK/os.h"
#include <stddef.h>

static struct {
	DVDCommandBlock* next;
	DVDCommandBlock* prev;
} WaitingQueue[4];

/**
 * @TODO: Documentation
 */
void __DVDClearWaitingQueue()
{
	int i;

	for (i = 0; i < 4; i++) {
		DVDCommandBlock* q = (DVDCommandBlock*)&(WaitingQueue[i]);

		q->next = q;
		q->prev = q;
	}
}

/**
 * @TODO: Documentation
 */
BOOL __DVDPushWaitingQueue(s32 idx, DVDCommandBlock* newTail)
{
	BOOL intrEnabled = OSDisableInterrupts();
	DVDCommandBlock* q;

	q             = (DVDCommandBlock*)&(WaitingQueue[idx]);
	q->prev->next = newTail;
	newTail->prev = q->prev;
	newTail->next = q;
	q->prev       = newTail;

	OSRestoreInterrupts(intrEnabled);
	return TRUE;
}

/**
 * @TODO: Documentation
 */
static DVDCommandBlock* PopWaitingQueuePrio(s32 prio)
{
	DVDCommandBlock *tmp, *q;
	BOOL enabled;

	enabled = OSDisableInterrupts();
	q       = (DVDCommandBlock*)&(WaitingQueue[prio]);

	tmp             = q->next;
	q->next         = tmp->next;
	tmp->next->prev = q;

	OSRestoreInterrupts(enabled);

	tmp->next = NULL;
	tmp->prev = NULL;
	return tmp;
}

/**
 * @TODO: Documentation
 */
DVDCommandBlock* __DVDPopWaitingQueue()
{
	s32 i;
	BOOL intrEnabled = OSDisableInterrupts();
	DVDCommandBlock* q;

	for (i = 0; i < 4; i++) {
		q = (DVDCommandBlock*)&(WaitingQueue[i]);

		if (q->next != q) {
			OSRestoreInterrupts(intrEnabled);
			return PopWaitingQueuePrio(i);
		}
	}

	OSRestoreInterrupts(intrEnabled);
	return NULL;
}

/**
 * @TODO: Documentation
 */
BOOL __DVDCheckWaitingQueue()
{
	u32 i;
	BOOL intrEnabled = OSDisableInterrupts();
	DVDCommandBlock* q;

	for (i = 0; i < 4; i++) {
		q = (DVDCommandBlock*)&(WaitingQueue[i]);

		if (q->next != q) {
			OSRestoreInterrupts(intrEnabled);
			return TRUE;
		}
	}

	OSRestoreInterrupts(intrEnabled);
	return FALSE;
}

/**
 * @TODO: Documentation
 */
DVDCommandBlock* __DVDGetNextWaitingQueue(void)
{
	u32 i;
	BOOL enabled = OSDisableInterrupts();
	DVDCommandBlock *q, *tmp;

	for (i = 0; i < 4; i++) {
		q = (DVDCommandBlock*)&(WaitingQueue[i]);

		if (q->next != q) {
			tmp = q->next;
			OSRestoreInterrupts(enabled);
			return tmp;
		}
	}

	OSRestoreInterrupts(enabled);
	return NULL;
}

/**
 * @TODO: Documentation
 */
BOOL __DVDDequeueWaitingQueue(DVDCommandBlock* block)
{
	BOOL intrEnabled = OSDisableInterrupts();
	DVDCommandBlock *prev, *next;

	prev = block->prev;
	next = block->next;

	if (prev == NULL || next == NULL) {
		OSRestoreInterrupts(intrEnabled);
		return FALSE;
	}

	prev->next = next;
	next->prev = prev;
	OSRestoreInterrupts(intrEnabled);
	return TRUE;
}

/**
 * @TODO: Documentation
 * @note UNUSED Size: 0000CC
 */
void __DVDIsBlockInWaitingQueue(void)
{
	// UNUSED FUNCTION
}

/**
 * @TODO: Documentation
 * @note UNUSED Size: 00010C
 */
void DVDDumpWaitingQueue(void)
{
	// UNUSED FUNCTION
}
