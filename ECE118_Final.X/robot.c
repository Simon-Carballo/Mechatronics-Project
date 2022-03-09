/*
 * File:   robot.c
 * Author: achemish
 *
 * Created on November 16, 2021, 5:53 PM
 */


#include "xc.h"
#include "robot.h"
#include "IO_Ports.h"
#include "pwm.h"
#include "AD.h"
#include "stdio.h"
#include "ES_Framework.h"

/*** Module Variables*/
int32_t left_enc_count;
int32_t right_enc_count;


//directly based on delay macro from roach.c file given to us by instructors

int delay(int x) {
    int wait;
    for (int wait = 0; wait < x; wait++) {
        asm("nop");
    }
}

void Robot_Init(void) {
    //hw init

    //tape pin setup
    FRONT_LEFT_TAPE_TRIS = 1;
    FRONT_RIGHT_TAPE_TRIS = 1;
    FRONT_CENTER_TAPE_TRIS = 1;
    BACK_LEFT_TAPE_TRIS = 1;
    BACK_RIGHT_TAPE_TRIS = 1;
    SIDE_FRONT_TAPE_TRIS = 1;
    SIDE_BACK_TAPE_TRIS = 1;
    //bumper pin setup
    FRONT_LEFT_BUMPER_TRIS = 1;
    FRONT_RIGHT_BUMPER_TRIS = 1;
    BACK_LEFT_BUMPER_TRIS = 1;
    BACK_RIGHT_BUMPER_TRIS = 1;
    SIDE_LEFT_BUMPER_TRIS = 1;
    SIDE_RIGHT_BUMPER_TRIS = 1;

    //motor init
    PWM_Init();
    PWM_SetFrequency(1000);

    PWM_AddPins(MTR_A_ENABLE | MTR_B_ENABLE);

    //motor 1
    //init dir & enc pins
    MTR_A_IN1_TRIS = 0;
    MTR_A_IN2_TRIS = 0;
    MTR_A_ENCA_TRIS = 1;
    MTR_A_ENCB_TRIS = 1;

    //set initial direction forward
    MTR_A_IN1_LAT = 1;
    MTR_A_IN2_LAT = 0;

    //motor2:
    //init dir & enc pins
    MTR_B_IN1_TRIS = 0;
    MTR_B_IN2_TRIS = 0;
    MTR_B_ENCA_TRIS = 1;
    MTR_B_ENCB_TRIS = 1;

    //set initial direction forward
    MTR_B_IN1_LAT = 1;
    MTR_B_IN2_LAT = 0;

    //encoder setup
    left_enc_count = 0;
    right_enc_count = 0;

    //solenoid pin
    SOLENOID_TRIS = 0;
    SOLENOID_LAT = 0;

    //track wire pin
    TRACK_WIRE_TRISA = 0;
    TRACK_WIRE_TRISB = 0;
    AD_Init();
    AD_AddPins(TRACK_WIRE_ADC);

    //beacon pin
    BEACON_TRISA = 0; //todo: maybe set A to true at start. Same applies to trackwire
    BEACON_TRISB = 0;
    BEACON_TRISC = 0;
    AD_AddPins(BEACON_ADC);
}

int16_t Robot_BatteryVoltage(void) {
    return AD_ReadADPin(BAT_VOLTAGE);
}

int16_t Robot_TrackWireDetector(void) {
    return AD_ReadADPin(TRACK_WIRE_ADC);
}

int16_t Robot_BeaconDetector(void) {
    return AD_ReadADPin(BEACON_ADC);
}

uint16_t Robot_ReadTape(void) {
    //return SIDE_FRONT_TAPE_BIT;
    return ((SIDE_FRONT_TAPE_BIT << 6) | (SIDE_BACK_TAPE_BIT << 5) | (BACK_RIGHT_TAPE_BIT << 4) | (BACK_LEFT_TAPE_BIT << 3) | (FRONT_CENTER_TAPE_BIT << 2) | (FRONT_RIGHT_TAPE_BIT << 1) | FRONT_LEFT_TAPE_BIT);
}

unsigned char Robot_ReadBumpers(void) {
    return ((SIDE_FRONT_BUMPER_BIT << 5) | (SIDE_BACK_BUMPER_BIT << 4) | (BACK_RIGHT_BUMPER_BIT << 3) | (BACK_LEFT_BUMPER_BIT << 2) | (FRONT_RIGHT_BUMPER_BIT << 1) | FRONT_LEFT_BUMPER_BIT);
}

//Set left motor speed and direction
//input: -100 to 100

unsigned char Robot_LeftMtrSpeed(int mtr_speed) {
    //check input against speed bounds
    if ((mtr_speed < MIN_MTR_SPEED) || (mtr_speed > MAX_MTR_SPEED)) {
        printf("Robot_SetLeftMtrSpeed ERROR: mtr_speed of %d exceeds bounds\n", mtr_speed);
        return -1;
    }


    //set direction
    if (mtr_speed > 0) {
        MTR_A_IN1_LAT = 1;
        MTR_A_IN2_LAT = 0;
    } else {
        mtr_speed = -mtr_speed; //get positive value for mtrspeed
        MTR_A_IN1_LAT = 0;
        MTR_A_IN2_LAT = 1;
    }
    mtr_speed = mtr_speed * 95 / 100; //try to compensate for stronger left motor by reducing power by 5%

    //set PWM based on speed
    PWM_SetDutyCycle(MTR_A_ENABLE, mtr_speed * 10);

    return 1;
}

//Set right motor speed and direction
//input: -100 to 100

unsigned char Robot_RightMtrSpeed(int mtr_speed) {
    //check input against speed bounds
    if ((mtr_speed < MIN_MTR_SPEED) || (mtr_speed > MAX_MTR_SPEED)) {
        printf("Robot_SetRightMtrSpeed ERROR: mtr_speed of %d exceeds bounds\n", mtr_speed);
        return -1;
    }

    //set direction

    if (mtr_speed > 0) {
        MTR_B_IN1_LAT = 1;
        MTR_B_IN2_LAT = 0;
    } else {
        mtr_speed = -mtr_speed; //get positive value for mtrspeed
        MTR_B_IN1_LAT = 0;
        MTR_B_IN2_LAT = 1;
    }


    //set PWM based on speed
    PWM_SetDutyCycle(MTR_B_ENABLE, mtr_speed * 10);

    return 1;
}

int Robot_IncrementLeftEnc(void) {

    left_enc_count++;

    return 1;
}

int Robot_DecrementLeftEnc(void) {
    left_enc_count--;

    return 1;
}

int Robot_IncrementRightEnc(void) {

    right_enc_count++;

}

int Robot_DecrementRightEnc(void) {

    right_enc_count--;

}

int Robot_SetLeftEncTick(int32_t ticks) {
    left_enc_count = ticks;
    return 1;
}

int Robot_SetRightEncTick(int32_t ticks) {
    right_enc_count = ticks;
    return 1;
}

int16_t Robot_GetLeftEnc_Degrees(void) {
    return (int16_t) (left_enc_count * 360 / ENC_TICKS_PER_REV);
}

int16_t Robot_GetLeftEnc_MM(void) {
    return (int16_t) (left_enc_count * 2 * 3.14 * WHEEL_DIAM_MM / ENC_TICKS_PER_REV);
}

int16_t Robot_GetRightEnc_Degrees(void) {
    return (int16_t) (right_enc_count * 360 / ENC_TICKS_PER_REV);
}

int16_t Robot_GetRightEnc_MM(void) {
    return (int16_t) (right_enc_count * 2 * 3.14 * WHEEL_DIAM_MM / ENC_TICKS_PER_REV);
}

unsigned char Robot_SolenoidPopBall(void) {
    SOLENOID_LAT = 1;
    delay(500000);
    SOLENOID_LAT = 0;

    return 1;
}



