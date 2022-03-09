/*
 * File:   TestHarnessMain.c
 * Author: achemish
 *
 * Created on November 16, 2021, 5:24 PM
 */


#include "xc.h"
#include <BOARD.h>
#include <stdio.h>
#include "robot.h"
#define FRONT_LEFT_BMP_MASK 0x8
#define FRONT_RIGHT_BMP_MASK 0x4
#define BACK_LEFT_BMP_MASK 0x2
#define BACK_RIGHT_BMP_MASK 0x1


//Test Harness Helper Function Declarations
int beacon_detector_test();
int trackwire_detector_test();
int tape_sensors_test();
int motors_test();
int encoders_test();

int main(void) {
    //init
    BOARD_Init();
    Robot_Init();

    //poll bumpers, if bumpers pressed run corresponding test
    uint8_t Current_Bumper_States;

    while (1) {
        Current_Bumper_States = Robot_ReadBumpers();

        //Front Left Bumper = Motors Test
        if (Current_Bumper_States & FRONT_LEFT_BMP_MASK) {
            Robot_LeftMtrSpeed(100);
            Robot_RightMtrSpeed(100);
            delay(5000000);
            Robot_LeftMtrSpeed(0);
            Robot_RightMtrSpeed(0);

            //encoders_test();
            //Front Right Bumper = Solenoid Test
        } else if (Current_Bumper_States & FRONT_RIGHT_BMP_MASK) {
            Robot_LeftMtrSpeed(100);
            Robot_RightMtrSpeed(100);
            delay(2500000);
            Robot_LeftMtrSpeed(-100);
            Robot_RightMtrSpeed(-100);
            delay(2500000);
            Robot_LeftMtrSpeed(0);
            Robot_RightMtrSpeed(0);
        } else if (Current_Bumper_States & BACK_LEFT_BMP_MASK) {
            Robot_LeftMtrSpeed(100);
            Robot_RightMtrSpeed(-100);
            delay(1000000);
Robot_LeftMtrSpeed(-100);
            Robot_RightMtrSpeed(100);
            delay(1000000);

            Robot_LeftMtrSpeed(0);
            Robot_RightMtrSpeed(0);
            //Back Right Bumper = Battery Test
        } else if (Current_Bumper_States & BACK_RIGHT_BMP_MASK) {
            Robot_LeftMtrSpeed(100);
            Robot_RightMtrSpeed(50);
            delay(2500000);
            Robot_LeftMtrSpeed(0);
            Robot_RightMtrSpeed(0);
        }
    }
    return 0;
}


//Test Harness Helper Function Implementations

int beacon_detector_test() {
    //print out the beacon state periodically
    for (int j = 0; j < 50; j++) {
        //printf("j\n");
        for (int beacon_select = 0; beacon_select < 3; beacon_select++) { //iterate beacon select from 0 to 2 to try each beacon sensor

            BEACON_LATA = 0;
            BEACON_LATB = 0;
            BEACON_LATC = 0;

            if (0 == beacon_select) {
                BEACON_LATA = 1;
            } else if (1 == beacon_select) {
                BEACON_LATB = 1;

            } else {
                BEACON_LATC = 1;

            }
            delay(200000);

            printf("Beacon %d: %d \n", beacon_select, AD_ReadADPin(BEACON_ADC));
        }
    }
    printf("Beacon Sensor Test Complete\n");
}

int trackwire_detector_test() {
    int track_wire_select = 1;
    //print out the beacon state periodically
    for (int j = 0; j < 20; j++) {
        TRACK_WIRE_LATA = track_wire_select;
        TRACK_WIRE_LATB = ~track_wire_select;
        delay(200000);

        printf("Trackwire %d: %d \n", Robot_TrackWireDetector());
        track_wire_select = ~track_wire_select;
    }
    printf("Track Wire Sensor Test Complete\n");
}

int tape_sensors_test() {
    //print out the tape sensors state periodically
    for (int j = 0; j < 20; j++) {
        delay(200000);
        printf("Tape State: 0x%X \n", Robot_ReadTape());
    }
    printf("Tape Sensor Test Complete\n");
}

//need to add encoder functionality

int motors_test() {
    int mtr = 100;
    Robot_LeftMtrSpeed(mtr);
    Robot_RightMtrSpeed(mtr);

    //iterate through motor speeds in both forward and reverse directions
    while (mtr >= -100) {
        printf("motor percent: %d \n", mtr);

        Robot_LeftMtrSpeed(mtr);
        Robot_RightMtrSpeed(mtr);
        delay(500000);
        mtr = mtr - 20;

    }

    //stop motor
    mtr = 0;
    Robot_LeftMtrSpeed(mtr);
    Robot_RightMtrSpeed(mtr);
    return SUCCESS;
}

int encoders_test() {
    Robot_SetLeftEncTick(0);
    Robot_SetRightEncTick(0);
    //test left motor encoder
    printf("Testing Left Wheel Encoder:");
    //for 5 seconds, print out encoder values in degrees twice a second
    for (int j = 0; j < 20; j++) {
        delay(200000);
        printf("Deg: %d%%, Dist: %d mm\n", Robot_GetLeftEnc_Degrees(), Robot_GetLeftEnc_MM());
    }
    //test right motor encoder
    printf("Testing Right Wheel Encoder:");
    //for 5 seconds, print out encoder values in degrees twice a second
    for (int j = 0; j < 20; j++) {
        delay(200000);
        printf("Deg: %d%%, Dist: %d mm\n", Robot_GetRightEnc_Degrees(), Robot_GetRightEnc_MM());
    }

    return SUCCESS;
}
