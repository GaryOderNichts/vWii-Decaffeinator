#ifndef _MENU_H_
#define _MENU_H_

#include "main.h"

#include "wut_types.h"

#define ADV_PER_PAGE_DRC 14

int getMenuSize();
void drawMenu(int selected, Region region);

void drawWarning();
void drawCleanConfirm();
void drawForceUpdateConfirm();
void drawUpdateInfo();

int getAdvancedSize();
void drawAdvancedMenu(int index, BOOL* enabled);

#endif