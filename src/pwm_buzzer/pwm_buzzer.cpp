#include "pwm_buzzer.h"

void pwm_buzzer_init(void){
    ledcSetup(PWM_CHANNEL, BIT_RATE, PWM_RESOLUTION);
    ledcAttachPin(PWM_PIN, PWM_CHANNEL);
}

void pwm_buzzer_deinit(void){
    ledcDetachPin(PWM_PIN);
}

void buzzer_on(uint8_t _duty_cycle, uint16_t _delay){
    ledcWrite(PWM_CHANNEL, (_duty_cycle*255)/100);
    //delayMicroseconds(_delay);
    vTaskDelay(_delay / portTICK_PERIOD_MS);
    ledcWrite(PWM_CHANNEL, 0);
}

void buzzer_off(void){
    ledcWrite(PWM_CHANNEL, 0);
}