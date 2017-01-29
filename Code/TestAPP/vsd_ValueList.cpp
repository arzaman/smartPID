#include "vsd_GUI.h"
#include "ABM_Buttons.h"
#include "ABM_Cfg.h"
#include "IPAddress.h"
#include "run_mode.h"
#include "utlts.h"

extern void WaitSynchDisplay(void);

void VSD_GUI::DrawValueListVar(sValueList* lst)
{
  uint16_t w,x,y;
  x=gui->getPrintCol();
  y=gui->getPrintRow();
  switch(lst->type & ~VSD_TYPE_RO)
  {
  case lUINT8:{
	uint8_t var=*((uint8_t*)(lst->val));
	w=gui->getStrWidth(vsdGUI.sb_printf("%d",var));
	mGUI.setPrintPos(127-w,y);
	gui->print((char*)vsdGUI.print_buf);
	}break;
  case lUINT16:{
	uint16_t var=*((uint16_t*)(lst->val));
	w=gui->getStrWidth(vsdGUI.sb_printf("%d",var));
	mGUI.setPrintPos(127-w,y);
	gui->print((char*)vsdGUI.print_buf);
	}break;
  case lUINT32:{
	uint32_t val = *((uint32_t*)(lst->val));

	w = gui->getStrWidth(vsdGUI.sb_printf("%d", val));
	gui->setPrintPos(127 - w, y);
	gui->print((char *)vsdGUI.print_buf);
	}break;
  case lFLOAT:{
	float var=*((float*)(lst->val));
	w=gui->getStrWidth(vsdGUI.sb_printf("%0.1f",var));
	mGUI.setPrintPos(127-w,y);
	gui->print((char*)vsdGUI.print_buf);
  }break;
  case lSTR_LIST:{
	uint8_t var=*((uint8_t*)(lst->val));
	//	mGUI.setFont(u8g_font_profont10/*u8g_font_6x12r/*u8g_font_6x10*/);
	w=gui->getStrWidth(lst->names[var]);
	mGUI.setPrintPos(127-w,y);
	vsdGUI.printf(lst->names[var]);
  }break;
  case lTIME_HMS:{
	uint32_t var=*((uint32_t*)(lst->val));
    uint8_t h,m,s;
    h=var/(60*60);
    m=(var/60)%60;
    s=var%60;
	if(h>0) w=gui->getStrWidth(vsdGUI.sb_printf("%d:%02d:%02d",h,m,s));
	else w=gui->getStrWidth(vsdGUI.sb_printf("%d:%02d",m,s));
	mGUI.setPrintPos(127-w,y);
	gui->print((char*)vsdGUI.print_buf);
  }break;  
  case lSTR:{
	char *str = (char *)(lst->val);

	w = gui->getStrWidth(str);
	gui->setPrintPos(127 - w, y);
	gui->print(str);
  }break;
  case lIP_ADDR:{
	IPAddress *ip = (IPAddress *)(lst->val);

	w = gui->getStrWidth(vsdGUI.sb_printf("%d.%d.%d.%d", (*ip)[0], (*ip)[1],
			(*ip)[2], (*ip)[3]));
	gui->setPrintPos(127 - w, y);
	gui->print((char*)vsdGUI.print_buf);
  }break;
  }
}

int VSD_GUI::ChangeVar(int val, int min, int max)
{
uint8_t kb=NO_PRESS,sp;
int bp_val=val;

mGUI.setFont(u8g_font_profont22);

sp=gui->getFontLineSpacing();
while(!(kb==BUTTON_SS || kb==BUTTON_ENTER)){
	WaitSynchDisplay();
	DrawTextCenterFrame_cl(64,32+sp,vsdGUI.sb_printf("%d",val),lsSOLID,100);
	kb=GetKeyPressF();
	if(kb==BUTTON_UP)
	{val=IncValLim(val, max);}
	else if(kb==BUTTON_DOWN)
	{val=DecValLim(val, min);}
	RunBgTasks();
}
if(kb==BUTTON_SS) return bp_val;
return val;
}

uint32_t VSD_GUI::ChangeVarHMS(uint32_t val, int min, int max)
{
uint8_t kb=NO_PRESS,sp;
int bp_val=val;
uint8_t h,m,s;

mGUI.setFont(u8g_font_profont22);

sp=gui->getFontLineSpacing();
while(!(kb==BUTTON_SS || kb==BUTTON_ENTER)){
	WaitSynchDisplay();
	h=val/(60*60);	m=(val/60)%60;	s=val%60;
	if(h>0) DrawTextCenterFrame_cl(64,32+sp,vsdGUI.sb_printf("%d:%02d:%02d",h,m,s),lsSOLID,100);
	else DrawTextCenterFrame_cl(64,32+sp,vsdGUI.sb_printf("%d:%02d",m,s),lsSOLID,100);
//	DrawTextCenterFrame_cl(64,32+sp,vsdGUI.sb_printf("%d",val),lsSOLID,100);
	kb=GetKeyPressF();
	if(kb==BUTTON_UP)
	{val=IncValLim((int)val, max);}
	else if(kb==BUTTON_DOWN)
	{val=DecValLim((int)val, min);}
	RunBgTasks();
}
WaitFreeAllKey();
if(kb==BUTTON_SS) return bp_val;
return val;
}

uint32_t VSD_GUI::ChangeVarHMSw(uint32_t val, int min, int max)
{
uint8_t kb=NO_PRESS,sp;
int bp_val=val;
uint8_t h,m,s;
uint8_t first=true;

mGUI.setFont(u8g_font_profont22);

sp=gui->getFontLineSpacing();
while(!(kb==BUTTON_SS || kb==BUTTON_ENTER)){
	WaitSynchDisplay();
	h=val/(60*60);	m=(val/60)%60;	s=val%60;
	if(h>0) DrawTextCenterFrame_cl(64,32+sp,vsdGUI.sb_printf("%d:%02d:%02d",h,m,s),lsSOLID,100);
	else DrawTextCenterFrame_cl(64,32+sp,vsdGUI.sb_printf("%d:%02d",m,s),lsSOLID,100);
//	DrawTextCenterFrame_cl(64,32+sp,vsdGUI.sb_printf("%d",val),lsSOLID,100);
    if(first==true){
      while(CheckAllLongPress()!=NO_PRESS){};
      first=false;
    }
	kb=GetKeyPressF();
	if(kb==BUTTON_UP)
	{val=IncValLim((int)val, max);}
	else if(kb==BUTTON_DOWN)
	{val=DecValLim((int)val, min);}
	RunBgTasks();
}
if(kb==BUTTON_SS) {WaitFreeAllKey(); return bp_val;}
WaitFreeAllKey();
return val;
}

int VSD_GUI::ChangeVar(char **str, uint8_t val, int min, int max)
{
uint8_t kb=NO_PRESS;
uint8_t bp_val=val;
uint16_t w,cx,cy,h,hf;
gui->setFont(u8g_font_profont11);
//gui->setFontPosBottom();
w=MaxSizeStrings(str,max);
h=gui->getFontLineSpacing();
cy=32-((h*(max))/2)-1;
cx=64-w/2-2;
hf=h*(max)+2;
w+=2;
//SetBlinkPixelStyle(lsSOLID);
while(!(kb==BUTTON_SS || kb==BUTTON_ENTER)){
	WaitSynchDisplay();

  gui->setDefaultBackgroundColor();
  gui->drawBox(cx, cy, w, hf);
  gui->setDefaultForegroundColor();

	SetBlinkPixelStyle(lsSOLID);
  for(int i=0;i<3;i++)
  {
  DrawLineHRight(cx-i,cy-i,w+i*2);
  DrawLineVBot(cx-i+(w+i*2),cy-i,hf+i*2);
  DrawLineHLeft(cx-i+(w+i*2),cy-i+(hf+i*2),w+i*2);
  DrawLineVTop(cx-i,cy-i+(hf+i*2),hf+i*2);
  SetBlinkPixelStyle(lsCLR);
  }
  for(int i=0; i<max; i++)
  {
  if(i==val){
  gui->setDefaultForegroundColor();
  gui->drawBox(cx+1, cy+h*i+h/2-3, w-1, h+1);
  gui->setDefaultBackgroundColor();
  }
  else
  {
  gui->setDefaultForegroundColor();
  }
  DrawTextCenter(64, cy+h*i+h*2-3, str[i]);
  }
	//	DrawTextCenterFrame_cl(64,32,vsdGUI.sb_printf("%d",val),lsSOLID,100);
	kb=GetKeyPressF();

	if(kb==BUTTON_UP)
	{val=DecValLim(val, min);}
	else if(kb==BUTTON_DOWN)
	{val=IncValLim(val, max-1);}
	RunBgTasks();
}
if(kb==BUTTON_SS) return bp_val;
return val;
}

float VSD_GUI::ChangeVar(float val, int min, int max)
{
uint8_t kb=NO_PRESS,sp;
float bp_val=val;

mGUI.setFont(u8g_font_profont22);
//	WaitSynchDisplay();
//ClrDisp_dt();
sp=gui->getFontLineSpacing();
while(!(kb==BUTTON_SS || kb==BUTTON_ENTER)){
	WaitSynchDisplay();
	DrawTextCenterFrame_cl(64,32+sp,vsdGUI.sb_printf("%0.1f",val),lsSOLID, 100);
	kb=GetKeyPressF();
	if(kb==BUTTON_UP)
	{val=IncValLim(val, max);}
	else if(kb==BUTTON_DOWN)
	{val=DecValLim(val, min);}
	RunBgTasks();
}
if(kb==BUTTON_SS) return bp_val;
return val;
}

void VSD_GUI::ChangeValueListVar(sValueList* lst)
{
  switch(lst->type)
  {
  case lUINT8:{
	uint8_t var=*((uint8_t*)(lst->val));
	var=ChangeVar(var,lst->min,lst->max);
	*((uint8_t*)(lst->val))=var;
	}break;
  case lUINT16:{
	uint16_t var=*((uint16_t*)(lst->val));
	var=ChangeVar(var,lst->min,lst->max);
	*((uint16_t*)(lst->val))=var;
	}break;
  case lFLOAT:{
	float var=*((float*)(lst->val));
	var=ChangeVar(var,lst->min,lst->max);
	*((float*)(lst->val))=var;
  }break;
  case lSTR_LIST:{
	uint8_t var=*((uint8_t*)(lst->val));
	var=ChangeVar(lst->names,var,lst->min,lst->max);
	*((uint8_t*)(lst->val))=var;
  }break;
    case lTIME_HMS:{
	uint32_t var=*((uint32_t*)(lst->val));
	var=ChangeVarHMS(var,lst->min,lst->max);
	*((uint32_t*)(lst->val))=var;
  }break;  
  }
}

void VSD_GUI::DrawValueList(const char *title, sValueList* lst, uint8_t count,
		void (*check_cb)(int))
{
  uint16_t h;
  uint8_t kb;
  uint8_t indx_offs=0,indx_pos=0;
  uint8_t multiline_offs, line_count, printed_lines;

  for(;;)
  {
	WaitSynchDisplay();
	vsdGUI.ClrDisp();
	DrawFrame(title);
	mGUI.setFont(u8g_font_profont11/*u8g_font_6x10*/);
	h=gui->getFontLineSpacing();
	multiline_offs = 0;

	for(int i = indx_offs;(i<count) && (i+multiline_offs<(indx_offs+5));i++)
	{
	  mGUI.setFontPosTop();
	  if (lst[i].type != lSTR_MULTILINE) {
		mGUI.setPrintPos(2,
				(h + 1) * (i + multiline_offs - indx_offs) + h + 3);
		if (i == indx_pos)
		  vsdGUI.printf_n(lst[i].str);
		else
		  vsdGUI.printf(lst[i].str);
	  } else {
		mGUI.setPrintPos(gui->getWidth() / 2,
				(h + 1) * (i + multiline_offs - indx_offs) + h + 3);
		if (i == indx_pos)
		  vsdGUI.printf_center_n(lst[i].str);
		else
		  vsdGUI.printf_center(lst[i].str);
		printed_lines = DrawMultiLine(&lst[i], &line_count);
		multiline_offs += printed_lines;
		if ((i == indx_pos) && (printed_lines < line_count))
			/* Couldn't display the entire content of a selected item, move
			 * everything up one position. */
			indx_offs++;
	  }
	  if ((lst[i].type != lSTR_MULTILINE) && (lst[i].type != lFULL_SCREEN))
		  DrawValueListVar(&lst[i]);
	}

 	kb=GetKeyPressF();
	if(kb==BUTTON_UP)
	{DecVal(&indx_pos, count);}
	else if(kb==BUTTON_DOWN)
	{
	  IncVal(&indx_pos, count);
	}
	else if(kb==BUTTON_SS) break;
	else if(kb==BUTTON_ENTER)
	{
	  if (lst[indx_pos].type != lFULL_SCREEN) {
		if(check_cb!=NULL) check_cb(0);
		ChangeValueListVar(&lst[indx_pos]);
		if(check_cb!=NULL) check_cb(1);
	  } else
		DrawFullScreen(&lst[indx_pos]);
    }
	if (indx_pos + multiline_offs > (indx_offs + 4))
	  indx_offs = indx_pos + multiline_offs - 4;
	else if (indx_pos < indx_offs)
		indx_offs = indx_pos;
	RunBgTasks();
  }
}

int VSD_GUI::DrawSelectionList(const char *title, const char *itemNames[],
  unsigned int itemCount)
{
  uint16_t h;
  uint8_t kb;
  uint8_t indx_offs = 0, indx_pos = 0;
  uint8_t line_count;

  for (;;) {
	WaitSynchDisplay();
	vsdGUI.ClrDisp();
	DrawFrame(title);
	mGUI.setFont(u8g_font_profont11);
	h = gui->getFontLineSpacing();
	for (int i = indx_offs; (i < itemCount) && (i < (indx_offs + 5)); i++) {
	  mGUI.setFontPosTop();
	  mGUI.setPrintPos(2, (h + 1) * (i - indx_offs) + h + 3);
	  if (i == indx_pos)
		vsdGUI.printf_n(itemNames[i]);
	  else
		vsdGUI.printf(itemNames[i]);
	}
	kb = GetKeyPressF();
	if (kb == BUTTON_UP)
	  DecVal(&indx_pos, itemCount);
	else if (kb == BUTTON_DOWN)
	  IncVal(&indx_pos, itemCount);
	else if (kb == BUTTON_SS) {
	  indx_pos = -1;
	  break;
	} else if (kb == BUTTON_ENTER)
	  break;
	if (indx_pos > (indx_offs + 4))
	  indx_offs = indx_pos + - 4;
	else if (indx_pos < indx_offs)
		indx_offs = indx_pos;
  }

  /* Before exiting, wait for buttons to be released. In this way, all button
   * events generated while in this function don't appear as "ghost events" in
   * the outer code. */
  WaitFreeAllKey();

  return (int8_t)indx_pos;
}

int VSD_GUI::DrawMultiLine(sValueList *lst, uint8_t *line_count)
{
  if (line_count)
	*line_count = getLineCount((char *)(lst->val), gui->getWidth() - 4);
  gui->setPrintPos(2, gui->getPrintRow() + gui->getFontLineSpacing());
  return print_multiline((char *)(lst->val), 2, gui->getWidth() - 4);
}

void VSD_GUI::DrawFullScreen(sValueList *lst)
{
  vsdFullScreenCB draw_cb = (vsdFullScreenCB)(lst->val);
  uint8_t kb = NO_PRESS;

  while (kb != BUTTON_SS) {
	WaitSynchDisplay();
	vsdGUI.ClrDisp();
	if (draw_cb(*this, lst, kb))
		break;
	RunBgTasks();
	kb = GetKeyPressF();
  }
}

void VSD_GUI::DrawFrame(const char *title)
{
  uint16_t h, w;

  gui->setFont(u8g_font_profont11);
  gui->setPrintPos(68, 62);
  h = gui->getFontLineSpacing();
  w = gui->getStrWidth(title);
  gui->setFontPosTop();
  gui->setPrintPos(2, 1);
  gui->print(title);
  gui->drawHLine(w + 2, h / 2 + 2, 128 - (w + 2));
  gui->drawVLine(0, h + 1, 64 - h - 1);
  gui->drawHLine(0, 63, 128 - 1);
  gui->drawVLine(127, h / 2 + 2, 64 - (h / 2) - 2);
}
