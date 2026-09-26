/* Compile-time feature flags and buffer sizes. */
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define FW_NAME     "Communicator-C"
/* Version is no longer a hardcoded constant here -- it comes from
 * ESP-IDF's own git-derived app descriptor (esp_app_get_description(),
 * populated at build time via `git describe --tags`), so it can never go
 * stale. See components/apps/launcher.c and main/app_main.c. */

/* LoRa / Meshtastic defaults (overridable at runtime via settings/NVS). */
#define DEFAULT_REGION        "EU_868"
#define DEFAULT_PRESET        "LongFast"
#define DEFAULT_CHANNEL_NAME  "LongFast"
#define DEFAULT_PSK_B64       "AQ=="
#define DEFAULT_HOP_LIMIT     3

/* Message history kept in RAM for the chat app. */
#define MSG_HISTORY_MAX       64
#define MSG_TEXT_MAX          234
#define APP_SLOTS_MAX         20    /* built-in + user app slots */

/* Backlight policy (seconds). 0 disables a stage. */
#define BL_DIM_TIMEOUT_S      60
#define BL_OFF_TIMEOUT_S      0
#define BL_DUTY_BRIGHT        700
#define BL_DUTY_DIM           120

/* Task stack sizes (words). */
#define STACK_UI              8192
#define STACK_RADIO           4096
#define STACK_INPUT           3072
#define STACK_HOUSEKEEPING    4096

#endif /* APP_CONFIG_H */
