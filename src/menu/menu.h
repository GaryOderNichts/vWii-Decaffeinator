#ifndef _MENU_H_
#define _MENU_H_

#include "main.h"

#include "wut_types.h"

#define ADV_PER_PAGE_DRC 14

int getMenuSize(void);
void drawMenu(int selected, Region region);

void drawWarning(void);
void drawCleanConfirm(void);
void drawForceUpdateConfirm(void);
void drawSettingConfirm(void);
void drawUpdateInfo(void);

int getAdvancedSize(void);
void drawAdvancedMenu(int index, BOOL* enabled);

#endif