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
#include "app_gsdml.h"
#include "pnal.h"
#include "wiringPi.h"

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

const int URICA_1_ID = 1;
const int URICA_1_DATA_PIN = 1;
const int URICA_1_CLOCK_PIN = 2;
const int URICA_1_REQUEST_PIN = 3;
const int URICA_1_SET_ZERO_PIN = 4;

const int URICA_2_ID = 2;
const int URICA_2_DATA_PIN = 5;
const int URICA_2_CLOCK_PIN = 6;
const int URICA_2_REQUEST_PIN = 7;
const int URICA_2_SET_ZERO_PIN = 8;

/* Digital submodule process data
 * The stored value is shared between all digital submodules in this example. */
static float urica_1 = 0.;
static float urica_2 = 0.;

static pthread_mutex_t urica_1_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t urica_2_mutex = PTHREAD_MUTEX_INITIALIZER;

static float urica_1_buf = 0.;
static float urica_2_buf = 0.;

static u_int8_t input[8] = {0};

float * app_data_get_input_data (
    uint16_t slot_nbr,
    uint16_t subslot_nbr,
    uint32_t submodule_id,
    uint16_t * size,
    uint8_t * iops) {

    if (size == NULL || iops == NULL) {
        return NULL;
    }

    if (submodule_id == APP_GSDML_SUBMOD_ID_DIGITAL_IN_1) {
        *size = APP_GSDML_INPUT_DATA_DIGITAL_SIZE;
        *iops = PNET_IOXS_GOOD;

        if (pthread_mutex_trylock (&urica_1_mutex) == 0) {
            memcpy (&urica_1, &urica_1_buf, sizeof urica_1);
            pthread_mutex_unlock (&urica_1_mutex);
        }

        return &urica_1_buf;
    }

    if (submodule_id == APP_GSDML_SUBMOD_ID_DIGITAL_IN_2) {
        *size = APP_GSDML_INPUT_DATA_DIGITAL_SIZE;
        *iops = PNET_IOXS_GOOD;

        if (pthread_mutex_trylock (&urica_2_mutex) == 0) {
            memcpy (&urica_2, &urica_2_buf, sizeof urica_2);
            pthread_mutex_unlock (&urica_2_mutex);
        }

        return &urica_2_buf;
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

    if (submodule_id == APP_GSDML_SUBMOD_ID_DIGITAL_OUT) {
        if (size == APP_GSDML_OUTPUT_DATA_DIGITAL_SIZE) {
            memcpy (input, data, size);
            return 0;
        }
    }

    return -1;
}

int app_data_set_default_outputs (void) {
    memset (input, 0, 8);

    return 0;
}

void * urica_loop() {
    for (;;) {
        printf ("Meri urice\n");

        float urica_1_meritev = meri_urico (URICA_1_ID);
        float urica_2_meritev = meri_urico (URICA_2_ID);

        pthread_mutex_lock (&urica_1_mutex);
        urica_1 = urica_1_meritev;
        pthread_mutex_unlock (&urica_1_mutex);

        pthread_mutex_lock (&urica_2_mutex);
        urica_2 = urica_2_meritev;
        pthread_mutex_unlock (&urica_2_mutex);

        os_usleep (100000);
    }
}

void setup_urica() {
    wiringPiSetupGpio();

    pinMode (URICA_1_DATA_PIN, INPUT);
    pinMode (URICA_1_CLOCK_PIN, INPUT);
    pinMode (URICA_1_REQUEST_PIN, OUTPUT);
    pinMode (URICA_1_SET_ZERO_PIN, OUTPUT);

    pullUpDnControl (URICA_1_DATA_PIN, PUD_UP);
    pullUpDnControl (URICA_1_CLOCK_PIN, PUD_UP);

    pinMode (URICA_2_DATA_PIN, INPUT);
    pinMode (URICA_2_CLOCK_PIN, INPUT);
    pinMode (URICA_2_REQUEST_PIN, OUTPUT);
    pinMode (URICA_2_SET_ZERO_PIN, OUTPUT);

    pullUpDnControl (URICA_2_DATA_PIN, PUD_UP);
    pullUpDnControl (URICA_2_CLOCK_PIN, PUD_UP);

    pthread_t urice;
    pthread_create (&urice, NULL, &urica_loop, NULL);
}

float meri_urico (int id) {
    int data;
    int clock;
    int request;
    uint8_t data_buf[13] = {0};
    float result;

    if (id == URICA_1_ID) {
        data = URICA_1_DATA_PIN;
        clock = URICA_1_CLOCK_PIN;
        request = URICA_1_REQUEST_PIN;
    } else if (id == URICA_2_ID) {
        data = URICA_2_DATA_PIN;
        clock = URICA_2_CLOCK_PIN;
        request = URICA_2_REQUEST_PIN;
    } else {
        return 0.;
    }

    digitalWrite (request, HIGH);

    int i;
    int j;
    for (i = 0; i < 13; i++) {
        u_int8_t nibble = 0;

        for (j = 0; j < 4; j++) {
            while (digitalRead (clock) == LOW) {
            }
            while (digitalRead (clock) == HIGH) {
            }

            uint8_t bit = digitalRead (data) & 0b00000001;

            nibble <<= 1;
            nibble = nibble | bit;
        }

        if (i == 0) {
            digitalWrite (request, LOW);
        }

        data_buf[i] = nibble;
    }

    uint8_t decimal = data_buf[11];
    uint16_t value = 0;

    for (i = 5; i < 11; i++) {
        value *= 10;
        value += (uint16_t)data_buf[i];
    }

    result = (float)value;
    result /= powf (result, decimal);

    if (data_buf[4] == 0x8) {
        result *= -1.;
    }

    return result;
}

void check_plc_output() {
}
