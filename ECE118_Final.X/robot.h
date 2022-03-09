/* 
 * File:   robot.h
 * Author: achemish
 *
 * Created on November 16, 2021, 5:51 PM
 */
#include "xc.h"
#include "IO_Ports.h"
#include "ES_Configure.h"
#include "ad.h"
#include "pwm.h"
#include "ES_Events.h"
#ifndef ROBOT_H
#define	ROBOT_H

/*** MACROS ***/

//PIN DEFINES - TAPE
#define FRONT_LEFT_TAPE_TRIS PORTX03_TRIS
#define FRONT_RIGHT_TAPE_TRIS PORTX04_TRIS
#define FRONT_CENTER_TAPE_TRIS PORTX05_TRIS
#define BACK_LEFT_TAPE_TRIS PORTX09_TRIS
#define BACK_RIGHT_TAPE_TRIS PORTX06_TRIS
#define SIDE_FRONT_TAPE_TRIS PORTX10_TRIS

#define SIDE_BACK_TAPE_TRIS PORTX08_TRIS

#define FRONT_LEFT_TAPE_BIT PORTX03_BIT
#define FRONT_RIGHT_TAPE_BIT PORTX04_BIT
#define FRONT_CENTER_TAPE_BIT PORTX05_BIT
#define BACK_LEFT_TAPE_BIT PORTX09_BIT
#define BACK_RIGHT_TAPE_BIT PORTX06_BIT
#define SIDE_FRONT_TAPE_BIT PORTX10_BIT
#define SIDE_BACK_TAPE_BIT PORTX08_BIT

//PIN DEFINES - BUMPERS  
#define FRONT_LEFT_BUMPER_TRIS PORTZ03_TRIS
#define FRONT_RIGHT_BUMPER_TRIS PORTZ04_TRIS
#define BACK_LEFT_BUMPER_TRIS PORTZ05_TRIS
#define BACK_RIGHT_BUMPER_TRIS PORTZ06_TRIS
#define SIDE_LEFT_BUMPER_TRIS PORTZ07_TRIS
#define SIDE_RIGHT_BUMPER_TRIS PORTZ08_TRIS

#define FRONT_LEFT_BUMPER_BIT PORTZ03_BIT
#define FRONT_RIGHT_BUMPER_BIT PORTZ04_BIT
#define BACK_LEFT_BUMPER_BIT PORTZ05_BIT
#define BACK_RIGHT_BUMPER_BIT PORTZ06_BIT
#define SIDE_BACK_BUMPER_BIT PORTZ07_BIT
#define SIDE_FRONT_BUMPER_BIT PORTZ08_BIT

////h-bridge
//#define MTR_A_ENABLE        PWM_PORTY04
//#define MTR_A_IN1_TRIS      PORTY05_TRIS
//#define MTR_A_IN1_LAT       PORTY05_LAT
//#define MTR_A_IN2_TRIS      PORTY06_TRIS
//#define MTR_A_IN2_LAT       PORTY06_LAT
//#define MTR_A_ENCA_TRIS      PORTY07_TRIS
//#define MTR_A_ENCA_BIT       PORTY07_BIT
//#define MTR_A_ENCB_TRIS      PORTY08_TRIS
//#define MTR_A_ENCB_BIT       PORTY08_BIT
//
//#define MTR_B_ENABLE        PWM_PORTY10
//#define MTR_B_IN1_TRIS      PORTY11_TRIS
//#define MTR_B_IN1_LAT       PORTY11_LAT
//#define MTR_B_IN2_TRIS      PORTY12_TRIS
//#define MTR_B_IN2_LAT       PORTY12_LAT
//#define MTR_B_ENCA_TRIS      PORTY09_TRIS
//#define MTR_B_ENCA_BIT       PORTY09_BIT
//#define MTR_B_ENCB_TRIS      PORTY03_TRIS
//#define MTR_B_ENCB_BIT       PORTY03_BIT


//h-bridge (A is left, B is right)
#define MTR_A_ENABLE        PWM_PORTY10
#define MTR_A_IN1_TRIS      PORTY11_TRIS
#define MTR_A_IN1_LAT       PORTY11_LAT
#define MTR_A_IN2_TRIS      PORTY12_TRIS
#define MTR_A_IN2_LAT       PORTY12_LAT
#define MTR_A_ENCA_TRIS      PORTY09_TRIS
#define MTR_A_ENCA_BIT       PORTY09_BIT
#define MTR_A_ENCB_TRIS      PORTY03_TRIS
#define MTR_A_ENCB_BIT       PORTY03_BIT

#define MTR_B_ENABLE        PWM_PORTY04
#define MTR_B_IN1_TRIS      PORTY05_TRIS
#define MTR_B_IN1_LAT       PORTY05_LAT
#define MTR_B_IN2_TRIS      PORTY06_TRIS
#define MTR_B_IN2_LAT       PORTY06_LAT
#define MTR_B_ENCA_TRIS      PORTY07_TRIS
#define MTR_B_ENCA_BIT       PORTY07_BIT
#define MTR_B_ENCB_TRIS      PORTY08_TRIS
#define MTR_B_ENCB_BIT       PORTY08_BIT

//PIN DEFINES - TRACK WIRE 
#define TRACK_WIRE_ADC AD_PORTW3
#define TRACK_WIRE_TRISA PORTW04_TRIS
#define TRACK_WIRE_TRISB PORTW05_TRIS
#define TRACK_WIRE_LATA PORTW04_LAT
#define TRACK_WIRE_LATB PORTW05_LAT

//PIN DEFINES - SOLENOID
#define SOLENOID_TRIS PORTV03_TRIS
#define SOLENOID_LAT PORTV03_LAT

//PIN DEFINES - BEACON  
#define BEACON_ADC AD_PORTV4
#define BEACON_TRISA PORTV05_TRIS
#define BEACON_TRISB PORTV06_TRIS
#define BEACON_TRISC PORTV07_TRIS
#define BEACON_LATA PORTV05_LAT
#define BEACON_LATB PORTV06_LAT
#define BEACON_LATC PORTV07_LAT


//BEACON MASK
//#define UPPER_BEACON_BOUND 630
//#define LOWER_BEACON_BOUND 600
#define LEFT_BEACON_MASK 0x1 //need to add corresponding offset amount macros
#define FRONT_BEACON_MASK 0x2
#define RIGHT_BEACON_MASK 0x4

//TAPE MASK
#define FRONT_LEFT_TAPE_MASK  0x01
#define FRONT_RIGHT_TAPE_MASK 0x02
#define FRONT_CENTER_TAPE_MASK 0x04
#define BACK_LEFT_TAPE_MASK   0x08
#define BACK_RIGHT_TAPE_MASK  0x10
#define SIDE_BACK_TAPE_MASK   0x20
#define SIDE_FRONT_TAPE_MASK  0x40

#define ALL_FLOOR_TAPE_MASK (FRONT_LEFT_TAPE_MASK | FRONT_RIGHT_TAPE_MASK | FRONT_CENTER_TAPE_MASK | BACK_LEFT_TAPE_MASK | BACK_RIGHT_TAPE_MASK)
#define ALL_SIDE_TAPE (SIDE_BACK_TAPE_MASK|SIDE_FRONT_TAPE_MASK)

//BUMPER MASK

#define FRONT_LEFT_BMP_MASK  0x01
#define FRONT_RIGHT_BMP_MASK 0x02
#define BACK_LEFT_BMP_MASK   0x04
#define BACK_RIGHT_BMP_MASK  0x08
#define SIDE_BACK_BMP_MASK  0x10
#define SIDE_FRONT_BMP_MASK  0x20

#define ALL_BUMPERS_MASK (FRONT_LEFT_BMP_MASK |FRONT_RIGHT_BMP_MASK | BACK_LEFT_BMP_MASK| BACK_RIGHT_BMP_MASK | SIDE_BACK_BMP_MASK | SIDE_FRONT_BMP_MASK) 
#define MIN_MTR_SPEED (-100)
#define MAX_MTR_SPEED (100)
#define ENC_TICKS_PER_REV 1632
#define WHEEL_DIAM_MM 62 
#define WHEEL_DIAM_1_100_IN 244 //diam in 1/100 inch units (diam in mm = 62)



/** Function Headers ***/
void Robot_Init(void);
int delay(int x);
unsigned char Robot_ReadBumpers(void);
uint16_t Robot_ReadTape(void);
unsigned char Robot_SolenoidPopBall(void); //activates solenoid for short amount of time
int16_t Robot_BatteryVoltage(void);
int16_t Robot_TrackWireDetector(void);
int16_t Robot_BeaconDetector(void);



//Motors
unsigned char Robot_LeftMtrSpeed(int mtr_speed);
unsigned char Robot_RightMtrSpeed(int mtr_speed);
int Robot_GetLeftMtrRevs(void); //returns revolutions traveled in x units
int Robot_GetRightMtrRevs(void); //returns revolutions traveled in x units
int Robot_GetLeftMtrDist(void); //returns distance traveled in 1/100 inches
int Robot_GetRightMtrDist(void); //returns distance traveled in 1/100 inches


//Encoders
int Robot_IncrementLeftEnc(void);
int Robot_DecrementLeftEnc(void);
int Robot_SetLeftEncTick(int32_t ticks);
int Robot_IncrementRightEnc(void);
int Robot_DecrementRightEnc(void);
int Robot_SetRightEncTick(int32_t ticks);

int16_t Robot_GetLeftEnc_Degrees(void); //returns encoder count in degrees rotated
int16_t Robot_GetLeftEnc_MM(void); //returns encoder count in distance traversed (in mm))

int16_t Robot_GetRightEnc_Degrees(void); //returns encoder count in degrees rotated
int16_t Robot_GetRightEnc_MM(void); //returns encoder count in distance traversed (in mm))

#endif	/* ROBOT_H */

