/* Screensaver animations. screensaver_init() must be called once at boot. */
#ifndef APPS_SCREENSAVER_H
#define APPS_SCREENSAVER_H

#include "lvgl.h"
#include "core/settings.h"

void screensaver_init(settings_t *reg);
void screensaver_build(lv_obj_t *parent);
void screensaver_tick(void);
void screensaver_destroy(void);

#endif /* APPS_SCREENSAVER_H */
