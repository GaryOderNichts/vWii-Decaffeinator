#ifndef _MAIN_H_
#define _MAIN_H_

#define HBL_TITLE_ID (0x0005000013374842)
#define MII_MAKER_JPN_TITLE_ID (0x000500101004A000)
#define MII_MAKER_USA_TITLE_ID (0x000500101004A100)
#define MII_MAKER_EUR_TITLE_ID (0x000500101004A200)

#define SLCCMPT_DEV_PATH "/dev/slccmpt01"
#define SLCCMPT_MOUNT_PATH "/vol/storage_slccmpt01"

#define SLC_VOL_PATH "/vol/system"

#define MLC_VOL_PATH "/vol/storage_mlc01"

#define SYSTEM_SETTINGS_TITLE_ID 0x0005001010047200

#include "wut_types.h"

typedef enum 
{
    REGION_JAPAN = 0,
    REGION_USA = 1,
    REGION_EUROPE = 2,
    REGION_UNSUPPORTED = 3
} Region;

static const char* const regionStrings[] = { "Japan", "USA", "Europe", "Unsupported" };

void console_print_pos(int x, int y, const char* format, ...);

int MenuMain(void);

void setCanExit(BOOL canExit);

#endif
