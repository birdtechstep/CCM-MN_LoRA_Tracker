#ifndef _LV_DISPLAY_DRIVER_H
#define _LV_DISPLAY_DRIVER_H

//#include "FS.h"
//#include <ffat.h>
//struct FatFile {
//  File file;
//};

void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p );
//void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data );
void* fs_open(lv_fs_drv_t* drv, const char* path, lv_fs_mode_t mode);

lv_fs_res_t fs_close(lv_fs_drv_t* drv, void* file_p);
lv_fs_res_t fs_read(lv_fs_drv_t* drv, void* file_p, void* buf, uint32_t btr, uint32_t* br);
lv_fs_res_t fs_write(lv_fs_drv_t* drv, void* file_p, const void* buf, uint32_t btw, uint32_t* bw);
lv_fs_res_t fs_seek(lv_fs_drv_t* drv, void* file_p, uint32_t pos, lv_fs_whence_t whence);

void lv_display_driver_init();

#ifdef __cplusplus
extern "C" {
#endif
void setBacklight(int bn);
int getBacklight();
#ifdef __cplusplus
}
#endif

#endif