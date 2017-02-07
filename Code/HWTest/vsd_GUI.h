#ifndef VSD_GUI_H
#define VSD_GUI_H

#include <U8glib.h>

#define sCOUNT(x) ((sizeof(x)/sizeof(x[0])))

#define VSD_TYPE_RO	0x80	/* read-only values */

void DrawOutputs(void);
void DrawOutputs(uint16_t nX, uint16_t nY);

typedef struct{
	const char *str;
	char **names;
	void *val;
	int16_t min;
	int16_t max;
	uint8_t type;
}sValueList;

enum list_types{
	lUINT8,
	lUINT16,
	lUINT32,
	lFLOAT,
	lSTR_LIST,
	lTIME_HMS,
	lSTR,
	lIP_ADDR,

	/* read-only values */
	lUINT8_RO = lUINT8 | VSD_TYPE_RO,
	lUINT16_RO,
	lUINT32_RO,
	lFLOAT_RO,
	lSTR_LIST_RO,
	lTIME_HMS_RO,
	lSTR_RO,
	lIP_ADDR_RO,
	lSTR_MULTILINE,
	lFULL_SCREEN,
};

typedef struct {
uint16_t x;
uint16_t y;
uint16_t w;
uint16_t h;
}s_vsdBOX;

typedef struct{
  s_vsdBOX box;
  uint8_t *text;
  uint8_t sstr;
  uint8_t count;
  uint8_t indx;
}vsdListBOX;

typedef struct{
	char **strs;
	uint8_t count;
	uint8_t curr_indx;
}vsdStringGroup;

typedef struct{
	const char *strName;
	bool (*cbFunc)(void);
}vsdMenuStrCb;

typedef struct{
	vsdMenuStrCb *StrCb;
	uint8_t count;
	uint8_t curr_indx;
}vsdStrCbGroup;

typedef struct{

}sMenu;

enum line_style{
	lsNONE,
	lsCLR,
	lsSOLID,
	lsDOTED,
	lsDOTED_12,
	lsDOTED_13,
	lsDOTED_14,
	lsDASHED_21,
	lsDASHED_31,
	lsDASHED_22,
	lsDASHED_32,
};

class VSD_GUI/* : public U8GLIB*/{

	U8GLIB *gui;
	uint16_t blink_pixel_count;
	uint8_t blink_set_val;
	uint8_t blink_clr_val;
	uint8_t blink_pixel_style;

	uint8_t print_buf[128];

public:
  VSD_GUI(U8GLIB *cgui) { gui=cgui; }

  void SetBlinkPixelCount(uint16_t bl){ blink_pixel_count=bl; }
  void SetBlinkPixelStyle(enum line_style style) { blink_pixel_style = style; }

  void DrawListBox(vsdListBOX *lbox);
  void DrawGroupBoxCentr(vsdStringGroup *str_gr, uint16_t x, uint16_t y);
  void DrawGroupBoxCentr(vsdStrCbGroup *str_gr, uint16_t x, uint16_t y,
		  uint8_t line_offset = 0, uint8_t line_count = 0);

  uint8_t MaxSizeStrings(char **str, uint8_t cnt);
  uint8_t MaxSizeStrings(vsdStrCbGroup **StrCb);
  uint8_t MaxSizeStrings(vsdStringGroup *str_gr);

  void DrawTextCenter(uint16_t x, uint16_t y, const char *s);
  void DrawTextCenterOvr(uint16_t x, uint16_t y, const char *s, uint8_t neg);
  void DrawTextCenterFrame(uint16_t x, uint16_t y, const char *s);
  void DrawTextCenterFrame(uint16_t x, uint16_t y, const char *s, enum line_style style);
  void DrawTextCenterFrame_cl(uint16_t x, uint16_t y, const char *s, enum line_style style, int width=0);
  void DrawTextCenterFrame(uint16_t x, uint16_t y, const char *s, enum line_style style, uint16_t width);
  void DrawPixelBlink(uint16_t x, uint16_t y);

  void DrawLineVTop(uint16_t x, uint16_t y, uint16_t l);
  void DrawLineVBot(uint16_t x, uint16_t y, uint16_t l);
  void DrawLineHLeft(uint16_t x, uint16_t y, uint16_t l);
  void DrawLineHRight(uint16_t x, uint16_t y, uint16_t l);

  void DrawTimeHMS(uint32_t seconds);
  void DrawTimeHMS(uint16_t x, uint16_t y, uint32_t seconds);
  void DrawTimeHMS(uint16_t x, uint16_t y, uint32_t seconds, const u8g_fntpgm_uint8_t *font);

  int printf(const char *str, ...);
  int printf_n(const char *str, ...);
  int printf_p(char *str, ...);
  int printf_aright(const char *str, ...);
  int printf_center(const char *str, ...);

  /* Print a formatted string with black characters on a white background, with
   * center alignment
   * Parameters: printf-style
   * Returns: number of printed characters */
  int printf_center_n(const char *str, ...);

  /* Print a string, occupying one or more display lines
   * Parameters:
   * - str: NULL-terminated string to be printed
   * - x_offset: horizontal offset on the display where each line should start,
   *     expressed in pixels
   * - line_width: maximum width to be occupied by each line, expressed in
   *     pixels
   * Returns: number of occupied lines */
  int print_multiline(const char *str, int x_offset, int line_width);

  char* sb_printf(const char *str, ...);

  /* Get the number of pixels on the horizontal direction needed to print a
   * string on the display
   * This method supports multi-line strings (in which case the returned number
   * is the sum of the pixels of each line).
   * This method assumes that all characters in the string have equal spacing.
   * Parameters:
   * - str: NULL-terminated string to be printed
   * Returns: number of pixels needed to print the string */
  int getStrWidth(const char *str);

  /* Get the number of display lines needed to print a string on the display
   * Parameters:
   * - str: NULL-terminated string to be printed
   * - line_width: maximum width to be occupied by each line, expressed in
   *     pixels
   * Returns: number of lines needed to print the string */
  int getLineCount(const char *str, int line_width);

  void ClrDisp(void);
  void ClrDisp_dt(void);
  
  void DrawValueList(const char *,sValueList*,uint8_t count,
		  void (*check_cb)(int) = NULL);
  void DrawValueListVar(sValueList* lst);

  /* Draw a list of items, and return the item selected by the user
   * Parameters:
   * - title: string to be displayed at the top of the screen
   * - itemNames: array of strings corresponding to the list items to be
   *              displayed
   * - itemCount: number of items to be displayed
   * Returns: zero-based index of item selected by the user, or a negative value
   *          if no item was selected (i.e. the user pressed the SS button)
   * */
  int DrawSelectionList(const char *title, const char *itemNames[],
    unsigned int itemCount);

  /* Draw the string value of a list item, occupying one or more display lines
   * Parameters:
   * - lst: pointer to list item
   * - line_count: if not NULL, pointer where the number of lines corresponding
   *     to the string length is stored
   * Returns: number of lines occupied when drawing the item value, which may be
   * lower than the number of lines corresponding to the string length if the
   * string didn't fit in the display
   * */
  int DrawMultiLine(sValueList *lst, uint8_t *line_count);

  /* Draw a list item in full-screen
   * Parameters:
   * - lst: pointer to list item
   * Returns: none
   * */
  void DrawFullScreen(sValueList *lst);

  /* Draw a display-sized frame with a title string superimposed at the top left
   * Parameters:
   * - title: string to be superimposed on the top left corner of the frame
   * Returns: none
   * */
  void DrawFrame(const char *title);

  void ChangeValueListVar(sValueList* lst);
  int ChangeVar(int val, int min, int max);
  int ChangeVar(char **str, uint8_t val, int min, int max);
  float ChangeVar(float val, int min, int max);
  uint32_t ChangeVarHMS(uint32_t val, int min, int max);
  uint32_t ChangeVarHMSw(uint32_t val, int min, int max);

  void DrawListMenu(const char *title, vsdStrCbGroup* lmenu);
  
};

/* Callback to draw a list item in full-screen
 * Parameters:
 * - vsdGUI: reference to the VSD_GUI object that handles the GUI
 * - lst: pointer to list item whose contents have to be displayed
 * - keypress: key press received from the user (can be used to change displayed
 *     contents depending on key presses, e.g. to scroll large content via the
 *     UP and DOWN buttons). Note: BUTTON_SS key presses are not passed in this
 *     argument, because when the calling code detect such key press it stops
 *     calling this function; in order to be able to intercept all key presses,
 *     this function should repeatedly call GetKeyPressF().
 * Returns: false if the list item should continue to be displayed, true if the
 *          GUI should exit from the current screen (e.g. due to user key
 *          presses)
 * */
typedef bool (*vsdFullScreenCB)(VSD_GUI &vsdGUI, sValueList *lst,
		uint8_t keypress);

extern U8GLIB mGUI;
extern VSD_GUI vsdGUI;
extern void WaitSynchDisplay(void);

#endif
