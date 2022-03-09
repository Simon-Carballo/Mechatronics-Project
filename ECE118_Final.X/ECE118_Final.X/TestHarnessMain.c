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

#define TEST_HARNESS_MODE 4 //0 is normal tests, 1 is motor tests, 2 is beacon tests

//Test Harness Helper Function Declarations
int beacon_detector_test();
int trackwire_detector_test();
int tape_sensors_test();
int motors_test();
int encoders_test();
int beacon_avg();

int main(void) {
    //init
    BOARD_Init();
    Robot_Init();

    //poll bumpers, if bumpers pressed run corresponding test
    uint8_t Current_Bumper_States;


    while (1) {
        Current_Bumper_States = Robot_ReadBumpers();


        if (TEST_HARNESS_MODE == 0) {
            //Front Left Bumper = Motors Test
            if (Current_Bumper_States & FRONT_LEFT_BMP_MASK) {
                printf("FrontLeftBumper: Running Motors/Encoders Test...\n");
                motors_test();

                //encoders_test();
                //Front Right Bumper = Solenoid Test
            } else if (Current_Bumper_States & FRONT_RIGHT_BMP_MASK) {
                printf("FrontRightBumper: Running Solenoid Test...\n");
                Robot_SolenoidPopBall();


                //Back Left Bumper = Sensor Tests
            } else if (Current_Bumper_States & BACK_LEFT_BMP_MASK) {
                printf("BackLeftBumper: Running Sensor Tests...\n");

                tape_sensors_test();
                trackwire_detector_test();
                beacon_detector_test();

                //Back Right Bumper = Battery Test
            } else if (Current_Bumper_States & BACK_RIGHT_BMP_MASK) {
                printf("BackRightBumper: Running Battery Test...\n");
                printf("Current Battery Voltage: %d \n", Robot_BatteryVoltage());
                delay(500000);
            }
        } else if (TEST_HARNESS_MODE == 1) {
            //Front Left Bumper = Motors Test
            if (Current_Bumper_States & FRONT_LEFT_BMP_MASK) {
                printf("FL");
                Robot_LeftMtrSpeed(100);
                Robot_RightMtrSpeed(100);
                delay(5000000);
                Robot_LeftMtrSpeed(0);
                Robot_RightMtrSpeed(0);
                printf("done");

                //encoders_test();
                //Front Right Bumper = Solenoid Test
            } else if (Current_Bumper_States & FRONT_RIGHT_BMP_MASK) {
                printf("FR");

                Robot_LeftMtrSpeed(100);
                Robot_RightMtrSpeed(100);
                delay(2500000);
                Robot_LeftMtrSpeed(-100);
                Robot_RightMtrSpeed(-100);
                delay(2500000);
                Robot_LeftMtrSpeed(0);
                Robot_RightMtrSpeed(0);
                printf("done");

            } else if (Current_Bumper_States & BACK_LEFT_BMP_MASK) {
                printf("BL");

                Robot_LeftMtrSpeed(100);
                Robot_RightMtrSpeed(-100);
                delay(1000000);
                Robot_LeftMtrSpeed(-100);
                Robot_RightMtrSpeed(100);
                delay(1000000);

                Robot_LeftMtrSpeed(0);
                Robot_RightMtrSpeed(0);
                printf("done");

                //Back Right Bumper = Battery Test
            } else if (Current_Bumper_States & BACK_RIGHT_BMP_MASK) {
                printf("BR");

                Robot_LeftMtrSpeed(100);
                Robot_RightMtrSpeed(50);
                delay(2500000);
                Robot_LeftMtrSpeed(0);
                Robot_RightMtrSpeed(0);
                printf("done");

            }

        } else if (TEST_HARNESS_MODE == 2) { //BEACON TEST HARNESS
            //Front Left Bumper = Motors Test
            if (Current_Bumper_States & FRONT_LEFT_BMP_MASK) {
                printf("FrontLeftBumper: Printing Beacon Values...\n");

                for (int i = 0; i < 35; i++) {
                    int sample_avg = beacon_avg();
                    printf("Avg: %d", sample_avg);
                    delay(200000);

                }
                printf("Beacon Value Test Done");

            } else if (Current_Bumper_States & FRONT_RIGHT_BMP_MASK) {
                printf("FrontRightBumper: Printing Beacon State");
                int sample_avg = 0;
                int beacon_state = 0;
                for (int i = 0; i < 35; i++) {
                    sample_avg = beacon_avg();
                    if (0 == beacon_state) { //NO DETECTION
                        if (sample_avg > 780) {
                            beacon_state = 1;
                            printf("HIGH\n     ");
                        }
                    } else {
                        if (sample_avg < 720) {
                            beacon_state = 0;
                            printf("LOW\n      ");
                        }
                    }
                    delay(200000);

                }
                printf("Beacon State Test Done");

                //Back Left Bumper = Sensor Tests
            } else if (Current_Bumper_States & BACK_LEFT_BMP_MASK) {
                printf("BackLeftBumper: Running Sensor Tests...\n");

                tape_sensors_test();
                trackwire_detector_test();
                beacon_detector_test();

                //Back Right Bumper = Battery Test
            } else if (Current_Bumper_States & BACK_RIGHT_BMP_MASK) {
                printf("BackRightBumper: Running Battery Test...\n");
                printf("Current Battery Voltage: %d \n", Robot_BatteryVoltage());
                delay(500000);
            }
        } else if (TEST_HARNESS_MODE == 3) {
            printf("Bumpers: %d     ", Robot_ReadBumpers());
            delay(300000);
        }
        else if (TEST_HARNESS_MODE == 4) {
            //Front Left Bumper = BEACON Test
            if (Current_Bumper_States & FRONT_LEFT_BMP_MASK) {
                printf("FrontLeftBumper: Beacon Test...\n");
                beacon_detector_test();

            } else if (Current_Bumper_States & FRONT_RIGHT_BMP_MASK) {
                printf("FrontRightBumper: Running Tape Sensor Test...\n");
                tape_sensors_test();

            } else if (Current_Bumper_States & BACK_LEFT_BMP_MASK) {
                printf("BackLeftBumper: Running Track Wire Test...\n");

                trackwire_detector_test();

                //Back Right Bumper = Battery Test
            } else if (Current_Bumper_States & BACK_RIGHT_BMP_MASK) {
                printf("BackRightBumper: Running Battery Test...\n");
                printf("Current Battery Voltage: %d \n", Robot_BatteryVoltage());
                delay(500000);
            }
        } 
    }

    return 0;
}

int beacon_avg() {
    //take moving avg
    int sample_count = 0;
    uint16_t sample_sum = 0;
    while (sample_count < 15) {
        //while(!AD_IsNewDataReady(); //replace this with a tiny busy delay if this does not delay enough
        sample_sum += AD_ReadADPin(BEACON_ADC);
        sample_count++;
        delay(2000);

    }
    return (int) (sample_sum / 15);
}

//Test Harness Helper Function Implementations

int beacon_detector_test() {
    //print out the beacon state periodically
    for (int j = 0; j < 10; j++) {
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

            printf("B%d: %d   \r\n", beacon_select, AD_ReadADPin(BEACON_ADC));
        }
    }
    printf("Beacon Sensor Test Complete\n");
}

int trackwire_detector_test() {
    int track_wire_select = 1;
    //print out the beacon state periodically
    for (int j = 0; j < 20; j++) {
        TRACK_WIRE_LATA = track_wire_select;
        TRACK_WIRE_LATB = !track_wire_select;
        delay(200000);

        printf("T%d: %d   \r\n", track_wire_select, Robot_TrackWireDetector());
        track_wire_select = !track_wire_select;
    }
    printf("Track Wire Sensor Test Complete\n");
}

int tape_sensors_test() {
    //print out the tape sensors state periodically
    for (int j = 0; j < 20; j++) {
        delay(200000);
        printf("State: 0x%X   \n", Robot_ReadTape());
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
