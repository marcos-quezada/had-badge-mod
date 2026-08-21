/* See apps/screensaver.h. Screensaver animations - fish, starfield, DVD
 * bounce, matrix rain. Built onto a caller-provided screen object and ticked
 * from the UI task via app_manager. */
#include "apps/screensaver.h"
#include "apps/app_iface.h"
#include "ui/theme.h"
#include "ui/colors.h"
#include "ui/layout.h"
#include <string.h>
#include "esp_random.h"

/* ------------------------------------------------------------------ */
#define FISH_COUNT    3
#define BUBBLE_MAX    6
#define GROUND_H      12    /* sandy strip at the bottom */

typedef struct {
    lv_obj_t *label;
    int x, y, dx, dy;
} fish_t;

typedef struct {
    lv_obj_t *label;
    int x, y;
    bool active;
    bool notify;    /* true = envelope bubble for unread notification */
} bubble_t;

static settings_t *s_reg;
static lv_obj_t   *s_parent;
static fish_t      s_fish[FISH_COUNT];
static bubble_t    s_bubbles[BUBBLE_MAX];
static int         s_tick;

/* ------------------------------------------------------------------ */
static int rnd_range(int lo, int hi)
{
    if (hi <= lo) return lo;
    return lo + (int)(esp_random() % (uint32_t)(hi - lo + 1));
}

static void fish_reset(fish_t *f)
{
    f->x  = rnd_range(0, SCREEN_W - 70);
    f->y  = rnd_range(8, SCREEN_H - GROUND_H - 18);
    f->dx = rnd_range(1, 3) * (rnd_range(0, 1) ? 1 : -1);
    f->dy = rnd_range(0, 1) ? 0 : (rnd_range(0, 1) ? 1 : -1);
    lv_label_set_text(f->label, f->dx > 0 ? "><(((°>" : "<°)))><");
    lv_obj_set_style_text_color(f->label, theme_tint(rnd_range(160, 255)), 0);
    lv_obj_set_pos(f->label, f->x, f->y);
}

static void fish_step(fish_t *f)
{
    f->x += f->dx;
    f->y += f->dy;
    if (f->x < 0 || f->x > SCREEN_W - 70) {
        f->dx = -f->dx;
        lv_label_set_text(f->label, f->dx > 0 ? "><(((°>" : "<°)))><");
    }
    if (f->y < 4 || f->y > SCREEN_H - GROUND_H - 16) f->dy = -f->dy;
    lv_obj_set_pos(f->label, f->x, f->y);
}

static void build_fish(void)
{
    /* Water gradient background. */
    lv_obj_t *bg = lv_obj_create(s_parent);
    lv_obj_set_size(bg, SCREEN_W, SCREEN_H);
    lv_obj_set_pos(bg, 0, 0);
    lv_obj_set_style_border_width(bg, 0, 0);
    lv_obj_set_style_bg_color(bg, theme_tint(120), 0);
    lv_obj_set_style_bg_grad_color(bg, theme_tint(30), 0);
    lv_obj_set_style_bg_grad_dir(bg, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, 0);
    lv_obj_remove_flag(bg, LV_OBJ_FLAG_SCROLLABLE);

    /* Sandy ground strip. */
    lv_obj_t *ground = lv_obj_create(s_parent);
    lv_obj_set_size(ground, SCREEN_W, GROUND_H);
    lv_obj_set_pos(ground, 0, SCREEN_H - GROUND_H);
    lv_obj_set_style_border_width(ground, 0, 0);
    lv_obj_set_style_bg_color(ground, theme_tint(40), 0);
    lv_obj_set_style_bg_opa(ground, LV_OPA_COVER, 0);
    lv_obj_remove_flag(ground, LV_OBJ_FLAG_SCROLLABLE);

    /* Fish. */
    for (int i = 0; i < FISH_COUNT; i++) {
        s_fish[i].label = lv_label_create(s_parent);
        fish_reset(&s_fish[i]);
    }

    /* Bubbles - created but hidden until activated in tick. */
    for (int i = 0; i < BUBBLE_MAX; i++) {
        s_bubbles[i].label = lv_label_create(s_parent);
        lv_label_set_text(s_bubbles[i].label, "o");
        lv_obj_set_style_text_color(s_bubbles[i].label, theme_hex(C_TEXT), 0);
        lv_obj_add_flag(s_bubbles[i].label, LV_OBJ_FLAG_HIDDEN);
        s_bubbles[i].active = false;
    }

    s_tick = 0;
}

static void tick_fish(void)
{
    s_tick++;

    /* Advance fish every tick. */
    for (int i = 0; i < FISH_COUNT; i++) fish_step(&s_fish[i]);

    /* Spawn a bubble every ~2 s (20 ticks at 100ms). */
    if (s_tick % 20 == 0) {
        for (int i = 0; i < BUBBLE_MAX; i++) {
            if (!s_bubbles[i].active) {
                s_bubbles[i].x = rnd_range(10, SCREEN_W - 10);
                s_bubbles[i].y = SCREEN_H - GROUND_H - 4;
                s_bubbles[i].active = true;
                s_bubbles[i].notify = messages_has_unread();
                lv_label_set_text(s_bubbles[i].label,
                                  s_bubbles[i].notify ? LV_SYMBOL_ENVELOPE : "o");
                lv_obj_set_style_text_color(s_bubbles[i].label,
                                  s_bubbles[i].notify ? theme_hex(C_ACCENT)
                                                      : theme_hex(C_TEXT), 0);
                lv_obj_set_pos(s_bubbles[i].label, s_bubbles[i].x, s_bubbles[i].y);
                lv_obj_remove_flag(s_bubbles[i].label, LV_OBJ_FLAG_HIDDEN);
                break;
            }
        }
    }

    /* Rise bubbles, deactivate when off-screen. */
    for (int i = 0; i < BUBBLE_MAX; i++) {
        if (!s_bubbles[i].active) continue;
        s_bubbles[i].y -= 2;
        if (s_bubbles[i].y < 0) {
            s_bubbles[i].active = false;
            lv_obj_add_flag(s_bubbles[i].label, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_set_pos(s_bubbles[i].label, s_bubbles[i].x, s_bubbles[i].y);
        }
    }
}

/* ------------------------------------------------------------------ */
void screensaver_init(settings_t *reg) { s_reg = reg; }

void screensaver_build(lv_obj_t *parent)
{
    s_parent = parent;
    char choice[16] = "off";
    if (s_reg) settings_get_str(s_reg, "screensaver", choice, sizeof choice);
    /* Other choices (starfield, dvd, matrix) added in tasks 5.5-5.7. */
    if (strcmp(choice, "fish") == 0) build_fish();
}

void screensaver_tick(void)
{
    if (!s_parent) return;
    char choice[16] = "off";
    if (s_reg) settings_get_str(s_reg, "screensaver", choice, sizeof choice);
    if (strcmp(choice, "fish") == 0) tick_fish();
}

void screensaver_destroy(void)
{
    /* LVGL deletes all children when the parent screen is deleted by
     * app_manager_screensaver_exit(); nothing to free here explicitly. */
    s_parent       = NULL;
    for (int i = 0; i < BUBBLE_MAX; i++) s_bubbles[i].active = false;
    for (int i = 0; i < FISH_COUNT; i++) s_fish[i].label = NULL;
    s_tick = 0;
}

bool screensaver_enabled(void)
{
    char choice[16] = "off";
    if (s_reg) settings_get_str(s_reg, "screensaver", choice, sizeof choice);
    return strcmp(choice, "off") != 0;
}
