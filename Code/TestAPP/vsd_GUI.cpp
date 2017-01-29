#include "ABM_HW_switch.h"
#include "vsd_GUI.h"
#include "wiring_digital.h"
#include <stdarg.h>

/* Force inclusion of the _printf_float symbol, in order to support floating
 * point specifiers in printf() format strings. */
asm (".global _printf_float");

VSD_GUI vsdGUI(&mGUI);

extern TDrive DriveOut[];



const char name_out[5][4]={
  "SSR",
  "DC1",
  "DC2",
  "RL1",
  "RL2"
};

//******************************************************************************
uint8_t readDriveSet(uint8_t m_indx)
{
  switch(m_indx)
  {
  case    0:  {return (digitalRead(DriveOut[OUT_SSR].pin)); break;}
  case    1:  {return (digitalRead(DriveOut[OUT_DC1].pin)); break;}
  case    2:  {return (digitalRead(DriveOut[OUT_DC2].pin)); break;}
  case    3:  {return (digitalRead(DriveOut[OUT_REL1].pin)); break;}
  case    4:  {return (digitalRead(DriveOut[OUT_REL2].pin)); break;}
  }
  return 0;
}
//******************************************************************************
void draw_LabelBox(char *str,uint8_t x,uint8_t y, uint8_t w,uint8_t h, uint8_t neg)
{
  uint8_t w_str,offs;
  //  mGUI.setFont(u8g_font_5x8);
  //  mGUI.setFont(u8g_font_5x7);
  //  mGUI.setFont(u8g_font_chikita);
  mGUI.setFont(u8g_font_04b_03);
  mGUI.setFontRefHeightText();
  mGUI.setFontPosTop();
  w_str=mGUI.getStrWidth(str);
  offs=((w/2-w_str/2)<0)?0:(w/2-w_str/2);
  if(neg==0)
  {
    mGUI.setDefaultForegroundColor();
    mGUI.drawFrame(x, y, w, h);
    //      mGUI.drawStr((x+offs, y+2 , str);
    //      mGUI.setDefaultBackgroundColor();
  }
  else
  {
	mGUI.setDefaultForegroundColor();
	mGUI.drawBox(x, y, w, h);
	mGUI.setDefaultBackgroundColor();
  }
  mGUI.drawStr(x+offs, y+1 , str);
  mGUI.setDefaultForegroundColor();
}
//******************************************************************************
void DrawOutputs(void)
{
  uint8_t x=1,y=9;
  for(int i=0;i<5;i++)
  {
    draw_LabelBox((char*)name_out[i],x,y,18,9,readDriveSet(i)/*i&1*/);
    y+=8;
  }
}
//******************************************************************************
void DrawOutputs(uint16_t nX, uint16_t nY)
{
  uint8_t x=nX,y=nY;
  for(int i=0;i<5;i++)
  {
    draw_LabelBox((char*)name_out[i],x,y,18,9,readDriveSet(i)/*i&1*/);
    y+=8;
  }
}
//******************************************************************************
void VSD_GUI::DrawListBox(vsdListBOX *lbox)
{

}
//******************************************************************************
void VSD_GUI::DrawGroupBoxCentr(vsdStringGroup *str_gr, uint16_t x, uint16_t y)
{
uint16_t w,h;
if(str_gr->count==0) return;
w=MaxSizeStrings(str_gr);
h=gui->getFontLineSpacing();
for(int i=0;i<str_gr->count;i++)
{
DrawTextCenterFrame(x,y+(h+1)*i,str_gr->strs[i],
		i==str_gr->curr_indx?lsDOTED:lsNONE,
		(uint16_t)w+2);
}

}
//******************************************************************************
void VSD_GUI::DrawGroupBoxCentr(vsdStrCbGroup *str_cb, uint16_t x, uint16_t y,
		uint8_t line_offset, uint8_t line_count)
{
uint16_t w,h;
if(str_cb->count==0) return;
if ((line_count == 0) || (line_count > (str_cb->count - line_offset)))
	line_count = str_cb->count - line_offset;
w=MaxSizeStrings(&str_cb);
h=gui->getFontLineSpacing();
for(int i=0;i<line_count;i++)
{
DrawTextCenterFrame(x,y+(h+1)*i,str_cb->StrCb[i+line_offset].strName,
		i+line_offset==str_cb->curr_indx?lsSOLID/*lsDOTED*/:lsNONE,
		(uint16_t)w+2);
}

}
//******************************************************************************
uint8_t VSD_GUI::MaxSizeStrings(char **str, uint8_t cnt)
{
  uint8_t rsize=0,t;
//  char *p=str;
  for(uint8_t i=0;i<cnt;i++)
  {
	t=mGUI.getStrWidth(str[i]);
	if(t>rsize) rsize=t;
  }
  return rsize;
}
//******************************************************************************
uint8_t VSD_GUI::MaxSizeStrings(vsdStrCbGroup **StrCb)
{
  uint8_t rsize=0,t;
//  char *p=str;
  for(uint8_t i=0;i<(*StrCb)->count;i++)
  {
	t=mGUI.getStrWidth(((*StrCb)->StrCb[i].strName));
	if(t>rsize) rsize=t;
  }
  return rsize;
}
//******************************************************************************
uint8_t VSD_GUI::MaxSizeStrings(vsdStringGroup *str_gr)
{
return MaxSizeStrings(str_gr->strs, /*str_gr->len_str,*/ str_gr->count);
}
//******************************************************************************
void VSD_GUI::DrawTextCenter(uint16_t x, uint16_t y, const char *s)
{
  uint16_t cx,cy;
  gui->setFontPosBottom();
  cy=y-(gui->getFontLineSpacing()/2);
  cx=x-(gui->getStrWidth(s)/2);
  gui->setPrintPos(cx,cy);
  gui->print(s);
}
//******************************************************************************
void VSD_GUI::DrawTextCenterOvr(uint16_t x, uint16_t y, const char *s, uint8_t neg)
{
  uint16_t cx,cy,w,h;
  cy=y-((h=gui->getFontLineSpacing())/2);
  cx=x-((w=gui->getStrWidth(s))/2);
  
  if(neg) gui->setDefaultForegroundColor(); else gui->setDefaultBackgroundColor();
  gui->drawBox(cx, cy-h, w, h);
  if(neg) gui->setDefaultBackgroundColor(); else gui->setDefaultForegroundColor();
  DrawTextCenter(x, y, s);
  gui->setDefaultForegroundColor();
}
//******************************************************************************
void VSD_GUI::DrawTextCenterFrame(uint16_t x, uint16_t y, const char *s)
{
  uint16_t cx,cy,w,h;
  cy=y-((h=gui->getFontLineSpacing())/2);
  cx=x-((w=gui->getStrWidth(s))/2);

  gui->drawFrame(cx-2, cy-h-2, w+3, h+2);
  DrawTextCenter(x, y, s);
}
//******************************************************************************
void VSD_GUI::DrawLineVTop(uint16_t x, uint16_t y, uint16_t l)
{
  for(int i=0;i<l;i++) DrawPixelBlink(x,y-i);
}
//******************************************************************************
void VSD_GUI::DrawLineVBot(uint16_t x, uint16_t y, uint16_t l)
{
  for(int i=0;i<l;i++) DrawPixelBlink(x,y+i);
}
//******************************************************************************
void VSD_GUI::DrawLineHLeft(uint16_t x, uint16_t y, uint16_t l)
{
  for(int i=0;i<l;i++) DrawPixelBlink(x-i,y);
}
//******************************************************************************
void VSD_GUI::DrawLineHRight(uint16_t x, uint16_t y, uint16_t l)
{
  for(int i=0;i<l;i++) DrawPixelBlink(x+i,y);
}
//******************************************************************************
void VSD_GUI::DrawTextCenterFrame(uint16_t x, uint16_t y, const char *s, enum line_style style)
{
  uint16_t cx,cy,w,h;
  cy=y-((h=gui->getFontLineSpacing())/2);
  cx=x-((w=gui->getStrWidth(s))/2);
  SetBlinkPixelCount(0);
  SetBlinkPixelStyle(style);
  //gui->drawFrame(cx-2, cy-h-1, w+3, h+1);

  cx=cx-2; cy=cy-h-1; w=w+3; h=h/*+1*/;
  DrawLineHRight(cx,cy,w);
  DrawLineVBot(cx+w,cy,h);
  DrawLineHLeft(cx+w,cy+h,w);
  DrawLineVTop(cx,cy+h,h);

  DrawTextCenter(x, y, s);
}
//******************************************************************************
void VSD_GUI::DrawTextCenterFrame_cl(uint16_t x, uint16_t y, const char *s, enum line_style style, int width)
{
  uint16_t cx,cy,w,h;
  cy=y-((h=gui->getFontLineSpacing())/2);
  if(width == 0){
  cx=x-((w=gui->getStrWidth(s))/2);
  }
  else{
  cx=x-((w=width)/2);
  }
  
  SetBlinkPixelCount(0);
  SetBlinkPixelStyle(style);

//  cx=cx-2; cy=cy-h-1; w=w+3; h=h/*+1*/;
  cx=cx-2; cy=cy-h-2; w=w+3; h=h+1;

  gui->setDefaultBackgroundColor();
  gui->drawBox(cx, cy, w, h);
  gui->setDefaultForegroundColor();
  
for(int i=0;i<3;i++)
{
  DrawLineHRight(cx,cy,w);
  DrawLineVBot(cx+w,cy,h);
  DrawLineHLeft(cx+w,cy+h,w);
  DrawLineVTop(cx,cy+h,h);
  SetBlinkPixelStyle(lsCLR);
  w+=2; h+=2; cx--; cy--;
}

  gui->setDefaultForegroundColor();
  DrawTextCenter(x, y, s);
}
//******************************************************************************
void VSD_GUI::DrawTextCenterFrame(uint16_t x, uint16_t y, const char *s, enum line_style style, uint16_t width)
{
  uint16_t cx,cy,w=width,h;
  cy=y-((h=gui->getFontLineSpacing())/2);
//  cx=x-((w=gui->getStrWidth(s))/2);
  cx=x-(w/2);
  SetBlinkPixelCount(0);
  SetBlinkPixelStyle(style);
  //gui->drawFrame(cx-2, cy-h-1, w+3, h+1);

//  cx=cx-2; cy=cy-h-1; w=w+3; h=h/*+1*/;
  cx=cx-2; cy=cy-h-2; w=w+3; h=h+2;

  DrawLineHRight(cx,cy,w);
  DrawLineVBot(cx+w,cy,h);
  DrawLineHLeft(cx+w,cy+h,w);
  DrawLineVTop(cx,cy+h,h);

  DrawTextCenter(x, y, s);
}
//******************************************************************************
void VSD_GUI::DrawPixelBlink(uint16_t x, uint16_t y)
{
  switch(blink_pixel_style)
  {
  case lsNONE:{}
  break;
  case lsCLR:{
	gui->setDefaultBackgroundColor();
	gui->drawPixel(x,y);
  }break;
  case lsSOLID:{
	gui->setDefaultForegroundColor();
	gui->drawPixel(x,y);
  }break;
  case lsDOTED:{
	if(blink_pixel_count&1) gui->setDefaultForegroundColor(); else gui->setDefaultBackgroundColor();
	gui->drawPixel(x,y);
  }break;
  case lsDOTED_12:{
	if((blink_pixel_count%3)<1) gui->setDefaultForegroundColor(); else gui->setDefaultBackgroundColor();
	gui->drawPixel(x,y);
  }break;
  case lsDOTED_13:{
	if((blink_pixel_count%4)<1) gui->setDefaultForegroundColor(); else gui->setDefaultBackgroundColor();
	gui->drawPixel(x,y);
  }break;
  case lsDOTED_14:{
	if((blink_pixel_count%5)<1) gui->setDefaultForegroundColor(); else gui->setDefaultBackgroundColor();
	gui->drawPixel(x,y);
  }break;
  case lsDASHED_21:{
	if((blink_pixel_count%3)<2) gui->setDefaultForegroundColor(); else gui->setDefaultBackgroundColor();
	gui->drawPixel(x,y);
  }break;
  case lsDASHED_31:{
	if((blink_pixel_count%4)<3) gui->setDefaultForegroundColor(); else gui->setDefaultBackgroundColor();
	gui->drawPixel(x,y);
  }break;
  case lsDASHED_22:{
	if((blink_pixel_count%4)<2) gui->setDefaultForegroundColor(); else gui->setDefaultBackgroundColor();
	gui->drawPixel(x,y);
  }break;
  case lsDASHED_32:{
	if((blink_pixel_count%5)<3) gui->setDefaultForegroundColor(); else gui->setDefaultBackgroundColor();
	gui->drawPixel(x,y);
  }break;

  }
  gui->setDefaultForegroundColor();
  blink_pixel_count++;
}
//******************************************************************************
int VSD_GUI::printf(const char *str, ...)
{
va_list args;
int size;

va_start( args, str );
  size=vsnprintf((char*)print_buf, 128, str, args);
va_end (args);

  gui->print((const char*)&print_buf[0]);
  return size;
}
//******************************************************************************
int VSD_GUI::printf_n(const char *str, ...)
{
va_list args;
int size,x,y,w,h;

va_start( args, str );
  size=vsnprintf((char*)print_buf, 128, str, args);
va_end (args);

	x=gui->getPrintCol();
	y=gui->getPrintRow();
	h=gui->getFontLineSpacing();
	w=gui->getStrWidth((const char*)&print_buf[0]);
  gui->setDefaultForegroundColor();
  y+=gui->getU8g()->font_calc_vref(gui->getU8g());

  gui->drawBox(x-1, y-h+1, w+1, h);
  //  gui->drawBox(x-1, y-h+1, w+1, h);
  gui->setDefaultBackgroundColor();
  gui->print((const char*)&print_buf[0]);
  gui->setDefaultForegroundColor();
  return size;
}
//******************************************************************************
int VSD_GUI::printf_p(char *str, ...)
{
va_list args;
int size,x,y,w,h;

va_start( args, str );
  size=vsnprintf((char*)print_buf, 128, str, args);
va_end (args);

	x=gui->getPrintCol();
	y=gui->getPrintRow();
	h=gui->getFontLineSpacing();
	w=gui->getStrWidth((const char*)&print_buf[0]);
  gui->setDefaultBackgroundColor();
  y+=gui->getU8g()->font_calc_vref(gui->getU8g());

  gui->drawBox(x-1, y-h+1, w+1, h);
  //  gui->drawBox(x-1, y-h+1, w+1, h);
  gui->setDefaultForegroundColor();
  gui->print((const char*)&print_buf[0]);
  gui->setDefaultForegroundColor();
  return size;
}
//******************************************************************************
int VSD_GUI::printf_aright(const char *str, ...)
{
va_list args;
int size,x,y,w,h;

va_start( args, str );
  size=vsnprintf((char*)print_buf, 128, str, args);
va_end (args);

	x=gui->getPrintCol();
	y=gui->getPrintRow();
	w=gui->getStrWidth((const char*)&print_buf[0]);
    gui->setPrintPos(x-w+1,y);
    gui->print((const char*)&print_buf[0]);
  return size;
}
//******************************************************************************
int VSD_GUI::printf_center(const char *str, ...)
{
va_list args;
int size,x,y,w,h;

va_start( args, str );
  size=vsnprintf((char*)print_buf, 128, str, args);
va_end (args);

	x=gui->getPrintCol();
	y=gui->getPrintRow();
	w=gui->getStrWidth((const char*)&print_buf[0]);
    gui->setPrintPos(x-(w/2),y);
    gui->print((const char*)&print_buf[0]);
  return size;
}
//******************************************************************************
int VSD_GUI::printf_center_n(const char *str, ...)
{
	va_list args;
	int size, x, y, w, h;

	va_start(args, str);
	size = vsnprintf((char*)print_buf, sizeof(print_buf), str, args);
	va_end(args);
	x = gui->getPrintCol();
	y = gui->getPrintRow();
	h = gui->getFontLineSpacing();
	w = gui->getStrWidth((const char*)&print_buf[0]);
	x -= w / 2;
	gui->setDefaultForegroundColor();
	gui->drawBox(x - 1,
			y + gui->getU8g()->font_calc_vref(gui->getU8g()) - h + 1, w + 1, h);
	gui->setDefaultBackgroundColor();
	gui->setPrintPos(x, y);
	gui->print((const char*)&print_buf[0]);
	gui->setDefaultForegroundColor();
	return size;
}
//******************************************************************************
int VSD_GUI::print_multiline(const char *str, int x_offset, int line_width)
{
	uint16_t w, y;
	int i, line_count, chars_per_line;
	char *rw_str = (char *)str;
	char tmp;

	w = getStrWidth(str);
	y = gui->getPrintRow();
	line_count = getLineCount(str, line_width);
	chars_per_line = line_width * strlen(str) / w;
	for (i = 0; i < line_count; i++) {
		gui->setPrintPos(x_offset, y + i * gui->getFontLineSpacing());
		if (gui->getPrintRow() + gui->getFontLineSpacing() > gui->getHeight()) {
			line_count = i;
			break;
		}
		if (i < line_count - 1) {
			tmp = str[chars_per_line * (i + 1)];
			rw_str[chars_per_line * (i + 1)] = '\0';
		}
		gui->print(str + i * chars_per_line);
		if (i < line_count - 1)
			rw_str[chars_per_line * (i + 1)] = tmp;
	}
	return line_count;
}
//******************************************************************************
char* VSD_GUI::sb_printf(const char *str, ...)
{
va_list args;
int size,x,y,w,h;

va_start( args, str );
  size=vsnprintf((char*)print_buf, 128, str, args);
va_end (args);

return (char*)print_buf;
}
//******************************************************************************
int VSD_GUI::getStrWidth(const char *str)
{
	char buf[2];

	buf[0] = str[0];
	buf[1] = '\0';
	return (gui->getStrWidth(buf) * strlen(str));
}
//******************************************************************************
int VSD_GUI::getLineCount(const char *str, int line_width)
{
	int w, line_count;

	w = getStrWidth(str);
	line_count = w / line_width;
	if (line_width * line_count < w)
		line_count++;
	return line_count;
}
//******************************************************************************
void VSD_GUI::DrawTimeHMS(uint32_t seconds)
{
uint8_t h,m,s;
h=seconds/(60*60);
m=(seconds/60)%60;
s=seconds%60;
if(h>0) printf((char*)"%d:%02d:%02d",h,m,s);
else printf((char*)"%d:%02d",m,s);
}
//******************************************************************************
void VSD_GUI::DrawTimeHMS(uint16_t x, uint16_t y, uint32_t seconds)
{
gui->setPrintPos(x,y);
DrawTimeHMS(seconds);
}
//******************************************************************************
void VSD_GUI::DrawTimeHMS(uint16_t x, uint16_t y, uint32_t seconds, const u8g_fntpgm_uint8_t *font)
{
gui->setFont(font);
DrawTimeHMS(x,y,seconds);
}
//******************************************************************************
extern void FB_Clear(void);
void VSD_GUI::ClrDisp(void)
{
FB_Clear();
}
//******************************************************************************
extern uint8_t oled_buffer[];
void VSD_GUI::ClrDisp_dt(void)
{
for(int i=0;i<((128/8)*64);i+=2)
{
oled_buffer[i]&=0xAA;
oled_buffer[i+1]&=0x55;
}
}
//******************************************************************************
//******************************************************************************

