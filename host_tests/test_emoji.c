#include "test_util.h"
#include "ui/emoji.h"

void run_emoji(void)
{
    char buf[512];

    SUITE("emoji/known-shortcode");
    emoji_subst(":smile:", buf, sizeof buf);
    CHECK_STR(buf, "\xF0\x9F\x98\x80");

    SUITE("emoji/unknown-shortcode");
    emoji_subst(":notanemoji:", buf, sizeof buf);
    CHECK_STR(buf, ":notanemoji:");

    SUITE("emoji/empty-string");
    emoji_subst("", buf, sizeof buf);
    CHECK_STR(buf, "");

    SUITE("emoji/adjacent-shortcodes");
    emoji_subst(":smile::wave:", buf, sizeof buf);
    CHECK_STR(buf, "\xF0\x9F\x98\x80\xF0\x9F\x91\x8B");

    SUITE("emoji/plain-text-unchanged");
    emoji_subst("hello world", buf, sizeof buf);
    CHECK_STR(buf, "hello world");

    SUITE("emoji/mixed-text-and-shortcode");
    emoji_subst("hi :wave: there", buf, sizeof buf);
    CHECK_STR(buf, "hi \xF0\x9F\x91\x8B there");

    SUITE("emoji/skin-tone-stripped");
    /* thumbsup + medium skin tone modifier */
    emoji_subst("\xF0\x9F\x91\x8D\xF0\x9F\x8F\xBD", buf, sizeof buf);
    CHECK_STR(buf, "\xF0\x9F\x91\x8D");

    SUITE("emoji/table-count");
    CHECK(emoji_subst_count() >= 30);

    SUITE("emoji/buffer-truncation");
    char small[5];
    emoji_subst(":smile:", small, sizeof small);
    CHECK(small[4] == '\0');    /* always null-terminated */
}
