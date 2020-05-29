#ifndef _VWII_CLEANER_H
#define _VWII_CLEANER_H

#include "wut_types.h"

#include "main.h"

#define SLCCMPT_TITLE_PATH "dev:/title/00000001"
#define SLCCMPT_TICKET_PATH "dev:/ticket/00000001"

#define TITLE_PREFIX "dev:/title/"
#define TICKET_PREFIX "dev:/ticket/"
#define TICKET_EXTENSION ".tik"

#define UPDATE_FOLDER_PATH "mlc:/sys/update"

// System menus data folder contains important data that does not get restored
static const char* const cleanBlackList[] =
{
    "dev:/title/00000001/00000002/data/setting.txt"
};

BOOL softClean();
BOOL agressiveClean();
BOOL customClean(BOOL* advancedEnabled, int size, Region region);
void forceUpdate();

#endif