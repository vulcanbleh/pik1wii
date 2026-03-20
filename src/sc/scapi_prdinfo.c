#include <RevoSDK/sc.h>
#include <RevoSDK/os.h>
#include <string.h>

typedef struct {
    s8 area;
    char string[4];
} SCProductAreaAndString;

static SCProductAreaAndString ProductAreaAndStringTbl[] = {
    SC_AREA_JPN, "JPN",
    SC_AREA_USA, "USA",
    SC_AREA_EUR, "EUR",
    SC_AREA_AUS, "AUS",
    SC_AREA_BRA, "BRA",
    SC_AREA_TWN, "TWN",
    SC_AREA_TWN, "ROC",
    SC_AREA_KOR, "KOR",
    SC_AREA_HKG, "HKG",
    SC_AREA_ASI, "ASI",
    SC_AREA_LTN, "LTN",
    SC_AREA_SAF, "SAF",
    -1
};

static BOOL SCGetProductAreaString(char* buf, u32 bufSize) {
    return __SCF1("AREA", buf, bufSize);
}


BOOL __SCF1(const char* keyword, char* buf, u32 bufSize) {
    BOOL result = FALSE;
    u8* p = (u8*)OSPhysicalToCached(0x3800);
    u32 size = 0x100, i, seed = 0x73b5dbfa, kwOffset = 0;
    u8 data;
    u32 bufOffset = 0;

    for (i = 0; i < size; i++) {
        data = p[i];

        if (data != 0) {
            data ^= seed;

            if (keyword[kwOffset] == '\0') {
                if (data == '=') {
                    result = TRUE;
                    break;
                }
            }

            if (((keyword[kwOffset] ^ data) & 0xDF) == 0) {
                kwOffset++;
            }
            else {
                kwOffset = 0;
            }
        }

        seed = (u32)((seed >> 31) | (seed << 1));
    }

    if (result) {
        i++;

        while (i < size && bufOffset < bufSize) {
            seed = (u32)((seed >> 31) | (seed << 1));
            data = p[i];

            if (data != 0) {
                data ^= seed;

                if (data == '\x0d' || data == '\x0a') {
                    data = 0;
                }
            }

            buf[bufOffset] = (char)data;
            bufOffset++;
            if (data == 0) {
                return TRUE;
            }

            i++;
        }
    }

    return FALSE;
}

s8 SCGetProductArea(void) {
    char buf[4];
    SCProductAreaAndString* p = ProductAreaAndStringTbl;

    if (SCGetProductAreaString(buf, sizeof(buf))) {
        while (p->area != -1) {
            if (strcmp(p->string, buf) == 0) {
                return p->area;
            }

            p++;
        }
    }

    return -1;
}

typedef struct {
    s8 game;
    char string[3];
} SCProductGameRegionAndString;

static SCProductGameRegionAndString ProductGameRegionAndStringTbl[] = {
    0, "JP",
    1, "US",
    2, "EU",
    -1
};

s8 SCGetProductGameRegion(void) {
    char buf[3];
    SCProductGameRegionAndString* p = ProductGameRegionAndStringTbl;

    if (__SCF1("GAME", buf, sizeof(buf))) {
        while (p->game != -1) {
            if (strcmp(p->string, buf) == 0) {
                return p->game;
            }

            p++;
        }
    }

    return -1;
}