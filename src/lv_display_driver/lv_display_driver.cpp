#include <Arduino.h>
//#include <Wire.h>
//#include <lv_conf.h>
#include <lvgl.h>
#include "lv_display_driver.h"

//#include "../ui/ui.h"
#include "panel_driver.h"

#include "FS.h"
#include <ffat.h>
struct FatFile {
  File file;
};

static const char* TAG = "lv display driver";

static const uint16_t screenWidth  = 320; //240;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
//static lv_color_t        *disp_draw_buf;
static lv_color_t        *disp_draw_buf1;
static lv_color_t        *disp_draw_buf2;
static lv_disp_drv_t      disp_drv;
static lv_indev_drv_t     indev_drv;
static lv_fs_drv_t        fs_drv;                /*Needs to be static or global*/

//LGFX gfx;

//void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p );
//void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data );
//void* fs_open(lv_fs_drv_t* drv, const char* path, lv_fs_mode_t mode);
//lv_fs_res_t fs_close(lv_fs_drv_t* drv, void* file_p);
//lv_fs_res_t fs_read(lv_fs_drv_t* drv, void* file_p, void* buf, uint32_t btr, uint32_t* br);
//lv_fs_res_t fs_write(lv_fs_drv_t* drv, void* file_p, const void* buf, uint32_t btw, uint32_t* bw);
//lv_fs_res_t fs_seek(lv_fs_drv_t* drv, void* file_p, uint32_t pos, lv_fs_whence_t whence);

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
  /*
    if (gfx.getStartCount() == 0)
    {   // Processing if not yet started
        gfx.startWrite();
    }
    gfx.pushImageDMA( area->x1
                    , area->y1
                    , area->x2 - area->x1 + 1
                    , area->y2 - area->y1 + 1
                    , ( lgfx::rgb565_t* )&color_p->full);
    lv_disp_flush_ready( disp );
    */
#ifndef DIRECT_MODE
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif
#endif // #ifndef DIRECT_MODE

  lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
/*
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    uint16_t touchX, touchY;

    data->state = LV_INDEV_STATE_REL;

    if( gfx.getTouch( &touchX, &touchY ) )
    {
        data->state = LV_INDEV_STATE_PR;

        / * Set the coordinates * /
        data->point.x = touchX;
        data->point.y = touchY;
    }
}
*/
/* ************************************************************************* *
 * Open a file
 * @param drv       pointer to a driver where this function belongs
 * @param path      path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param mode      read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return          a file descriptor or NULL on error
 * ************************************************************************* */
void* fs_open(lv_fs_drv_t* drv, const char* path, lv_fs_mode_t mode) {
  LV_UNUSED(drv);

  const char* flags = "";

  if (mode == LV_FS_MODE_WR)
    flags = FILE_WRITE;
  else if (mode == LV_FS_MODE_RD)
    flags = FILE_READ;
  else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
    flags = FILE_WRITE;

  //File f = LittleFS.open(path, flags);
  File f = FFat.open(path, flags);
  if (!f) {
    return NULL;
  }

  //LittleFile* lf = new LittleFile{f};
  FatFile* lf = new FatFile{f};

  return (void*)lf;
}

/* ************************************************************************* *
 * Close an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 * ************************************************************************* */
lv_fs_res_t fs_close(lv_fs_drv_t* drv, void* file_p) {
  LV_UNUSED(drv);
  //LittleFile* lf = (LittleFile*)file_p;
  FatFile* lf = (FatFile*)file_p;

  lf->file.close();

  delete lf;
  return LV_FS_RES_OK;
}

/* ************************************************************************* *
 * Read data from an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable.
 * @param buf       pointer to a memory block where to store the read data
 * @param btr       number of Bytes To Read
 * @param br        the real number of read bytes (Byte Read)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 * ************************************************************************* */
lv_fs_res_t fs_read(lv_fs_drv_t* drv, void* file_p, void* buf, uint32_t btr, uint32_t* br) {
  LV_UNUSED(drv);
  //LittleFile* lf = (LittleFile*)file_p;
  FatFile* lf = (FatFile*)file_p;

  *br = lf->file.read((uint8_t*)buf, btr);

  return (int32_t)(*br) < 0 ? LV_FS_RES_UNKNOWN : LV_FS_RES_OK;
}

/* ************************************************************************* *
 * Write into a file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable
 * @param buf       pointer to a buffer with the bytes to write
 * @param btw       Bytes To Write
 * @param bw        the number of real written bytes (Bytes Written). NULL if unused.
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 * ************************************************************************* */
lv_fs_res_t fs_write(lv_fs_drv_t* drv, void* file_p, const void* buf, uint32_t btw, uint32_t* bw) {
  LV_UNUSED(drv);
  //LittleFile* lf = (LittleFile*)file_p;
  FatFile* lf = (FatFile*)file_p;
  *bw = lf->file.write((uint8_t*)buf, btw);
  return (int32_t)(*bw) < 0 ? LV_FS_RES_UNKNOWN : LV_FS_RES_OK;
}

/* ************************************************************************* *
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open )
 * @param pos       the new position of read write pointer
 * @param whence    tells from where to interpret the `pos`. See @lv_fs_whence_t
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 * ************************************************************************* */
lv_fs_res_t fs_seek(lv_fs_drv_t* drv, void* file_p, uint32_t pos, lv_fs_whence_t whence) {
  LV_UNUSED(drv);
  SeekMode mode;
  if (whence == LV_FS_SEEK_SET)
    mode = SeekSet;
  else if (whence == LV_FS_SEEK_CUR)
    mode = SeekCur;
  else if (whence == LV_FS_SEEK_END)
    mode = SeekEnd;

  //LittleFile* lf = (LittleFile*)file_p;
  FatFile* lf = (FatFile*)file_p;
  lf->file.seek(pos, mode);
  return LV_FS_RES_OK;
}

void lv_display_driver_init(){
//  gfx.begin();
  // Set the rotation direction from 4 directions from 0 to 3.
  // (If you use rotations 4 to 7, the image will be mirrored.)
//  gfx.setRotation( 0 ); /* Landscape orientation, flipped */
  // Set the backlight brightness in the range 0-255
//  gfx.setBrightness(200);
  // Set the color mode as needed. (Initial value is 16)
  // 16 - Faster, but the red and blue tones are 5 bits.
  // 24 - Slower, but the gradation expression is cleaner.
//  gfx.setColorDepth(16);
//  gfx.fillScreen(TFT_WHITE);
// Init Display
  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
  gfx->setRotation( 1 );
  gfx->fillScreen(BLACK);

#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
  // LEDC
#endif
  gfx->setCursor(10, 10);
  gfx->setTextColor(WHITE);
  gfx->println("Wait! ...");

  lv_init();
  //disp_draw_buf = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * screenWidth * screenHeight, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
//  disp_draw_buf = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * screenWidth * 20, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  disp_draw_buf1 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * screenWidth * 20, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  disp_draw_buf2 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * screenWidth * 20, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
//  if (!disp_draw_buf) {
//    Serial.println("LVGL disp_draw_buf allocate failed!");
  if (!disp_draw_buf1) {
    Serial.println("LVGL disp_draw_buf1 allocate failed!");
  } else if (!disp_draw_buf2) {
    Serial.println("LVGL disp_draw_buf2 allocate failed!");
  } else {
    //lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * screenHeight);
    //lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * 20);
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf1, disp_draw_buf2, screenWidth * 20);

    /*Initialize the display*/
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*Initialize the input device driver*/
//    lv_indev_drv_init( &indev_drv );
//    indev_drv.type = LV_INDEV_TYPE_POINTER;
//    indev_drv.read_cb = my_touchpad_read;
//    lv_indev_drv_register( &indev_drv );

    /*Initialize the (LittleFS) File System device driver*/
    lv_fs_drv_init(&fs_drv);                  /*Basic initialization*/

    fs_drv.letter = 'F';                      /*An uppercase letter to identify the drive */
    //fs_drv.cache_size = LV_FS_FATFS_CACHE_SIZE;
    fs_drv.open_cb = fs_open;                 /*Callback to open a file */
    fs_drv.close_cb = fs_close;               /*Callback to close a file */
    fs_drv.read_cb = fs_read;                 /*Callback to close a file */
    fs_drv.write_cb = fs_write;               /*Callback to write a file */
    fs_drv.seek_cb = fs_seek;                 /*Callback to seek in a file (Move cursor) */
    fs_drv.tell_cb = NULL;                    /*Callback to tell the cursor position  */
    fs_drv.dir_open_cb = NULL;                /*Callback to open directory to read its content */
    fs_drv.dir_read_cb = NULL;                /*Callback to read a directory's content */
    fs_drv.dir_close_cb = NULL;               /*Callback to close a directory */

    lv_fs_drv_register(&fs_drv);              /*Finally register the drive*/

    //lv_ui_init();
  }
}

void setBacklight(int bn){
//  gfx.setBrightness(bn);
}

int getBacklight(){
  return 0;
//  return gfx.getBrightness();
}
