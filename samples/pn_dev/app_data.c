/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2021 rt-labs AB, Sweden.
 *
 * This software is dual-licensed under GPLv3 and a commercial
 * license. See the file LICENSE.md distributed with this software for
 * full license information.
 ********************************************************************/

#include "app_data.h"
#include "app_utils.h"
#include "app_gsdml.h"
#include "app_log.h"
#include "sampleapp_common.h"
#include "osal.h"
#include "pnal.h"
#include <pnet_api.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define APP_DATA_DEFAULT_OUTPUT_DATA 0

/* Digital submodule process data
 * The stored value is shared between all digital submodules in this example. */
static float urica_1 = 0.;
static float urica_2 = 0.;

static u_int8_t input[8] = {0};

/**
 * Set LED state.
 *
 * Compares new state with previous state, to minimize system calls.
 *
 * Uses the hardware specific app_set_led() function.
 *
 * @param led_state        In:    New LED state
 */
// static void app_handle_data_led_state (bool led_state) {
//     static bool previous_led_state = false;

//     if (led_state != previous_led_state) {
//         app_set_led (APP_DATA_LED_ID, led_state);
//     }
//     previous_led_state = led_state;
// }

uint8_t * app_data_get_input_data (
    uint16_t slot_nbr,
    uint16_t subslot_nbr,
    uint32_t submodule_id,
    uint16_t * size,
    uint8_t * iops) {
    // float inputfloat;
    // float outputfloat;
    // uint32_t hostorder_inputfloat_bytes;
    // uint32_t hostorder_outputfloat_bytes;

    if (size == NULL || iops == NULL) {
        return NULL;
    }

    if (submodule_id == APP_GSDML_SUBMOD_ID_DIGITAL_IN) {
        /* Prepare digital input data
         * Lowest 7 bits: Counter    Most significant bit: Button
         */
        // inputdata[0] = counter++;
        // if (button_pressed)
        // {
        //     inputdata[0] |= 0x80;
        // }
        // else
        // {
        //     inputdata[0] &= 0x7F;
        // }

        *size = APP_GSDML_INPUT_DATA_DIGITAL_SIZE;
        *iops = PNET_IOXS_GOOD;
        return input;
    }

    /* Automated RT Tester scenario 2 - unsupported (sub)module */
    return NULL;
}

int app_data_set_output_data (
    uint16_t slot_nbr,
    uint16_t subslot_nbr,
    uint32_t submodule_id,
    uint8_t * data,
    uint16_t size) {
    // bool led_state;

    if (data == NULL) {
        return -1;
    }

    if (submodule_id == APP_GSDML_SUBMOD_ID_DIGITAL_OUT_1) {
        if (size == APP_GSDML_OUTPUT_DATA_DIGITAL_SIZE) {
            memcpy (&urica_1, data, size);
            return 0;
        }
    }

    if (submodule_id == APP_GSDML_SUBMOD_ID_DIGITAL_OUT_2) {
        if (size == APP_GSDML_OUTPUT_DATA_DIGITAL_SIZE) {
            memcpy (&urica_2, data, size);
            return 0;
        }
    }

    return -1;
}

int app_data_set_default_outputs (void) {
    urica_1 = 0.;
    urica_2 = 0.;
    return 0;
}
