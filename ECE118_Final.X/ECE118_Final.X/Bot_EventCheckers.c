/*
 * File:   TemplateEventChecker.c
 * Author: Gabriel Hugh Elkaim
 *
 * Template file to set up typical EventCheckers for the  Events and Services
 * Framework (ES_Framework) on the Uno32 for the CMPE-118/L class. Note that
 * this file will need to be modified to fit your exact needs, and most of the
 * names will have to be changed to match your code.
 *
 * This EventCheckers file will work with both FSM's and HSM's.
 *
 * Remember that EventCheckers should only return TRUE when an event has occured,
 * and that the event is a TRANSITION between two detectable differences. They
 * should also be atomic and run as fast as possible for good results.
 *
 * This file includes a test harness that will run the event detectors listed in the
 * ES_Configure file in the project, and will conditionally compile main if the macro
 * EVENTCHECKER_TEST is defined (either in the project or in the file). This will allow
 * you to check you event detectors in their own project, and then leave them untouched
 * for your project unless you need to alter their post functions.
 *
 * Created on September 27, 2013, 8:37 AM
 */

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "Bot_EventCheckers.h"
#include "ES_Events.h"
#include "serial.h"
#include "AD.h"
#include "robot.h"
#include "stdio.h"
#include "RobotHSM.h"
#include "ES_Timers.h"
/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define BATTERY_DISCONNECT_THRESHOLD 175
#define UPPER_BEACON_BOUND 820
#define LOWER_BEACON_BOUND 720
#define BEACON_NUM_SAMPLES 200
#define TRACKWIRE_NUM_SAMPLES 20
#define TRACKWIRE_DEBUG_PRINT 0 //set to 1 to print each calculated trackwire avg. Do with high
                                //trackwire switch time or you will spam print.
#define TRACKWIRE_SWITCH_TIME 10 //time in ms between switching (and reading) trackwire sensors
                                //set to 2 for nromal operation. Set to 500 when debugging 
                                //if setting print trackwrire values to true


//TRACK WIRE
#define UPPER_TRACKWIRE_BOUND 580
#define LOWER_TRACKWIRE_BOUND 540
/*******************************************************************************
 * EVENTCHECKER_TEST SPECIFIC CODE                                                             *
 ******************************************************************************/

//#define EVENTCHECKER_TEST
#ifdef EVENTCHECKER_TEST
#define SaveEvent(x) do {eventName=__func__; storedEvent=x;} while (0)

static const char *eventName;
static ES_Event storedEvent;
#endif

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this EventChecker. They should be functions
   relevant to the behavior of this particular event checker */

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/

/* Any private module level variable that you might need for keeping track of
   events would be placed here. Private variables should be STATIC so that they
   are limited in scope to this module. */
enum encoder_states {
    state_0_0, state_0_1, state_1_0, state_1_1
};

enum encoder_steps {
    enc_no_step, enc_increment, enc_decrement
};
int left_encoder_state = state_0_0;
int32_t left_last_count = 0;
int test_bool = 0;
/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function TemplateCheckBattery(void)
 * @param none
 * @return TRUE or FALSE
 * @brief This function is a prototype event checker that checks the battery voltage
 *        against a fixed threshold (#defined in the .c file). Note that you need to
 *        keep track of previous history, and that the actual battery voltage is checked
 *        only once at the beginning of the function. The function will post an event
 *        of either BATTERY_CONNECTED or BATTERY_DISCONNECTED if the power switch is turned
 *        on or off with the USB cord plugged into the Uno32. Returns TRUE if there was an 
 *        event, FALSE otherwise.
 * @note Use this code as a template for your other event checkers, and modify as necessary.
 * @author Gabriel H Elkaim, 2013.09.27 09:18
 * @modified Gabriel H Elkaim/Max Dunne, 2016.09.12 20:08 */
uint8_t CheckBattery(void) {
    static ES_EventTyp_t lastEvent = BATTERY_DISCONNECTED;
    ES_EventTyp_t curEvent;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
    uint16_t batVoltage = AD_ReadADPin(BAT_VOLTAGE); // read the battery voltage

    if (batVoltage > BATTERY_DISCONNECT_THRESHOLD) { // is battery connected?
        curEvent = BATTERY_CONNECTED;
    } else {
        curEvent = BATTERY_DISCONNECTED;

    }
    if (curEvent != lastEvent) { // check for change from last time
        thisEvent.EventType = curEvent;
        thisEvent.EventParam = batVoltage;
        returnVal = TRUE;
        lastEvent = curEvent; // update history
        //printf("Battery Change: 0x%X\n", batVoltage);
        //printf("testbool: %d   ", test_bool);
#ifndef EVENTCHECKER_TEST           // keep this as is for test harness
        //PostGenericService(thisEvent);
        PostRobotHSM(thisEvent);
#else
        SaveEvent(thisEvent);
#endif   
    }
    return (returnVal);
}

uint8_t CheckTape(void) {
    static uint16_t prevTape = 0x00;

    ES_EventTyp_t curEvent;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
    uint16_t currentTape = Robot_ReadTape(); // read the battery voltage

    if (currentTape != prevTape) { // is battery connected?
        prevTape = currentTape;
        curEvent = TAPE_CHANGED;
        thisEvent.EventType = curEvent;
        thisEvent.EventParam = currentTape;
        returnVal = TRUE;
        //printf("TAPE_CHANGED: 0x%X\n", currentTape);

#ifndef EVENTCHECKER_TEST           // keep this as is for test harness
        //PostGenericService(thisEvent);
        PostRobotHSM(thisEvent);
#else
        SaveEvent(thisEvent);
#endif   
    }
    return (returnVal);
}

//uint8_t CheckBumper(void) {
//    static uint16_t prevBumper_state = 0xFF;
//    ES_EventTyp_t curEvent;
//    ES_Event thisEvent;
//    uint8_t returnVal = FALSE;
//    uint16_t newBumper_state = Robot_ReadBumpers(); // read the battery voltage
//
//    if (newBumper_state != prevBumper_state) { // is battery connected?
//        curEvent = BUMPERS_CHANGED;
//        thisEvent.EventType = curEvent;
//        thisEvent.EventParam = newBumper_state;
//        returnVal = TRUE;
//        prevBumper_state = newBumper_state;
//        printf("BUMPER_CHANGED: 0x%X\n", newBumper_state);
//
//#ifndef EVENTCHECKER_TEST           // keep this as is for test harness
//        //PostGenericService(thisEvent);
//        PostRobotHSM(thisEvent);
//
//#else
//        SaveEvent(thisEvent);
//#endif   
//    }
//    return (returnVal);
//}

uint8_t CheckBeacon(void) {
    static uint32_t sample_sum = 0;
    static uint32_t start_time = 0;
    static uint8_t sample_count = 0;
    static uint8_t wait = 0;
    static uint8_t beacon_state[] = {0, 0, 0};

    //beacon (from services))
    static uint8_t * beacon_active = &beacon_state[1]; //pointer to currently active beacon detector's variable
    static uint8_t beacon_select = 1;

    ES_EventTyp_t curEvent;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
    uint16_t sample_avg = 0;
    if (wait) {

        if ((ES_Timer_GetTime() - start_time) > 2) { //wait at least 1 ms after switching beacon
            wait = 0;

        }
    } else {

        //take next sample if goal not reached
        if (sample_count < BEACON_NUM_SAMPLES) {
            sample_sum += AD_ReadADPin(BEACON_ADC);
            sample_count++;
        }//update beacon state based on avg and switch beacons
        else {

            //calculate avg and update state
            sample_avg = sample_sum / BEACON_NUM_SAMPLES;
            //printf("   avg: %d       \n", sample_avg);

            //if beacon detector low on last read && current reading above high threshold 
            if ((0 == (*beacon_active)) && (sample_avg > UPPER_BEACON_BOUND)) {
                *beacon_active = 1;
                curEvent = BEACON_CHANGED;

                //if beacon detector high on last read && current reading below low threshold
            } else if ((*beacon_active) && (sample_avg < LOWER_BEACON_BOUND)) {
                *beacon_active = 0;
                curEvent = BEACON_CHANGED;
            }

            //post event if beacon detection state changed
            if (BEACON_CHANGED == curEvent) {
                returnVal = TRUE;

                thisEvent.EventType = curEvent;
                thisEvent.EventParam = ((beacon_state[0] << 2) | (beacon_state[1] << 1) | beacon_state[2]);

#ifndef EVENTCHECKER_TEST           // keep this as is for test harness
                PostRobotHSM(thisEvent);
#else
                SaveEvent(thisEvent);
#endif  

            }

            //switch active beacon
            //beacon_select = (beacon_select + 1) % 3; //loop through 0, 1, 2
            beacon_active = &beacon_state[beacon_select];
            BEACON_LATA = (0 == beacon_select);
            BEACON_LATB = (1 == beacon_select);
            BEACON_LATC = (2 == beacon_select);

            //set values
            start_time = ES_Timer_GetTime();
            wait = 1;
            sample_count = 0;
            sample_sum = 0;

            //printf("beacon ec done");

        }
    }
    return (returnVal);
}

uint8_t CheckTrackWire(void) {
    static uint32_t sample_sum = 0;
    static uint32_t start_time = 0;
    static uint8_t sample_count = 0;
    static uint8_t wait = 0;
    static uint8_t trackwire_state[] = {0, 0};

    //beacon (from services))
    static uint8_t * trackwire_active = &trackwire_state[0]; //pointer to currently active trackwire detector's variable
    static uint8_t trackwire_select = 0;

    ES_EventTyp_t curEvent;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;
    uint16_t sample_avg = 0;
    if (wait) {

        //TODO- SET VALUE TO 2 NOT 500 DURING NORMAL OP
        if ((ES_Timer_GetTime() - start_time) > TRACKWIRE_SWITCH_TIME) { //wait at least 1 ms after switching trackwire
            wait = 0;
        }
    } else {

        //take next sample if goal not reached
        if (sample_count < TRACKWIRE_NUM_SAMPLES) {
            sample_sum += AD_ReadADPin(TRACK_WIRE_ADC);
            sample_count++;
        }//update beacon state based on avg and switch beacons
        else {
 
            //calculate avg and update state
            sample_avg = sample_sum / TRACKWIRE_NUM_SAMPLES;
            if(TRACKWIRE_DEBUG_PRINT){
            printf("t%d: %d       \n", trackwire_select, sample_avg);
            }
            if ((!(*trackwire_active)) && (sample_avg > UPPER_TRACKWIRE_BOUND)) {
                *trackwire_active = 1;

                //post event indicating trackwire found if both track wire sensors are now detecting
                if (trackwire_state[0] && trackwire_state[1]) {
                    returnVal = TRUE;
                    thisEvent.EventParam = 1; //trackwire found

                    thisEvent.EventType = TRACK_WIRE_CHANGED;
                }

            } else if ((*trackwire_active) && (sample_avg < LOWER_TRACKWIRE_BOUND)) {
                //trackwire is considered lost both sensors were detecting before (so either sensor is now no longer detecting)
                if ((trackwire_state[0] && trackwire_state[1])) { //checking last state of track wire
                    returnVal = TRUE;

                    thisEvent.EventType = TRACK_WIRE_CHANGED;
                    thisEvent.EventParam = 0; //trackwire lost

                }

                //update state of active trackwire
                *trackwire_active = 0;
            }
            //post event if beacon detection state changed

            if (TRACK_WIRE_CHANGED == thisEvent.EventType) {
                    returnVal = TRUE;

#ifndef EVENTCHECKER_TEST           // keep this as is for test harness
                    PostRobotHSM(thisEvent);
#else
                    SaveEvent(thisEvent);
#endif  

                }

            //switch active beacon
            trackwire_select = !trackwire_select; //switch between 1 and 0
            trackwire_active = &trackwire_state[trackwire_select];
            TRACK_WIRE_LATA = trackwire_select;
            TRACK_WIRE_LATB = !trackwire_select;
            //set values
            start_time = ES_Timer_GetTime();
            wait = 1;
            sample_count = 0;
            sample_sum = 0;

            //printf("beacon ec done");

        }
    }
    return (returnVal);
}

//Encoder code reused from Aram's ECE 167 encoder lab
//This function tracks the steps of the encoder, and appropriately increments 
// or decrements the encounter step count held in the robot.c module (the step
// count must be accessible with getter functions in the robot.c module. Keeping 
// count in this module and setting the value in the robot.c module every step 
// would be much more expensive due to the large variable size)

uint8_t CheckLeftEncoder(void) {
    static uint16_t left_encoder_state = 0x0;
    //ES_EventTyp_t curEvent;
    //ES_Event thisEvent;
    int8_t returnVal = FALSE;


    int A = MTR_A_ENCA_BIT;
    int B = MTR_A_ENCB_BIT;

    //SM Code
    int left_new_state = (A << 1) + B;
    int left_encoder_step = enc_no_step; //no step by default
    if (left_new_state != left_encoder_state) {
        switch (left_encoder_state) {
            case state_0_0:
                //cw
                if (left_new_state == state_1_0) {
                    left_encoder_step = enc_increment;
                }//ccw
                else if (left_new_state == state_0_1) {
                    left_encoder_step = enc_decrement;
                }
                break;
            case state_1_0:
                //cw
                if (left_new_state == state_1_1) {
                    left_encoder_step = enc_increment;
                }//ccw
                else if (left_new_state == state_0_0) {
                    left_encoder_step = enc_decrement;
                }
                break;
            case state_1_1:
                //cw
                if (left_new_state == state_0_1) {
                    left_encoder_step = enc_increment;
                }//ccw
                else if (left_new_state == state_1_0) {
                    left_encoder_step = enc_decrement;
                }
                break;

            case state_0_1:
                //cw
                if (left_new_state == state_0_0) {
                    left_encoder_step = enc_increment;
                }//ccw
                else if (left_new_state == state_1_1) {
                    left_encoder_step = enc_decrement;
                }
                break;
        }

        //update state
        left_encoder_state = left_new_state;

        //update robot.c count
        if (enc_increment == left_encoder_step) {
            Robot_IncrementLeftEnc();
        } else if (enc_decrement == left_encoder_step) {
            Robot_DecrementLeftEnc();

        }

        //        if (abs(encoder_count - last_count) > 10) {
        //            printf("state:%d count:%d\n", encoder_state, encoder_count);
        //            last_count = encoder_count;
        //        }
    }
    return (returnVal);

}

uint8_t CheckRightEncoder(void) {
    static uint16_t right_encoder_state = 0x0;
    //ES_EventTyp_t curEvent;
    //ES_Event thisEvent;
    int8_t returnVal = FALSE;


    int A = MTR_A_ENCA_BIT;
    int B = MTR_A_ENCB_BIT;

    //SM Code
    int right_new_state = (A << 1) + B;
    int right_encoder_step = enc_no_step; //no step by default
    if (right_new_state != right_encoder_state) {
        switch (right_encoder_state) {
            case state_0_0:
                //cw
                if (right_new_state == state_1_0) {
                    right_encoder_step = enc_increment;
                }//ccw
                else if (right_new_state == state_0_1) {
                    right_encoder_step = enc_decrement;
                }
                break;
            case state_1_0:
                //cw
                if (right_new_state == state_1_1) {
                    right_encoder_step = enc_increment;
                }//ccw
                else if (right_new_state == state_0_0) {
                    right_encoder_step = enc_decrement;
                }
                break;
            case state_1_1:
                //cw
                if (right_new_state == state_0_1) {
                    right_encoder_step = enc_increment;
                }//ccw
                else if (right_new_state == state_1_0) {
                    right_encoder_step = enc_decrement;
                }
                break;

            case state_0_1:
                //cw
                if (right_new_state == state_0_0) {
                    right_encoder_step = enc_increment;
                }//ccw
                else if (right_new_state == state_1_1) {
                    right_encoder_step = enc_decrement;
                }
                break;
        }

        //update state
        right_encoder_state = right_new_state;

        //update robot.c count
        if (enc_increment == right_encoder_step) {
            Robot_IncrementRightEnc();
        } else if (enc_decrement == right_encoder_step) {
            Robot_DecrementRightEnc();

        }
    }
    return (returnVal);

}

/* 
 * The Test Harness for the event checkers is conditionally compiled using
 * the EVENTCHECKER_TEST macro (defined either in the file or at the project level).
 * No other main() can exist within the project.
 * 
 * It requires a valid ES_Configure.h file in the project with the correct events in 
 * the enum, and the correct list of event checkers in the EVENT_CHECK_LIST.
 * 
 * The test harness will run each of your event detectors identically to the way the
 * ES_Framework will call them, and if an event is detected it will print out the function
 * name, event, and event parameter. Use this to test your event checking code and
 * ensure that it is fully functional.
 * 
 * If you are locking up the output, most likely you are generating too many events.
 * Remember that events are detectable changes, not a state in itself.
 * 
 * Once you have fully tested your event checking code, you can leave it in its own
 * project and point to it from your other projects. If the EVENTCHECKER_TEST marco is
 * defined in the project, no changes are necessary for your event checkers to work
 * with your other projects.
 */
#ifdef EVENTCHECKER_TEST
#include <stdio.h>
static uint8_t(*EventList[])(void) = {EVENT_CHECK_LIST};

void PrintEvent(void);

void main(void) {
    BOARD_Init();
    /* user initialization code goes here */
    Robot_Init();
    ES_Timer_Init();
    //Robot_SetLeftMtrSpeed(1000);
    //Robot_PopPingPongbBall();
    // Do not alter anything below this line
    int i;

    printf("\r\nEvent checking test harness for %s", __FILE__);

    while (1) {
        if (IsTransmitEmpty()) {
            for (i = 0; i< sizeof (EventList) >> 2; i++) {
                if (EventList[i]() == TRUE) {
                    PrintEvent();
                    break;
                }

            }
        }
    }
}

void PrintEvent(void) {
    printf("\r\nFunc: %s\tEvent: %s\tParam: 0x%X", eventName,
            EventNames[storedEvent.EventType], storedEvent.EventParam);
}
#endif