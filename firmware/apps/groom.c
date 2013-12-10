#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <pentabug/hal.h>
#include <pentabug/helper.h>
#include <pentabug/app.h>
#include <pentabug/timer.h>


static uint16_t osc[3];
static uint16_t sample;
static uint8_t note;
static uint8_t row;


enum {
    NOTE_LENGTH = 420,
    BEAT_NOTES = 48,
    DUTY = 0xf000
};
enum {
    xxx = 0,
    g_0, gs0, a_0, as0, b_0,
    c_1, cs1, d_1, ds1, e_1, f_1, fs1, g_1, gs1, a_1, as1, b_1,
    c_2, cs2, d_2, ds2, e_2, f_2, fs2, g_2, gs2, a_2, as2, b_2,
    c_3, cs3, d_3, ds3, e_3, f_3, fs3, g_3, gs3, a_3, as3, b_3,
    c_4,
};

/**
 * Transcribed from 
 *     http://www.fluegel-klavier.de/mendelssohn/hochzeitsmarsch.htm
 *
 * 1/48 Beats
 * 1th: 48×
 * 2nd: 24×
 * 4th * 1.5: 16×
 * 4th: 12×
 * 8th: 6×
 * 16th: 3×
 *
 * Remember to leave some room between successive beats of the same note.
 */
static const uint8_t patterns[][3][BEAT_NOTES] PROGMEM = {
    {
        /* 1. Zeile, 1. Takt */
        {xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,c_1,c_1,xxx,c_1,c_1,xxx,c_1,c_1,xxx},
        {xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx},
        {xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx},
    },{
        /* 1. Zeile, 2. + 3. Takt */
        {c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,c_1,c_1,xxx,c_1,c_1,xxx,c_1,c_1,xxx},
        {xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx},
        {xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx},
    },{
        /* 1. Zeile, 4. Takt */
        {c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,xxx,xxx,xxx,c_1,c_1,xxx,c_1,c_1,xxx,c_1,c_1,xxx,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,xxx,xxx,xxx,c_1,c_1,xxx,c_1,c_1,xxx,c_1,c_1,xxx},
        {e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,xxx,xxx,xxx,e_1,e_1,xxx,e_1,e_1,xxx,e_1,e_1,xxx,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,xxx,xxx,xxx,e_1,e_1,xxx,e_1,e_1,xxx,e_1,e_1,xxx},
        {xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx},
    },{
        /* 2. Zeile, 1. Takt */
        {c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,xxx,xxx,xxx,c_1,c_1,xxx,c_1,c_1,xxx,c_1,c_1,xxx,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,xxx,xxx,xxx,c_1,c_1,xxx,c_1,c_1,xxx,c_1,c_1,xxx},
        {e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,xxx,xxx,xxx,e_1,e_1,xxx,e_1,e_1,xxx,e_1,e_1,xxx,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,xxx,xxx,xxx,e_1,e_1,xxx,e_1,e_1,xxx,e_1,e_1,xxx},
        {g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,xxx,xxx,xxx,g_1,g_1,xxx,g_1,g_1,xxx,g_1,g_1,xxx,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,xxx,xxx,xxx,g_1,g_1,xxx,g_1,g_1,xxx,g_1,g_1,xxx},
    },{
        /* 2. Zeile, 2. Takt */
        {a_0,a_0,a_0,a_0,a_0,a_0,a_0,a_0,a_0,a_0,a_0,a_0,
         a_0,a_0,a_0,a_0,a_0,a_0,a_0,a_0,a_0,a_0,a_0,a_0,
         b_0,b_0,b_0,b_0,b_0,b_0,b_0,b_0,b_0,b_0,b_0,b_0,
         b_0,b_0,b_0,b_0,xxx,b_0,b_0,b_0,b_0,b_0,b_0,xxx},
        {a_1,a_1,a_1,a_1,a_1,a_1,a_1,a_1,a_1,a_1,a_1,a_1,
         a_1,a_1,a_1,a_1,a_1,a_1,a_1,a_1,a_1,a_1,a_1,a_1,
         b_1,b_1,b_1,b_1,b_1,b_1,b_1,b_1,b_1,b_1,b_1,b_1,
         b_1,b_1,b_1,b_1,xxx,b_1,b_1,b_1,b_1,b_1,b_1,xxx},
        {xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx},
    },{
        /* 2. Zeile, 2. Takt */
        {e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,
         e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,e_1,xxx,
         f_1,f_1,f_1,f_1,f_1,f_1,f_1,f_1,f_1,f_1,f_1,xxx,
         f_1,f_1,f_1,f_1,f_1,f_1,f_1,f_1,f_1,f_1,f_1,xxx},
        {e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,
         e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,xxx,
         f_2,f_2,f_2,f_2,f_2,f_2,f_2,f_2,f_2,f_2,f_2,xxx,
         f_2,f_2,f_2,f_2,f_2,f_2,f_2,f_2,f_2,f_2,f_2,xxx},
        {xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx},
    },{
        /* 2. Zeile, 3. Takt */
        {g_0,g_0,g_0,g_0,g_0,g_0,g_0,g_0,g_0,g_0,g_0,g_0,
         g_0,g_0,g_0,g_0,g_0,g_0,g_0,g_0,g_0,g_0,g_0,xxx,
         g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,xxx,
         g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,xxx},
        {g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,
         g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,g_1,xxx,
         e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,xxx,
         e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,e_2,xxx},
        {xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx},
    },{
        /* 2. Zeile, 4. Takt */
        {c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,c_1,xxx,
         c_2,c_2,c_2,c_2,c_2,xxx,e_2,e_2,e_2,e_2,e_2,xxx,
         g_2,g_2,g_2,g_2,g_2,xxx,c_3,c_3,c_3,c_3,c_3,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx},
        {c_2,c_2,c_2,c_2,c_2,c_2,c_2,c_2,c_2,c_2,c_2,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx},
        {xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,
         xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx,xxx},
    }
};


static const uint16_t freq[] PROGMEM = {
    0,
    653, 692, 733, 777, 823, 872,
    924, 979, 1037, 1099, 1164, 1233,
    1306, 1384, 1466, 1554, 1646, 1744,
    1848, 1957, 2074, 2197, 2328, 2466,
    2613, 2768, 2933, 3107, 3292, 3488,
    3695, 3915, 4148, 4394, 4656, 4933,
    5226, 5537, 5866, 6215, 6584, 6976
};


static const uint8_t order[] = {
    0, 1, 1, 2,
    3, 4, 5, 6, 7
};


static void mix_hochzeit(void) {

    if (++sample >= NOTE_LENGTH) {
        sample = 0;
        if (++note >= BEAT_NOTES) {
            note = 0;
            if (++row >= ARRAY_SIZE(order)) row = 5; // skip intro
        }
        uint8_t beat_p = note % 12;
        if (beat_p == 0)
          motor_on();
        else if (note > 2 && beat_p > 0)
          motor_off();
    }

    uint8_t	p = order[row];
    uint8_t n;
    static uint8_t prev_n0 = 0, prev_n2 = 0;


    n = pgm_read_byte(&patterns[p][0][note]);
    if (n == 0) osc[0] = 0;
    osc[0] += pgm_read_word(&freq[n]);
    if (n != prev_n0) led_set(RIGHT, (n > 0)); // blink
    prev_n0 = n;


    n = pgm_read_byte(&patterns[p][1][note]);
    if (n == 0) osc[1] = 0;
    osc[1] += pgm_read_word(&freq[n]);

    n = pgm_read_byte(&patterns[p][2][note]);
    if (n == 0) osc[2] = 0;
    osc[2] += pgm_read_word(&freq[n]);
    if (n != prev_n2) led_set(LEFT, (n > 0)); // blink
    prev_n2 = n;

    uint8_t amp = (osc[0] > DUTY) | (osc[1] > DUTY) | (osc[2] > DUTY);

    static uint8_t prev_amp = 0;
    if (amp != prev_amp) {
        buzzer_inv();
        prev_amp = amp;
    }

}

static void init_hochzeit(void) {
    sample = 0;
    note = 0;
    row = 0;
    start_timer(PRESCALE_8, 100, mix_hochzeit);
}

static void run_hochzeit(void) {}

REGISTER(run_hochzeit, init_hochzeit, NULL);
