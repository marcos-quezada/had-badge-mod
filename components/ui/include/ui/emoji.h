/* Shortcode to glyph substitution for message rendering. */
#ifndef UI_EMOJI_H
#define UI_EMOJI_H

#include <stddef.h>

/* Scan src for :name: shortcodes and write the substituted text into dst. 
 * Returns dst. Never allocates heap. */
const char *emoji_subst(const char *src, char *dst, size_t dst_sz);

/* Number of entries in the shortcode table. */
int emoji_subst_count(void);

#endif /* UI_EMOJI_H */
