#include "ABM_Buttons.h"
#include "ABM_Cfg.h"
#include "run_mode.h"
#include "vsd_GUI.h"
#include "utlts.h"

extern void WaitSynchDisplay(void);

void VSD_GUI::DrawListMenu(const char *title, vsdStrCbGroup* lmenu)
{
uint8_t kb;
gui->setFont(u8g_font_6x12r);
while(kb!=BUTTON_SS){
kb=GetKeyPressF();
if(kb==BUTTON_UP)
	DecVal(&(lmenu->curr_indx), lmenu->count);
else if(kb==BUTTON_DOWN)
	IncVal(&(lmenu->curr_indx), lmenu->count);
else if(kb==BUTTON_ENTER)
	if ((lmenu->StrCb[lmenu->curr_indx].cbFunc!=NULL)
			&& lmenu->StrCb[lmenu->curr_indx].cbFunc())
		break;
WaitSynchDisplay();
ClrDisp();
DrawGroupBoxCentr(lmenu,64,32);
gui->setPrintPos(1,0);
gui->setFontPosTop();
gui->print(title);
RunBgTasks();
}
WaitFreeAllKey();
}

void BigMessSetFontParam(void)
{
  WaitSynchDisplay();
  mGUI.firstPage();
  mGUI.setFont(u8g_font_10x20r);
  mGUI.setFontRefHeightText();
  mGUI.setFontPosTop();
}
void messInitEEprom(void)
{
  BigMessSetFontParam();
  mGUI.setPrintPos(10,20);
  mGUI.print("Init EEPROM");
}
void messInitFuseBit(void)
{
  BigMessSetFontParam();
  mGUI.setPrintPos(15,20);
  mGUI.print("Init Fuse");
}
void messReboot(void)
{
  BigMessSetFontParam();
  mGUI.setPrintPos(15,20);
  mGUI.print("go Reboot");
}
