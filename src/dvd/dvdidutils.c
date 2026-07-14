#include "RevoSDK/dvd.h"
#include "string.h"

BOOL DVDCompareDiskID(DVDDiskID* discID1, DVDDiskID* discID2)
{
	if ((discID1->gameName[0] != 0) && (discID2->gameName[0] != 0) && (strncmp(discID1->gameName, discID2->gameName, 4) != 0)) {
		return FALSE;
	}
	if ((discID1->company[0] == 0) || (discID2->company[0] == 0) || (strncmp(discID1->company, discID2->company, 2) != 0)) {
		return FALSE;
	}

	if (discID1->diskNumber != 0xFF && discID2->diskNumber != 0xFF && discID1->diskNumber != discID2->diskNumber) {
		return FALSE;
	}

	if (discID1->gameVersion != 0xFF && discID2->gameVersion != 0xFF && discID1->gameVersion != discID2->gameVersion) {
		return FALSE;
	}

	return TRUE;
}

/**
 * @note Address: N/A
 * @note Size: 0x80
 */
void DVDGenerateDiskID(void)
{
	// UNUSED FUNCTION
}
