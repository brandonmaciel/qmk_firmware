#include QMK_KEYBOARD_H

#include "raw_hid.h"
// #include "music_reactive.h"

#define RAW_EPSIZE 32

enum alt_keycodes {
    U_T_AUTO = SAFE_RANGE, //USB Extra Port Toggle Auto Detect / Always Active
    U_T_AGCR,              //USB Toggle Automatic GCR control
    DBG_TOG,               //DEBUG Toggle On / Off
    DBG_MTRX,              //DEBUG Toggle Matrix Prints
    DBG_KBD,               //DEBUG Toggle Keyboard Prints
    DBG_MOU,               //DEBUG Toggle Mouse Prints
    MD_BOOT,               //Restart into bootloader after hold timeout

    /* Brandon */
    LVE_T0G,               // Toggle live audio visualizer
    DBG_LM                 // debugger - test any functionality
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_DEL,  \
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_HOME, \
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,  KC_PGUP, \
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,   KC_PGDN, \
        KC_LCTL, KC_LALT, KC_LGUI,                            KC_SPC,                             KC_RGUI, MO(1),   KC_LEFT, KC_DOWN, KC_RGHT  \
    ),
    [1] = LAYOUT(
        KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______, KC_MUTE, \
        DBG_LM, RGB_SPD, RGB_VAI, RGB_SPI, RGB_HUI, RGB_SAI, _______, U_T_AUTO,U_T_AGCR,_______, KC_PSCR, KC_SLCK, KC_PAUS, _______, KC_MPLY, \
        _______, RGB_RMOD,RGB_VAD, RGB_MOD, RGB_HUD, RGB_SAD, _______, _______, _______, _______, _______, _______,          _______, KC_VOLU, \
        LVE_T0G, RGB_TOG, _______, _______, _______, MD_BOOT, NK_TOGG, DBG_TOG, KC_MPRV, KC_MNXT, _______, _______,          KC_PGUP, KC_VOLD, \
        _______, _______, _______,                            _______,                            _______, _______, KC_HOME, KC_PGDN, KC_END  \
    ),
    /*
    [X] = LAYOUT(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______, \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______, \
        _______, _______, _______,                            _______,                            _______, _______, _______, _______, _______  \
    ),
    */
};

#define MODS_SHIFT  (get_mods() & MOD_BIT(KC_LSHIFT) || get_mods() & MOD_BIT(KC_RSHIFT))
#define MODS_CTRL  (get_mods() & MOD_BIT(KC_LCTL) || get_mods() & MOD_BIT(KC_RCTRL))
#define MODS_ALT  (get_mods() & MOD_BIT(KC_LALT) || get_mods() & MOD_BIT(KC_RALT))


/* raw hid */

/* https://docs.qmk.fm/#/feature_rawhid */

/*
    Vendor ID:  0x04D8
    Product ID: 0xEED3

    By default, Usage Page is 0xFF60 and Usage is 0x61.
    Usage Page: 0xFF60
    Usage:      0x61
*/

/*  */

uint8_t is_hid_connected = 45; // 45 = -
uint8_t data_buffer[32] =  {72, 69, 76, 76, 79}; // Buffer used to store sent info from host computer
uint8_t live_music = 0;

uint8_t awake[RAW_EPSIZE] = {43, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t sleep[RAW_EPSIZE] = {45, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void send_message(uint8_t test) {
    data_buffer[5] = test;
  raw_hid_send(data_buffer, sizeof(data_buffer));
}

void send_message_test(void) {
  // Send the current info screen index to the connected node script so that it can pass back the new data
  uint8_t send_data[32] = {72, 69, 76, 76, 79};
//   send_data[0] = 1; // Add one so that we can distinguish it from a null byte
  raw_hid_send(send_data, sizeof(send_data));
}

void set_rgb_range(uint8_t r, uint8_t g, uint8_t b, int start, int end) {

    for(int i = start; i <= end; i++) {
        rgb_matrix_set_color(i, r, g, b);
    }
}

/* for data to be received incoming data must be of length = 32 */
void raw_hid_receive(uint8_t *data, uint8_t length) {
    rgb_matrix_set_flags(LED_FLAG_NONE);
    live_music = 1;

    // memcpy(data_buffer, data, length * sizeof(data[0]));

    switch (data[0]) {
        case 1:
            /* ! means it is awake */
            rgb_matrix_set_flags(LED_FLAG_NONE);
            // rgb_matrix_set_color_all(0, 255, 0);
            uint8_t test_color = 128;
            for(uint8_t i = 0; i <= test_color; i++) {
                set_rgb_range(i, 0, i, 67, 105);
            }
            for(uint8_t i = test_color; i >= 0; i--) {
                set_rgb_range(i, 0, i, 67, 105);
            }
            break;
        case 2:
            // uint8_t test_color = 128;
            for(uint8_t i = 0; i <= 128; i++) {
                set_rgb_range(i, 0, i, 67, 104);
            }
            break;
        case 3:
            set_rgb_range(0, 0, 0, 67, 104);
            break;
        case 4:
            set_rgb_range(128, 0, 128, 67, 104);
            break;
        default:
            // rgb_matrix_set_color_all(0, 0, 255);
            break;
    }

    // raw_hid_send(data, length); // use this to debug (i.e send back the received data to confirm data is correct)
}

/* raw hid */

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    static uint32_t key_timer;

    switch (keycode) {
        case U_T_AUTO:
            if (record->event.pressed && MODS_SHIFT && MODS_CTRL) {
                TOGGLE_FLAG_AND_PRINT(usb_extra_manual, "USB extra port manual mode");
            }
            return false;
        case U_T_AGCR:
            if (record->event.pressed && MODS_SHIFT && MODS_CTRL) {
                TOGGLE_FLAG_AND_PRINT(usb_gcr_auto, "USB GCR auto mode");
            }
            return false;
        case DBG_TOG:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_enable, "Debug mode");
            }
            return false;
        case DBG_MTRX:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_matrix, "Debug matrix");
            }
            return false;
        case DBG_KBD:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_keyboard, "Debug keyboard");
            }
            return false;
        case DBG_MOU:
            if (record->event.pressed) {
                TOGGLE_FLAG_AND_PRINT(debug_mouse, "Debug mouse");
            }
            return false;
        case MD_BOOT:
            if (record->event.pressed) {
                key_timer = timer_read32();
            } else {
                if (timer_elapsed32(key_timer) >= 500) {
                    reset_keyboard();
                }
            }
            return false;
        case RGB_TOG:
            if (record->event.pressed) {
              switch (rgb_matrix_get_flags()) {
                case LED_FLAG_ALL: {
                    rgb_matrix_set_flags(LED_FLAG_KEYLIGHT | LED_FLAG_MODIFIER | LED_FLAG_INDICATOR | LED_FLAG_CUSTOM_UNDERGLOW);
                    rgb_matrix_set_color_all(0, 0, 0);
                  }
                  break;
                case (LED_FLAG_KEYLIGHT | LED_FLAG_MODIFIER | LED_FLAG_INDICATOR | LED_FLAG_CUSTOM_UNDERGLOW): {
                    rgb_matrix_set_flags(LED_FLAG_UNDERGLOW);
                    rgb_matrix_set_color_all(0, 0, 0);
                  }
                  break;
                case (LED_FLAG_UNDERGLOW): {
                    rgb_matrix_set_flags(LED_FLAG_NONE);
                    rgb_matrix_disable_noeeprom();
                  }
                  break;
                default: {
                    rgb_matrix_set_flags(LED_FLAG_ALL);
                    rgb_matrix_enable_noeeprom();
                  }
                  break;
              }
            }
            return false;
        case LVE_T0G:
            if (record->event.pressed) {
                // live to idle
                if(live_music == 1){
                    raw_hid_send(sleep, sizeof(sleep)); // send message for host to sleep
                    rgb_matrix_set_flags(LED_FLAG_KEYLIGHT | LED_FLAG_MODIFIER | LED_FLAG_INDICATOR | LED_FLAG_CUSTOM_UNDERGLOW); // set keylight back to (todo: return to previous) light mode
                    // rgb_matrix_set_color_all(0, 0, 0);
                    live_music = false;
                    return false;
                }
                // idle to live
                raw_hid_send(awake, sizeof(awake)); // awaken host
                rgb_matrix_set_flags(LED_FLAG_NONE);
            }
            return false;
        case DBG_LM:
            if (record->event.pressed) {
                rgb_matrix_set_flags(LED_FLAG_NONE);
                rgb_matrix_set_color_all(0, 0, 255);
            }
            return false;
        default:
            return true; //Process all other keycodes normally
    }
}
