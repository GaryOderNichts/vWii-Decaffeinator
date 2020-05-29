#include "menu.h"

#include <coreinit/screen.h>

#include "title_strings.h"
#include "main.h"

int getMenuSize()
{
    return sizeof(mainMenuOptions) / 4;
}

void drawMenu(int selected, Region region)
{
    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);

    int drawIndex = -1, menuIndex = 0;
    for (int i = 0; i < sizeof(mainMenuHeader) / 4; i++)
    {
        if (i == 2)
        {
            console_print_pos(0, ++drawIndex, "|Detected Region: %s", regionStrings[region]);
            console_print_pos(40, drawIndex, "|");
        }

        console_print_pos(0, ++drawIndex, mainMenuHeader[i]);
    }

    drawIndex++;

    for (int i = 0; i < sizeof(mainMenuOptions) / 4; i++)
    {
        if (selected == menuIndex)
            console_print_pos(0, ++drawIndex, "--> %s", mainMenuOptions[i]);
        else 
            console_print_pos(0, ++drawIndex, "    %s", mainMenuOptions[i]);

        if (i == 2 || i == 3)
            drawIndex++;
        
        menuIndex++;
    }

    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
}

void drawWarning()
{
    OSScreenClearBufferEx(SCREEN_TV, 0xb30505FF);
    OSScreenClearBufferEx(SCREEN_DRC, 0xb30505FF);

    console_print_pos(0, 0, "WARNING! WARNING! WARNING! WARNING! WARNING! WARNING!");
    console_print_pos(0, 2, "Aggressive mode will delete EVERYTHING!");
    console_print_pos(0, 3, "Save Data, Channels, ...");
    console_print_pos(0, 6, "Press START to begin, any other button to cancel");

    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
}

void drawCleanConfirm()
{
    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);

    console_print_pos(0, 0, "Light Mode");
    console_print_pos(0, 1, "This will restore every essential sytem title");
    console_print_pos(0, 3, "Press START to begin, any other button to cancel");

    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
}

void drawForceUpdateConfirm()
{
    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);

    console_print_pos(0, 0, "Force Update");
    console_print_pos(0, 1, "This will force a system update");
    console_print_pos(0, 3, "Press START to begin, any other button to cancel");

    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
}

void drawUpdateInfo()
{
    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);

    console_print_pos(0, 0, "Done!");
    console_print_pos(0, 1, "Now update your console from System Settings");
    console_print_pos(0, 3, "Press any button to launch System Settings");

    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
}

int getAdvancedSize()
{
    return sizeof(advancedOptions) / 4;
}

BOOL isEnabled(int index, BOOL* enabled)
{
    return enabled[index];
}

int start = 0;
int end = ADV_PER_PAGE_DRC;
void drawAdvancedMenu(int index, BOOL* enabled)
{
    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);

    int drawIndex = -1;
    console_print_pos(0, ++drawIndex, "Advanced Options - Use the DPAD to select");
    console_print_pos(0, ++drawIndex, "Press B to return or START to start the restore process");

    drawIndex++;

    if (index >= end)
    {
        end = index + 1;
        start = end - ADV_PER_PAGE_DRC;
    }
    else if (index < start)
    {
        start = index;
        end = start + ADV_PER_PAGE_DRC;
    }

    for (int i = start; i < end; i++)
    {
        if (i == index)
        {
            console_print_pos(0, drawIndex + 1, "--> %s", advancedOptions[i]);
        }
        else
        {
            console_print_pos(0, drawIndex + 1, "    %s", advancedOptions[i]);
        }
        

        if (isEnabled(i, enabled))
        {
            console_print_pos(30, drawIndex + 1, ":  no <yes>");
        }
        else
        {
            console_print_pos(30, drawIndex + 1, ": <no> yes");
        }
        drawIndex++;
    }

    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
}