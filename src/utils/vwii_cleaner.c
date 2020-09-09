#include "vwii_cleaner.h"

#include <whb/log.h>
#include <whb/log_console.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <iosuhax.h>

#include "fsutils.h"
#include "digest_utils.h"
#include "main.h"
#include "menu/title_strings.h"

BOOL softClean()
{
    setCanExit(FALSE);

    BOOL titleRes = clearFolderWithBlackList(SLCCMPT_TITLE_PATH, cleanBlackList, sizeof(cleanBlackList) / 4, 0, NULL);
    BOOL ticketRes = clearFolderWithBlackList(SLCCMPT_TICKET_PATH, cleanBlackList, sizeof(cleanBlackList) / 4, 0, NULL);

    // If we deleted anything make sure to trigger an update
    if (titleRes || ticketRes)
    {
        return TRUE;
    }

    setCanExit(TRUE);
    return FALSE;
}

BOOL agressiveClean()
{
    setCanExit(FALSE);

    BOOL res = clearFolderWithBlackList("dev:", cleanBlackList, sizeof(cleanBlackList) / 4, 0, NULL);

    // If we deleted anything make sure to trigger an update
    if (res)
    {
        return TRUE;
    }

    setCanExit(TRUE);
    return FALSE;
}

BOOL customClean(BOOL* advancedEnabled, int size, Region region)
{
    setCanExit(FALSE);

    BOOL modified = FALSE;
    for (int i = 0; i < size; i++)
    {
        if (advancedEnabled[i])
        {
            char* full_title_path = malloc(1024);
            full_title_path[0] = '\0';
            char* full_ticket_path = malloc(1024);
            full_ticket_path[0] = '\0';

            if (advancedOptionsIds[i] == 0)
            {
                if (region == REGION_UNSUPPORTED)
                    continue;

                char* id = malloc(8 + 1);

                switch (i)
                {
                case 0:
                    strcpy(id, wiiMenuManualId[region]);
                    break;
                case 1:
                    strcpy(id, vWiiSystemChannelId[region]);
                    break;
                case 2:
                    strcpy(id, regionSelectId[region]);
                    break;
                }
                
                snprintf(full_title_path, 1024, TITLE_PREFIX "%s/%s", advancedOptionsFolders[i], id);
                snprintf(full_ticket_path, 1024, TICKET_PREFIX "%s/%s" TICKET_EXTENSION, advancedOptionsFolders[i], id);
                free(id);
            }
            else
            {
                if (i != 3) // only delete content when cleaning sysmenu
                {
                    snprintf(full_title_path, 1024, TITLE_PREFIX "%s/%s", advancedOptionsFolders[i], advancedOptionsIds[i]);
                }
                else
                {
                    snprintf(full_title_path, 1024, TITLE_PREFIX "%s/%s/content", advancedOptionsFolders[i], advancedOptionsIds[i]);
                }

                snprintf(full_ticket_path, 1024, TICKET_PREFIX "%s/%s" TICKET_EXTENSION, advancedOptionsFolders[i], advancedOptionsIds[i]);         
            }

            if (deleteFolder(full_title_path))
                modified = TRUE;

            if (unlink(full_ticket_path) == 0)
            {
                WHBLogPrintf("removed %s", full_ticket_path);
                WHBLogConsoleDraw();
                modified = TRUE;
            }
            else
            {
                WHBLogPrintf("cannot remove %s", full_ticket_path);
                WHBLogConsoleDraw();
            }
            
            free(full_title_path);
            free(full_ticket_path);
        }
    }

    if (!modified)
        setCanExit(TRUE);

    return modified;
}

void forceUpdate()
{
    setCanExit(FALSE);

    if (writeDigest())
        WHBLogPrintf("digest.bin written");
    else
        WHBLogPrintf("Error writing digest.bin");
    WHBLogConsoleDraw();

    // we need to recreate the update folder or else the update will fail

    deleteFolder(UPDATE_FOLDER_PATH);
    mkdir(UPDATE_FOLDER_PATH, 0777);
}