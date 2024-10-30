/* ************************************************************************* *
 * @file pwm_buzzer.h
 * ************************************************************************* */
#ifndef PWM_BUZZER_H
#define PWM_BUZZER_H

/*********************
 *      INCLUDES
 *********************/
#include <Arduino.h>
#include <board_config.h>

#include "FS.h"
#include <ffat.h>

/*********************
 *      DEFINES
 *********************/
#define PWM_PIN        PWM_LEFT_CHANNEL_GPIO
#define PWM_CHANNEL    0
#define PWM_RESOLUTION 8
#define BIT_RATE       2730 //Hz CBT-09427-SMT-TR

//uint32_t sample_rate=200;
//uint8_t duty_cycle = 50;
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void pwm_buzzer_init(void);
void pwm_buzzer_deinit(void);
void buzzer_on(uint8_t _duty_cycle, uint16_t _delay);
//void buzzer_on(uint8_t _duty_cycle);
void buzzer_off(void);

//void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
/**********************
 *      MACROS
 **********************/
#define beep()  buzzer_on(200, 200)
#endif /*PWM_BUZZER_H*/