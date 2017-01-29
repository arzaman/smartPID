#include <U8glib.h>

#define OLED_NEG
#undef OLED_NEG

#define WIDTH 128
#define HEIGHT 64
#define PAGE_HEIGHT 64

//u8g_t u8g;

uint8_t oled_buffer[(WIDTH/8)*HEIGHT];

//******************************************************************************
uint8_t u8g_com_FB_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr)
{
  switch(msg)
  {
    case U8G_COM_MSG_INIT:
      break;
    case U8G_COM_MSG_STOP:
      break;

   
    case U8G_COM_MSG_CHIP_SELECT:
      /* arg_val contains the chip number, which should be enabled */
      break;


    case U8G_COM_MSG_WRITE_BYTE:
      break;
    case U8G_COM_MSG_WRITE_SEQ:
      break;
  }
  return 1;
}
//******************************************************************************
uint8_t r_u8g_dev_FB_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg)
{
  switch(msg)
  {
    case U8G_DEV_MSG_INIT:
//      u8g_InitCom(u8g, dev, U8G_SPI_CLK_CYCLE_300NS);
//      u8g_WriteEscSeqP(u8g, dev, u8g_dev_ssd1306_128x64_adafruit2_init_seq);
      break;
    case U8G_DEV_MSG_STOP:
      break;
    case U8G_DEV_MSG_PAGE_NEXT:
      {
//        u8g_pb_t *pb = (u8g_pb_t *)(dev->dev_mem);
//        u8g_WriteEscSeqP(u8g, dev, u8g_dev_ssd1306_128x64_data_start);    
//        u8g_WriteByte(u8g, dev, 0x0b0 | pb->p.page); /* select current page (SSD1306) */
//        u8g_SetAddress(u8g, dev, 1);           /* data mode */
//        if ( u8g_pb_WriteBuffer(pb, u8g, dev) == 0 )
//          return 0;
//        u8g_SetChipSelect(u8g, dev, 0);
      }
      break;
    case U8G_DEV_MSG_SLEEP_ON:
//      u8g_WriteEscSeqP(u8g, dev, u8g_dev_ssd13xx_sleep_on);    
      return 1;
    case U8G_DEV_MSG_SLEEP_OFF:
//      u8g_WriteEscSeqP(u8g, dev, u8g_dev_ssd13xx_sleep_off);    
      return 1;
  }
//  return u8g_dev_pb8v1_base_fn(u8g, dev, msg, arg);
  return 1;
}
//******************************************************************************
void r_u8g_set_pixel(u8g_pb_t *b, u8g_uint_t x, u8g_uint_t y, uint8_t color_index)
{
  register uint8_t mask;
  uint8_t *ptr = (uint8_t*)b->buf;
  
  y -= b->p.page_y0;
  mask = 1;
  y &= 0x07;
  mask <<= y;
  ptr += x;
  if ( color_index )
  {
    *ptr |= mask;
  }
  else
  {
    mask ^=0xff;
    *ptr &= mask;
  }
}
//******************************************************************************
static const uint8_t  PXsetBit[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 },
                      PXclrBit[] = { 0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE };

void u8g_set_pixel(u8g_pb_t *b, u8g_uint_t x, u8g_uint_t y, uint8_t color_index)
{

//  uint8_t *buffer = (uint8_t*)b->buf;

//  if(buffer) {
//    if((x < 0) || (y < 0) || (x >= _width) || (y >= _height)) return;
  if(!(b->buf)) return;
/*
    int16_t t;
    switch(rotation) {
     case 1:
      t = x;
      x = WIDTH  - 1 - y;
      y = t;
      break;
     case 2:
      x = WIDTH  - 1 - x;
      y = HEIGHT - 1 - y;
      break;
     case 3:
      t = x;
      x = y;
      y = HEIGHT - 1 - t;
      break;
    }
*/
//    int b_indx=(x / 8) + y * ((WIDTH + 7) / 8);
//    uint8_t *ptr = &((uint8_t*)b->buf)[b_indx];
//    if(color_index/*color*/) *ptr |= PXsetBit[x & 7];
//    else      *ptr &= PXclrBit[x & 7];
    int b_indx=(x) + ((y & ~7)<<4);
    uint8_t *ptr = &((uint8_t*)b->buf)[b_indx];
#if defined(OLED_NEG)
    if(color_index/*color*/) *ptr &= ~(1<<(y & 7));
    else *ptr |= 1<<(y & 7);     
#else    
    if(color_index/*color*/) *ptr |= 1<<(y & 7);
    else      *ptr &= ~(1<<(y & 7));
#endif
}  
//******************************************************************************
void u8g_SetPixel(u8g_pb_t *b, const u8g_dev_arg_pixel_t * const arg_pixel)
{
  if ( arg_pixel->y < b->p.page_y0 )
    return;
  if ( arg_pixel->y > b->p.page_y1 )
    return;
  if ( arg_pixel->x >= b->width )
    return;
  u8g_set_pixel(b, arg_pixel->x, arg_pixel->y, arg_pixel->color);
}
//******************************************************************************
void u8g_FB_Set8Pixel(u8g_pb_t *b, u8g_dev_arg_pixel_t *arg_pixel)
{
  register uint8_t pixel = arg_pixel->pixel;
  u8g_uint_t dx = 0;
  u8g_uint_t dy = 0;
  
  switch( arg_pixel->dir )
  {
    case 0: dx++; break;
    case 1: dy++; break;
    case 2: dx--; break;
    case 3: dy--; break;
  }
  
  do
  {
    if ( pixel & 128 )
      u8g_SetPixel(b, arg_pixel);
    arg_pixel->x += dx;
    arg_pixel->y += dy;
    pixel <<= 1;
  } while( pixel != 0  );
  
}
//******************************************************************************
void u8g_FB_Clear(u8g_pb_t *b)
{
  uint8_t *ptr = (uint8_t *)b->buf;
  uint8_t *end_ptr = ptr;
  end_ptr += (WIDTH/8)*HEIGHT;//b->width*2;
  do
  {
#if defined(OLED_NEG)
    *ptr++ = 0xff;
#else
    *ptr++ = 0;
#endif
  } while( ptr != end_ptr );
}
//******************************************************************************
uint8_t u8g_dev_FB_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg)
{
  u8g_pb_t *pb = (u8g_pb_t *)(dev->dev_mem);
  switch(msg)
  {
    case U8G_DEV_MSG_SET_8PIXEL:
      if ( u8g_pb_Is8PixelVisible(pb, (u8g_dev_arg_pixel_t *)arg) )
        u8g_FB_Set8Pixel(pb, (u8g_dev_arg_pixel_t *)arg);
      break;
    case U8G_DEV_MSG_SET_PIXEL:
        u8g_SetPixel(pb, (u8g_dev_arg_pixel_t *)arg);
      break;
    case U8G_DEV_MSG_INIT:
      break;
    case U8G_DEV_MSG_STOP:
      break;
    case U8G_DEV_MSG_PAGE_FIRST:
      u8g_FB_Clear(pb);
      u8g_page_First(&(pb->p));
      break;
    case U8G_DEV_MSG_PAGE_NEXT:
//      if ( u8g_page_Next(&(pb->p)) == 0 )
        return 0;
//      u8g_pb14v1_Clear(pb);
      break;
#ifdef U8G_DEV_MSG_IS_BBX_INTERSECTION
    case U8G_DEV_MSG_IS_BBX_INTERSECTION:
      return u8g_pb_IsIntersection(pb, (u8g_dev_arg_bbx_t *)arg);
#endif
    case U8G_DEV_MSG_GET_PAGE_BOX:
      u8g_pb_GetPageBox(pb, (u8g_box_t *)arg);
      break;
    case U8G_DEV_MSG_GET_WIDTH:
      *((u8g_uint_t *)arg) = pb->width;
      break;
    case U8G_DEV_MSG_GET_HEIGHT:
      *((u8g_uint_t *)arg) = pb->p.total_height;
      break;
    case U8G_DEV_MSG_SET_COLOR_ENTRY:
      break;
    case U8G_DEV_MSG_SET_XY_CB:
      break;
    case U8G_DEV_MSG_GET_MODE:
      return U8G_MODE_BW;
  }
  return 1;
}
//******************************************************************************
//U8G_PB_DEV(u8g_dev_null, WIDTH, HEIGHT, PAGE_HEIGHT, u8g_dev_null_fn, u8g_com_null_fn);
//#define U8G_PB_DEV(name, width, height, page_height, dev_fn, com_fn) \
//uint8_t name##_buf[width] U8G_NOCOMMON ; \
//u8g_pb_t name##_pb = { {page_height, height, 0, 0, 0},  width, name##_buf}; \
//u8g_dev_t name = { dev_fn, &name##_pb, com_fn }

//uint8_t u8g_dev_FB_buf[WIDTH] U8G_NOCOMMON;
u8g_pb_t u8g_dev_FB_pb = { {PAGE_HEIGHT, HEIGHT, 0, 0, 0},  WIDTH, oled_buffer/*u8g_dev_FB_buf*/};
u8g_dev_t u8g_dev_FB = {u8g_dev_FB_fn, &u8g_dev_FB_pb, u8g_com_FB_fn};

U8GLIB mGUI(&u8g_dev_FB, u8g_com_FB_fn);

//******************************************************************************
void u8g_setup(void)
{  
mGUI.firstPage();
//  u8g_InitComFn(&u8g, &u8g_dev_FB, u8g_com_FB_fn);
  //u8g_SetDefaultForegroundColor(&u8g);
//  u8g_FirstPage(&u8g);
}
//******************************************************************************
void FB_Clear(void)
{
for(int i=0;i<((WIDTH/8)*HEIGHT);i++)
{
#if defined(OLED_NEG)
oled_buffer[i]=0xff;
#else
oled_buffer[i]=0;
#endif
}
}
//******************************************************************************
