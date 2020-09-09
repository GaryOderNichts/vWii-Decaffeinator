#include "main.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include <whb/log.h>
#include <whb/log_console.h>
#include <whb/proc.h>

#include <coreinit/screen.h>
#include <coreinit/mcp.h>
#include <coreinit/ios.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <coreinit/foreground.h>
#include <coreinit/title.h>
#include <proc_ui/procui.h>
#include <sysapp/launch.h>
#include <sysapp/title.h>
#include <vpad/input.h>

#include <iosuhax.h>
#include <iosuhax_devoptab.h>

#include "utils/digest_utils.h"
#include "utils/vwii_cleaner.h"
#include "utils/setting_generator.h"
#include "menu/menu.h"

static BOOL canExit = TRUE;
static BOOL exitApplication = FALSE;

void setCanExit(BOOL inCanExit)
{
    canExit = inCanExit;
}

void console_print_pos(int x, int y, const char* format, ...)
{
	char* tmp = NULL;

	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va) >= 0) && tmp)
	{
        if(strlen(tmp) > 79)
            tmp[79] = 0;

        OSScreenPutFontEx(SCREEN_TV, x, y, tmp);
        OSScreenPutFontEx(SCREEN_DRC, x, y, tmp);

	}
	va_end(va);

	if(tmp)
        free(tmp);
}

BOOL AppFromHBL()
{
    uint64_t titleID = OSGetTitleID();
    return (titleID == HBL_TITLE_ID ||
        titleID == MII_MAKER_JPN_TITLE_ID ||
        titleID == MII_MAKER_USA_TITLE_ID ||
        titleID == MII_MAKER_EUR_TITLE_ID);
}

uint32_t AppQuitToHBL(void *context)
{
    if (canExit)
        exitApplication = TRUE;

    return 0;
}

//just to be able to call async
void someFunc(IOSError err, void *arg)
{
	(void)arg;
}

static int mcp_hook_fd = -1;
int MCPHookOpen(void)
{
	//take over mcp thread
	mcp_hook_fd = MCP_Open();
	if(mcp_hook_fd < 0)
		return -1;
	IOS_IoctlAsync(mcp_hook_fd, 0x62, (void*)0, 0, (void*)0, 0, someFunc, (void*)0);
	//let wupserver start up
	OSSleepTicks(OSMillisecondsToTicks(500));
	if(IOSUHAX_Open("/dev/mcp") < 0)
		return -1;
	return 0;
}

void MCPHookClose(void)
{
	if(mcp_hook_fd < 0)
		return;
	//close down wupserver, return control to mcp
	IOSUHAX_Close();
	//wait for mcp to return
	OSSleepTicks(OSMillisecondsToTicks(500));
	MCP_Close(mcp_hook_fd);
	mcp_hook_fd = -1;
}

void closeIOSUHAX(int fsaFd)
{
    unmount_fs("slc");
    unmount_fs("dev");
    unmount_fs("mlc");

    if (fsaFd >= 0)
    {
        IOSUHAX_FSA_Close(fsaFd);
    }

    if(mcp_hook_fd >= 0)
        MCPHookClose();
    else
        IOSUHAX_Close();
}

int main()
{
    BOOL fromHBL = AppFromHBL();
 
    if (fromHBL)
        OSEnableHomeButtonMenu(FALSE);
 
    ProcUIInit(&OSSavesDone_ReadyToRelease);
 
    if (fromHBL)
        ProcUIRegisterCallback(PROCUI_CALLBACK_HOME_BUTTON_DENIED, &AppQuitToHBL, NULL, 100);

    WHBLogConsoleInit();
    WHBLogConsoleSetColor(0);

    int fsaFd = -1;
    BOOL launchSettings = FALSE;

    MCPSysProdSettings* psettings = memalign(64, sizeof(MCPSysProdSettings)); // the settings need to be aligned to 64 for some reason
    int handle = MCP_Open();
    if (handle < 0)
    {
        WHBLogPrintf("Cannot open MCP");
        WHBLogConsoleDraw();
        OSSleepTicks(OSMillisecondsToTicks(5000));
		goto exit;
    }
    else
    {
        if (MCP_GetSysProdSettings(handle, psettings) < 0)
        {
            WHBLogPrintf("Error getting MCPSysProd");
            WHBLogConsoleDraw();
            OSSleepTicks(OSMillisecondsToTicks(5000));
		    goto exit;
        } 

        MCP_Close(handle);
    }
    MCPSysProdSettings settings = *psettings;
    free(psettings);

    Region region = REGION_UNSUPPORTED;
	if (settings.product_area == MCP_REGION_EUROPE)
        region = REGION_EUROPE;
	else if (settings.product_area == MCP_REGION_USA)
		region = REGION_USA;
	else if (settings.product_area == MCP_REGION_JAPAN)
		region = REGION_JAPAN;

	int res = IOSUHAX_Open(NULL);
	if (res < 0)
		res = MCPHookOpen();
	if(res < 0)
	{
		WHBLogPrintf("IOSUHAX_Open failed");
		WHBLogPrintf("Make sure to run CFW!");
        WHBLogConsoleDraw();
		OSSleepTicks(OSMillisecondsToTicks(5000));
		goto exit;
	}

	fsaFd = IOSUHAX_FSA_Open();
	if(fsaFd < 0)
	{
		WHBLogPrintf("IOSUHAX_FSA_Open failed");
        WHBLogConsoleDraw();
		OSSleepTicks(OSMillisecondsToTicks(5000));
		goto exit;
	}

	int mountres = mount_fs("slc", fsaFd, NULL, SLC_VOL_PATH);
	if(mountres < 0)
	{
		WHBLogPrintf("Mount of %s failed", SLC_VOL_PATH);
        WHBLogConsoleDraw();
		OSSleepTicks(OSMillisecondsToTicks(5000));
		goto exit;
	}

	mountres = mount_fs("dev", fsaFd, SLCCMPT_DEV_PATH, SLCCMPT_MOUNT_PATH);
	if(mountres < 0)
	{
		WHBLogPrintf("Mount of %s to %s failed", SLCCMPT_DEV_PATH, SLCCMPT_MOUNT_PATH);
        WHBLogConsoleDraw();
		OSSleepTicks(OSMillisecondsToTicks(5000));
		goto exit;
	}

    mountres = mount_fs("mlc", fsaFd, NULL, MLC_VOL_PATH);
	if(mountres < 0)
	{
		WHBLogPrintf("Mount of %s failed", MLC_VOL_PATH);
        WHBLogConsoleDraw();
		OSSleepTicks(OSMillisecondsToTicks(5000));
		goto exit;
	}

    drawMenu(0, region);

    int menuIndex = 0;
    int menuItemsAmount = getMenuSize();

    BOOL advancedMenu = FALSE;
    BOOL* advancedEnabled = malloc(getAdvancedSize() * sizeof(BOOL));
    memset(advancedEnabled, 0, getAdvancedSize() * sizeof(BOOL));

    VPADStatus vpad;
    ProcUIStatus status;
    while (((status = ProcUIProcessMessages(TRUE)) != PROCUI_STATUS_EXITING) && !exitApplication)
    {
        if(status == PROCUI_STATUS_RELEASE_FOREGROUND)
            ProcUIDrawDoneRelease();
        if(status != PROCUI_STATUS_IN_FOREGROUND)
            continue;

        VPADRead(VPAD_CHAN_0, &vpad, 1, NULL);

        if (vpad.trigger & VPAD_BUTTON_HOME && canExit)
        {
            AppQuitToHBL(NULL);
            continue;
        }

        if (vpad.trigger & VPAD_BUTTON_DOWN)
        {
            if (advancedMenu)
            {
                if (menuIndex < menuItemsAmount - 1)
                    drawAdvancedMenu(++menuIndex, advancedEnabled);
            }
            else
            {      
                if (menuIndex < menuItemsAmount - 1)
                    drawMenu(++menuIndex, region);
            }
            continue;
        }

        if (vpad.trigger & VPAD_BUTTON_UP)
        {
            if (advancedMenu)
            {
                if (menuIndex > 0)
                    drawAdvancedMenu(--menuIndex, advancedEnabled);                
            }
            else
            {
                if (menuIndex > 0)
                    drawMenu(--menuIndex, region);
            }
            continue;
        }

        if (vpad.trigger & VPAD_BUTTON_A)
        {
            if (!advancedMenu)
            {
                switch (menuIndex)
                {
                case 0:
                    drawCleanConfirm();
                    while (1)
                    {
                        VPADRead(VPAD_CHAN_0, &vpad, 1, NULL);

                        if (vpad.trigger & VPAD_BUTTON_PLUS)
                        {                
                            if (softClean())
                            {
                                forceUpdate();
                                OSSleepTicks(OSMillisecondsToTicks(2500));
                                drawUpdateInfo();
                                while (1)
                                {
                                    VPADRead(VPAD_CHAN_0, &vpad, 1, NULL);

                                    if (vpad.trigger != 0)
                                    {                                        
                                        launchSettings = TRUE;
                                        closeIOSUHAX(fsaFd);
                                        SYSLaunchSettings(NULL);
                                        break;
                                    }
                                }
                            }
                            break;
                        }

                        if (vpad.trigger != 0)
                            break;
                    }
                    break;

                case 1:
                    drawWarning();
                    while (1)
                    {
                        VPADRead(VPAD_CHAN_0, &vpad, 1, NULL);

                        if (vpad.trigger & VPAD_BUTTON_PLUS)
                        {
                            if (agressiveClean())
                            {
                                forceUpdate();
                                OSSleepTicks(OSMillisecondsToTicks(2500));
                                drawUpdateInfo();
                                while (1)
                                {
                                    VPADRead(VPAD_CHAN_0, &vpad, 1, NULL);

                                    if (vpad.trigger != 0)
                                    {                                        
                                        launchSettings = TRUE;
                                        closeIOSUHAX(fsaFd);
                                        SYSLaunchSettings(NULL);
                                        break;
                                    }
                                }
                                break;
                            }
                        }

                        if (vpad.trigger != 0)
                            break;
                    }       
                    break;

                case 2:
                    drawForceUpdateConfirm();
                    while (1)
                    {
                        VPADRead(VPAD_CHAN_0, &vpad, 1, NULL);

                        if (vpad.trigger & VPAD_BUTTON_PLUS)
                        {                
                            forceUpdate();
                            OSSleepTicks(OSMillisecondsToTicks(2500));
                            drawUpdateInfo();
                            while (1)
                            {
                                VPADRead(VPAD_CHAN_0, &vpad, 1, NULL);

                                if (vpad.trigger != 0)
                                {                                        
                                    launchSettings = TRUE;
                                    closeIOSUHAX(fsaFd);
                                    SYSLaunchSettings(NULL);
                                    break;
                                }
                            }
                            break;
                        }

                        if (vpad.trigger != 0)
                            break;
                    }
                    break;
                
                case 3:
                    drawSettingConfirm();
                    while (1)
                    {
                        VPADRead(VPAD_CHAN_0, &vpad, 1, NULL);

                        if (vpad.trigger & VPAD_BUTTON_PLUS)
                        {                
                            if (!regenerateSettingFile(fsaFd, settings, region))
                            {
                                WHBLogPrintf("Regeneration failed!");
                                WHBLogConsoleDraw();
                                // display the log longer when we errored
                                OSSleepTicks(OSMillisecondsToTicks(2500));
                            }
                            else
                            {
                                WHBLogPrintf("setting.txt successfully regenerated!");
                                WHBLogConsoleDraw();
                            }
                            
                            OSSleepTicks(OSMillisecondsToTicks(2500));
                            break;
                        }

                        if (vpad.trigger != 0)
                            break;
                    }
                    break;

                case 4:
                    advancedMenu = TRUE;
                    memset(advancedEnabled, 0, getAdvancedSize() * sizeof(BOOL));
                    drawAdvancedMenu(0, advancedEnabled);
                    menuIndex = 0;
                    menuItemsAmount = getAdvancedSize();
                    break;

                case 5:
                    goto exit;
                    break;
                }

                if (advancedMenu)
                    drawAdvancedMenu(menuIndex, advancedEnabled);
                else
                    drawMenu(menuIndex, region);

                continue;
            }  
        }

        if (vpad.trigger & VPAD_BUTTON_B)
        {
            if (advancedMenu)
            {
                advancedMenu = FALSE;
                menuIndex = 0;
                menuItemsAmount = getMenuSize();
                drawMenu(0, region);
                continue;
            }
        }

        if (vpad.trigger & (VPAD_BUTTON_LEFT | VPAD_BUTTON_RIGHT))
        {
            if (advancedMenu)
            {
                advancedEnabled[menuIndex] = !advancedEnabled[menuIndex];
                drawAdvancedMenu(menuIndex, advancedEnabled);
            }
        }

        if (vpad.trigger & VPAD_BUTTON_PLUS)
        {
            if (advancedMenu)
            {
                if (customClean(advancedEnabled, getAdvancedSize(), region))
                {
                    forceUpdate();
                    OSSleepTicks(OSMillisecondsToTicks(2500));
                    drawUpdateInfo();
                    while (1)
                    {
                        VPADRead(VPAD_CHAN_0, &vpad, 1, NULL);

                        if (vpad.trigger != 0)
                        {                                        
                            launchSettings = TRUE;
                            closeIOSUHAX(fsaFd);
                            SYSLaunchSettings(NULL);
                            break;
                        }
                    }
                }
                continue;
            }
        }
    }

exit: ;
    if (!launchSettings)
        closeIOSUHAX(fsaFd);

    WHBLogConsoleFree();

    ProcUIShutdown();

    if (fromHBL && !launchSettings)
        SYSRelaunchTitle(0, NULL);

    return 0;
}