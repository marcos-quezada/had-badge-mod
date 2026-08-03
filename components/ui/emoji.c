/* See ui/emoji.h. */
#include "ui/emoji.h"
#include "lvgl.h"
#include <string.h>

typedef struct {
    const char *name;
    const char *glyph;
} emoji_entry_t;

static const emoji_entry_t s_table[] = {
    /* Noto emoji (Unicode codepoints via fallback font) */
    { "smile",      "\xF0\x9F\x98\x80"  },
    { "joy",        "\xF0\x9F\x98\x82"  },
    { "sweat",      "\xF0\x9F\x98\x85"  },
    { "wink",       "\xF0\x9F\x98\x89"  },
    { "blush",      "\xF0\x9F\x98\x8A"  },
    { "cool",       "\xF0\x9F\x98\x8E"  },
    { "sad",        "\xF0\x9F\x98\x94"  }, /* U+1F614, pensive */
    { "angry",      "\xF0\x9F\x98\xA0"  },
    { "wow",        "\xF0\x9F\x98\xAE"  },
    { "think",      "\xF0\x9F\xA4\x94"  },
    { "wave",       "\xF0\x9F\x91\x8B"  },
    { "thumbsup",   "\xF0\x9F\x91\x8D"  },
    { "thumbsdown", "\xF0\x9F\x91\x8E"  },
    { "clap",       "\xF0\x9F\x91\x8F"  },
    { "heart",      "\xE2\x9D\xA4"      },
    { "star",       "\xE2\xAD\x90"      },
    { "fire",       "\xF0\x9F\x94\xA5"  },
    { "tada",       "\xF0\x9F\x8E\x89"  },

    /* LVGL built-in symbols */
    { "bell",       LV_SYMBOL_BELL      },
    { "ok",         LV_SYMBOL_OK        },
    { "x",          LV_SYMBOL_CLOSE     },
    { "warn",       LV_SYMBOL_WARNING   },
    { "mail",       LV_SYMBOL_ENVELOPE  },
    { "call",       LV_SYMBOL_CALL      },
    { "wifi",       LV_SYMBOL_WIFI      },
    { "audio",      LV_SYMBOL_AUDIO     },
    { "eye",        LV_SYMBOL_EYE_OPEN  },
    { "edit",       LV_SYMBOL_EDIT      },
    { "trash",      LV_SYMBOL_TRASH     },
    { "bt",         LV_SYMBOL_BLUETOOTH }
};

#define TABLE_N ((int)(sizeof s_table / sizeof s_table[0]))

int emoji_subst_count(void) { return TABLE_N; }

static const char *lookup_glyph(const char *name)
{
    for (int i = 0; i < TABLE_N; i++)
        if (strcmp(s_table[i].name, name) == 0) return s_table[i].glyph;
    return NULL;
}

/* Try to match a shortcode at src[0] (must start with ':').
 * On match, copies the glyph into dst (up to dst_sz bytes) and sets
 * *src_used (bytes consumed from src) and *dst_used (bytes written to dst).
 * Returns 1 on match, 0 otherwise. */
static int try_shortcode(const char *src, char *dst, size_t dst_sz,
                         size_t *src_used, size_t *dst_used)
{
    size_t j = 1;
    while (src[j] != '\0' && src[j] != ':') j++;
    if (src[j] != ':') return 0;              /* no closing colon */

    size_t name_len = j - 1;
    if (name_len == 0 || name_len >= 32) return 0;

    char name_buf[32];
    memcpy(name_buf, src + 1, name_len);
    name_buf[name_len] = '\0';

    const char *glyph = lookup_glyph(name_buf);
    if (!glyph) return 0;

    size_t glyph_len = strlen(glyph);
    if (glyph_len >= dst_sz) return 0;        /* no room */

    memcpy(dst, glyph, glyph_len);
    *src_used = j + 1;
    *dst_used = glyph_len;
    return 1;
}

/* Returns 1 if s points to a UTF-8 skin tone modifier (U+1F3FB-U+1F3FF). */
static int is_skin_tone(const char *s)
{
    return (unsigned char)s[0] == 0xF0 &&
           (unsigned char)s[1] == 0x9F &&
           (unsigned char)s[2] == 0x8F &&
           (unsigned char)s[3] >= 0xBB &&
           (unsigned char)s[3] <= 0xBF;
}

const char *emoji_subst(const char *src, char *dst, size_t dst_sz)
{
    if (!src || !dst || dst_sz == 0) return dst;

    size_t sp = 0, dp = 0;

    while (src[sp] != '\0') {
        if (is_skin_tone(src + sp)) { sp += 4; continue; }

        /* Try shortcode substitution. */
        if (src[sp] == ':') {
            size_t src_used, dst_used;

            if (try_shortcode(src + sp, dst + dp, dst_sz - dp,
                              &src_used, &dst_used)) {
                sp += src_used;
                dp += dst_used;
                continue;
            }
        }

        /* Verbatim copy. */
        if (dp + 1 >= dst_sz) break;
        dst[dp++] = src[sp++];
    }

    dst[dp] = '\0';
    return dst;
}
