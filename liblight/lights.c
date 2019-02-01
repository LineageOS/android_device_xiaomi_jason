/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2014, 2017 The  Linux Foundation. All rights reserved.
 * Not a contribution
 * Copyright (C) 2015 The CyanogenMod Project
 * Copyright (C) 2017-2018 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <log/log.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <hardware/lights.h>

#ifndef DEFAULT_LOW_PERSISTENCE_MODE_BRIGHTNESS
#define DEFAULT_LOW_PERSISTENCE_MODE_BRIGHTNESS 0x80
#endif

/******************************************************************************/

static pthread_once_t g_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static struct light_state_t g_attention;
static struct light_state_t g_notification;
static struct light_state_t g_battery;
static int g_last_backlight_mode = BRIGHTNESS_MODE_USER;

char const*const LCD_FILE
        = "/sys/class/leds/lcd-backlight/brightness";

char const*const LCD_MAX_BRIGHTNESS_FILE
        = "/sys/class/leds/lcd-backlight/max_brightness";

char const*const BUTTON_1_BRIGHTNESS_FILE
        = "/sys/class/leds/button-backlight/brightness";

char const*const BUTTON_1_MAX_BRIGHTNESS_FILE
        = "/sys/class/leds/button-backlight/max_brightness";

char const*const BUTTON_2_BRIGHTNESS_FILE
        = "/sys/class/leds/button-backlight1/brightness";

char const*const BUTTON_2_MAX_BRIGHTNESS_FILE
        = "/sys/class/leds/button-backlight1/max_brightness";

char const*const WHITE_LED_FILE
        = "/sys/class/leds/white/brightness";

char const*const WHITE_LED_MAX_BRIGHTNESS_FILE
        = "/sys/class/leds/white/max_brightness";

char const*const WHITE_DUTY_PCTS_FILE
        = "/sys/class/leds/white/duty_pcts";

char const*const WHITE_START_IDX_FILE
        = "/sys/class/leds/white/start_idx";

char const*const WHITE_PAUSE_LO_FILE
        = "/sys/class/leds/white/pause_lo";

char const*const WHITE_PAUSE_HI_FILE
        = "/sys/class/leds/white/pause_hi";

char const*const WHITE_RAMP_STEP_MS_FILE
        = "/sys/class/leds/white/ramp_step_ms";

char const*const WHITE_BLINK_FILE
        = "/sys/class/leds/white/blink";

char const*const PERSISTENCE_FILE
        = "/sys/class/graphics/fb0/msm_fb_persist_mode";

#define RAMP_SIZE 8
static int BRIGHTNESS_RAMP[RAMP_SIZE]
        = { 0, 12, 25, 37, 50, 72, 85, 100 };
#define RAMP_STEP_DURATION 50

#define DEFAULT_MAX_BRIGHTNESS 255
static int button1_max_brightness;
static int button2_max_brightness;
static int panel_max_brightness;
static int led_max_brightness;

/**
 * device methods
 */

static int read_int(char const* path)
{
    int fd, len;
    int num_bytes = 10;
    char buf[11];
    int retval;

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        ALOGE("%s: failed to open %s\n", __func__, path);
        goto fail;
    }

    len = read(fd, buf, num_bytes - 1);
    if (len < 0) {
        ALOGE("%s: failed to read from %s\n", __func__, path);
        goto fail;
    }

    buf[len] = '\0';
    close(fd);

    // no endptr, decimal base
    retval = strtol(buf, NULL, 10);
    return retval == 0 ? -1 : retval;

fail:
    if (fd >= 0)
        close(fd);
    return -1;
}

static int
write_int(char const* path, int value)
{
    int fd;
    static int already_warned = 0;

    fd = open(path, O_RDWR);
    if (fd >= 0) {
        char buffer[20];
        int bytes = snprintf(buffer, sizeof(buffer), "%d\n", value);
        ssize_t amt = write(fd, buffer, (size_t)bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    } else {
        if (already_warned == 0) {
            ALOGE("write_int failed to open %s\n", path);
            already_warned = 1;
        }
        return -errno;
    }
}

static int
write_str(char const* path, char* value)
{
    int fd;
    static int already_warned = 0;

    fd = open(path, O_RDWR);
    if (fd >= 0) {
        char buffer[1024];
        int bytes = snprintf(buffer, sizeof(buffer), "%s\n", value);
        ssize_t amt = write(fd, buffer, (size_t)bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    } else {
        if (already_warned == 0) {
            ALOGE("write_int failed to open %s\n", path);
            already_warned = 1;
        }
        return -errno;
    }
}

void init_globals(void)
{
    // init the mutex
    pthread_mutex_init(&g_lock, NULL);
}

static int
is_lit(struct light_state_t const* state)
{
    return state->color & 0x00ffffff;
}

static int
rgb_to_brightness(struct light_state_t const* state)
{
    int color = state->color & 0x00ffffff;
    return ((77*((color>>16)&0x00ff))
            + (150*((color>>8)&0x00ff)) + (29*(color&0x00ff))) >> 8;
}

static int
set_light_backlight(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    int brightness = rgb_to_brightness(state);
    unsigned int lpEnabled =
        state->brightnessMode == BRIGHTNESS_MODE_LOW_PERSISTENCE;
    if(!dev) {
        return -1;
    }

    // If max panel brightness is not the default (255),
    // apply linear scaling across the accepted range.
    if (panel_max_brightness != DEFAULT_MAX_BRIGHTNESS) {
        int old_brightness = brightness;
        brightness = brightness * panel_max_brightness / DEFAULT_MAX_BRIGHTNESS;
        ALOGV("%s: scaling panel brightness %d => %d\n", __func__, old_brightness, brightness);
    }

    pthread_mutex_lock(&g_lock);
    // Toggle low persistence mode state
    if ((g_last_backlight_mode != state->brightnessMode && lpEnabled) ||
        (!lpEnabled &&
         g_last_backlight_mode == BRIGHTNESS_MODE_LOW_PERSISTENCE)) {
        if ((err = write_int(PERSISTENCE_FILE, lpEnabled)) != 0) {
            ALOGE("%s: Failed to write to %s: %s\n", __FUNCTION__,
                   PERSISTENCE_FILE, strerror(errno));
        }
        if (lpEnabled != 0) {
            brightness = DEFAULT_LOW_PERSISTENCE_MODE_BRIGHTNESS;
        }
    }

    g_last_backlight_mode = state->brightnessMode;

    if (!err) {
        err = write_int(LCD_FILE, brightness);
    }

    pthread_mutex_unlock(&g_lock);
    return err;
}

static int
set_light_buttons(struct light_device_t *dev,
        const struct light_state_t *state)
{
    int err = 0;
    int brightness = rgb_to_brightness(state);
    int button1_brightness = brightness, button2_brightness = brightness;
    if(!dev) {
        return -1;
    }

    // If button 1 max brightness is not the default (255),
    // apply linear scaling across the accepted range.
    if (button1_max_brightness != DEFAULT_MAX_BRIGHTNESS) {
        button1_brightness = brightness * button1_max_brightness / DEFAULT_MAX_BRIGHTNESS;
        ALOGV("%s: scaling button brightness %d => %d\n", __func__, brightness, button1_brightness);
    }

    // If button 2 max brightness is not the default (255),
    // apply linear scaling across the accepted range.
    if (button2_max_brightness != DEFAULT_MAX_BRIGHTNESS) {
        button2_brightness = brightness * button2_max_brightness / DEFAULT_MAX_BRIGHTNESS;
        ALOGV("%s: scaling button brightness %d => %d\n", __func__, brightness, button2_brightness);
    }

    pthread_mutex_lock(&g_lock);
    err += write_int(BUTTON_1_BRIGHTNESS_FILE, button1_brightness);
    err += write_int(BUTTON_2_BRIGHTNESS_FILE, button2_brightness);
    pthread_mutex_unlock(&g_lock);
    return err;
}

static char*
get_scaled_duty_pcts(int brightness)
{
    char *buf = malloc(5 * RAMP_SIZE * sizeof(char));
    char *pad = "";
    int i = 0;

    memset(buf, 0, 5 * RAMP_SIZE * sizeof(char));

    for (i = 0; i < RAMP_SIZE; i++) {
        char temp[5] = "";
        snprintf(temp, sizeof(temp), "%s%d", pad, (BRIGHTNESS_RAMP[i] * brightness / 255));
        strcat(buf, temp);
        pad = ",";
    }
    ALOGV("%s: brightness=%d duty=%s", __func__, brightness, buf);
    return buf;
}

static int
set_speaker_light_locked(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int white, blink;
    int onMS, offMS, stepDuration, pauseHi;
    unsigned int alpha;
    char *duty;

    if(!dev) {
        return -1;
    }

    switch (state->flashMode) {
        case LIGHT_FLASH_TIMED:
            onMS = state->flashOnMS;
            offMS = state->flashOffMS;
            break;
        case LIGHT_FLASH_NONE:
        default:
            onMS = 0;
            offMS = 0;
            break;
    }

    ALOGV("set_speaker_light_locked mode %d, onMS=%d, offMS=%d\n",
            state->flashMode, onMS, offMS);

    white = rgb_to_brightness(state);
    blink = onMS > 0 && offMS > 0;

    // Extract brightness from AARRGGBB
    alpha = (state->color >> 24) & 0xff;

    // Scale brightness if a brightness has been applied by the user
    if (alpha != 0xff) {
        white = (white * alpha) / 0xff;
    }

    // disable all blinking to start
    write_int(WHITE_BLINK_FILE, 0);

    if (blink) {
        stepDuration = RAMP_STEP_DURATION;
        pauseHi = onMS - (stepDuration * RAMP_SIZE * 2);
        if (stepDuration * RAMP_SIZE * 2 > onMS) {
            stepDuration = onMS / (RAMP_SIZE * 2);
            pauseHi = 0;
        }

        // white
        write_int(WHITE_START_IDX_FILE, 0);
        duty = get_scaled_duty_pcts(white);    
        write_str(WHITE_DUTY_PCTS_FILE, duty);
        write_int(WHITE_PAUSE_LO_FILE, offMS);
        // The led driver is configured to ramp up then ramp
        // down the lut. This effectively doubles the ramp duration.
        write_int(WHITE_PAUSE_HI_FILE, pauseHi);
        write_int(WHITE_RAMP_STEP_MS_FILE, stepDuration);
        free(duty);

        // start the party
        write_int(WHITE_BLINK_FILE, 1);

    } else {
        if (white == 0) {
            write_int(WHITE_BLINK_FILE, 0);
        }

        // If max led brightness is not the default (255),
        // apply linear scaling across the accepted range.
        if (led_max_brightness != DEFAULT_MAX_BRIGHTNESS) {
            int old_brightness = white;
            white = white * led_max_brightness / DEFAULT_MAX_BRIGHTNESS;
            ALOGV("%s: scaling led brightness %d => %d\n", __func__, old_brightness, white);
        }

        write_int(WHITE_LED_FILE, white);
    }


    return 0;
}

static void
handle_speaker_light_locked(struct light_device_t* dev)
{
    if (is_lit(&g_attention)) {
        set_speaker_light_locked(dev, &g_attention);
    } else if (is_lit(&g_notification)) {
        set_speaker_light_locked(dev, &g_notification);
    } else {
        set_speaker_light_locked(dev, &g_battery);
    }
}

static int
set_light_battery(struct light_device_t* dev,
        struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);
    g_battery = *state;
    handle_speaker_light_locked(dev);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int
set_light_notifications(struct light_device_t* dev,
        struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);
    g_notification = *state;
    handle_speaker_light_locked(dev);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int
set_light_attention(struct light_device_t* dev,
        struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);
    g_attention = *state;
    handle_speaker_light_locked(dev);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

/** Close the lights device */
static int
close_lights(struct light_device_t *dev)
{
    if (dev) {
        free(dev);
    }
    return 0;
}


/******************************************************************************/

/**
 * module methods
 */

/** Open a new instance of a lights device using name */
static int open_lights(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    int (*set_light)(struct light_device_t* dev,
            struct light_state_t const* state);

    if (0 == strcmp(LIGHT_ID_BACKLIGHT, name))
        set_light = set_light_backlight;
    else if (0 == strcmp(LIGHT_ID_BUTTONS, name))
        set_light = set_light_buttons;
    else if (0 == strcmp(LIGHT_ID_BATTERY, name))
        set_light = set_light_battery;
    else if (0 == strcmp(LIGHT_ID_NOTIFICATIONS, name))
        set_light = set_light_notifications;
    else if (0 == strcmp(LIGHT_ID_ATTENTION, name))
        set_light = set_light_attention;
    else
        return -EINVAL;

    button1_max_brightness = read_int(BUTTON_1_MAX_BRIGHTNESS_FILE);
    if (button1_max_brightness < 0) {
        ALOGE("%s: failed to read button 1 max brightness, fallback to 255!\n", __func__);
        button1_max_brightness = DEFAULT_MAX_BRIGHTNESS;
    }

    button2_max_brightness = read_int(BUTTON_2_MAX_BRIGHTNESS_FILE);
    if (button2_max_brightness < 0) {
        ALOGE("%s: failed to read button 2 max brightness, fallback to 255!\n", __func__);
        button2_max_brightness = DEFAULT_MAX_BRIGHTNESS;
    }

    panel_max_brightness = read_int(LCD_MAX_BRIGHTNESS_FILE);
    if (panel_max_brightness < 0) {
        ALOGE("%s: failed to read panel max brightness, fallback to 255!\n", __func__);
        panel_max_brightness = DEFAULT_MAX_BRIGHTNESS;
    }

    led_max_brightness = read_int(WHITE_LED_MAX_BRIGHTNESS_FILE);
    if (led_max_brightness < 0) {
        ALOGE("%s: failed to read led max brightness, fallback to 255!\n", __func__);
        led_max_brightness = DEFAULT_MAX_BRIGHTNESS;
    }

    pthread_once(&g_init, init_globals);

    struct light_device_t *dev = malloc(sizeof(struct light_device_t));

    if(!dev)
        return -ENOMEM;

    memset(dev, 0, sizeof(*dev));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = LIGHTS_DEVICE_API_VERSION_2_0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int (*)(struct hw_device_t*))close_lights;
    dev->set_light = set_light;

    *device = (struct hw_device_t*)dev;
    return 0;
}

static struct hw_module_methods_t lights_module_methods = {
    .open =  open_lights,
};

/*
 * The lights Module
 */
struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "Jason Lights Module",
    .author = "The LineageOS Project",
    .methods = &lights_module_methods,
};
