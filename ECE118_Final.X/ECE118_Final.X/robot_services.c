/*
 * File:   robot_services.c
 * Author: achemish
 *
 * Created on November 29, 2021, 10:04 PM
 */


#include "xc.h"
#include "robot.h"
#include "IO_Ports.h"
#include "pwm.h"
#include "AD.h"
#include "stdio.h"
#include "ES_Framework.h"
#include "robot_services.h"


//MACROS
#define BUMPER_TIMER_TICKS 50
#define TRACK_WIRE_TIMER_TICKS 10 //poll board and switch active sensor every 10 ms, so each track wire sensor is polled every 20 ms
#define BEACON_SWITCH_TIMER_TICKS 10 

//Module Variables

//service priority
static uint8_t TrackWirePriority;
static uint8_t BeaconPriority;
static uint8_t BumpPriority;
static uint8_t ManeuverPriority;
static uint8_t WaitPriority;
static uint8_t LostPriority;

//track wire
uint8_t trackwire_A = 0; //0 = not detected, 1 = detected
uint8_t trackwire_B = 0; //0 = not detected, 1 = detected
uint8_t * trackwire_active = &trackwire_A; //pointer to currently active track wire detector's variable)
uint8_t trackwire_A_out = 0; //
uint8_t trackwire_B_out = 0; //
uint16_t trackwire_reading;




//Track Wire Service
//
//uint8_t InitTrackWireService(uint8_t Priority) {
//    ES_Event TrackWireEvent;
//
//    TrackWirePriority = Priority;
//
//    // in here you write your initialization code
//    // this includes all hardware and software initialization
//    // that needs to occur.
//
//    //set digital switch selector pins to activate trackwire sensor A first
//    TRACK_WIRE_LATA = 1;
//    TRACK_WIRE_LATB = 0;
//
//    ES_Timer_InitTimer(TRACK_WIRE_SERVICE_TIMER, TRACK_WIRE_TIMER_TICKS);
//
//    // post the initial transition event
//    TrackWireEvent.EventType = ES_INIT;
//
//    if (ES_PostToService(TrackWirePriority, TrackWireEvent) == TRUE) {
//        return TRUE;
//    } else {
//
//        return FALSE;
//    }
//}
//
//uint8_t PostTrackWireService(ES_Event ThisEvent) {
//
//    return ES_PostToService(TrackWirePriority, ThisEvent);
//}
//
//ES_Event RunTrackWireService(ES_Event ThisEvent) {
//    ES_Event ReturnEvent;
//    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
//
//    static uint8_t last_trackwire_state = 0x000;
//    ES_EventTyp_t curEvent;
//
//    switch (ThisEvent.EventType) {
//        case ES_INIT:
//            // No hardware initialization or single time setups, those
//            // go in the init function above.
//
//            // This section is used to reset service for some reason
//            break;
//
//        case ES_TIMERACTIVE:
//        case ES_TIMERSTOPPED:
//            break;
//
//            //check for change in bumper state and post event if changed
//        case ES_TIMEOUT:
//            //read track wire value
//            trackwire_reading = AD_ReadADPin(TRACK_WIRE_ADC); // read bumpers
//            //printf("TrackWireVal: %d\n", trackwire_reading);
//            //update state of trackwire detector 
//            if (0 == (*trackwire_active)) {
//                if (trackwire_reading > UPPER_TRACKWIRE_BOUND) {
//                    *trackwire_active = 1;
//
//                    //post event if both track wire sensors are detecting
//                    if (trackwire_A && trackwire_B) {
//                        curEvent = TRACK_WIRE_DETECTED;
//                        ReturnEvent.EventType = curEvent;
//                        ReturnEvent.EventParam = 0xFF; //does this need to be anything? placeholder
//                        printf("Trackwire Detected!\n");
//                        PostTrackWireService(ReturnEvent);
//                    }
//                }
//
//            } else {
//                if (trackwire_reading < LOWER_TRACKWIRE_BOUND) {
//                    //trackwire is considered lost if either sensor stops detecting it
//                    if (trackwire_A && trackwire_B) {
//                        curEvent = TRACK_WIRE_LOST;
//                        ReturnEvent.EventType = curEvent;
//                        ReturnEvent.EventParam = 0xFF; //does this need to be anything? placeholder
//                        printf("Trackwire Lost!\n");
//                        PostTrackWireService(ReturnEvent);
//                    }
//
//                    //update state of active trackwire
//                    *trackwire_active = 0;
//
//
//                }
//            }
//            //switch active pins and pointers
//            TRACK_WIRE_LATA = (TRACK_WIRE_LATA + 1) % 2;
//            TRACK_WIRE_LATB = (TRACK_WIRE_LATB + 1) % 2;
//
//            //switch pointer
//            if (trackwire_active == (&trackwire_A)) {
//                trackwire_active = &trackwire_B;
//            } else {
//                trackwire_active = &trackwire_A;
//
//            }
//
//            ES_Timer_InitTimer(TRACK_WIRE_SERVICE_TIMER, TRACK_WIRE_TIMER_TICKS);
//
//
//
//#ifndef SIMPLESERVICE_TEST           // keep this as is for test harness
//            //PostRoachFSM(ReturnEvent);
//#else
//            PostTrackWireService(ReturnEvent);
//#endif   
//            break;
//
//    }
//    return ReturnEvent;
//}


//Bumper Service

uint8_t InitBumperDebounceService(uint8_t Priority) {
    ES_Event BumpEvent;

    BumpPriority = Priority;

    // in here you write your initialization code
    // this includes all hardware and software initialization
    // that needs to occur.

    ES_Timer_InitTimer(BUMPER_DEBOUNCE_SERVICE_TIMER, BUMPER_TIMER_TICKS);

    // post the initial transition event
    BumpEvent.EventType = ES_INIT;

    if (ES_PostToService(BumpPriority, BumpEvent) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

uint8_t PostBumperDebounceService(ES_Event ThisEvent) {
    return ES_PostToService(BumpPriority, ThisEvent);
}

ES_Event RunBumperDebounceService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    static uint8_t last_bumper_state = 0x000;
    ES_EventTyp_t curEvent;
    uint8_t bumper_state = Robot_ReadBumpers(); // read bumpers

    switch (ThisEvent.EventType) {
        case ES_INIT:
            // No hardware initialization or single time setups, those
            // go in the init function above.

            // This section is used to reset service for some reason
            break;

        case ES_TIMERACTIVE:
            break;
        case ES_TIMERSTOPPED:
            break;

            //check for change in bumper state and post event if changed
        case ES_TIMEOUT:
            ES_Timer_InitTimer(BUMPER_DEBOUNCE_SERVICE_TIMER, BUMPER_TIMER_TICKS);
            if (bumper_state != last_bumper_state) {
                curEvent = BUMPERS_CHANGED;
                ReturnEvent.EventType = curEvent;
                ReturnEvent.EventParam = bumper_state;
                last_bumper_state = bumper_state; // update history
                printf("BUMPER_CHANGED: 0x%X\n", bumper_state);

#ifndef SIMPLESERVICE_TEST           // keep this as is for test harness
                PostRobotHSM(ReturnEvent);
#else
                PostBumperDebounceService(ReturnEvent);
#endif   
            }
            break;
#ifdef SIMPLESERVICE_TEST     // keep this as is for test harness      
        default:
            printf("\r\nEvent: %s\tParam: 0x%X",
                    EventNames[ThisEvent.EventType], ThisEvent.EventParam);
            break;
#endif
    }

    return ReturnEvent;
}


//Maneuver Timer

uint8_t InitManeuverService(uint8_t Priority) {
    ES_Event ManeuverEvent;

    ManeuverPriority = Priority;

    // in here you write your initialization code
    // this includes all hardware and software initialization
    // that needs to occur.

    // post the initial transition event
    ManeuverEvent.EventType = ES_INIT;

    if (ES_PostToService(ManeuverPriority, ManeuverEvent) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

uint8_t PostManeuverService(ES_Event ThisEvent) {
    return ES_PostToService(ManeuverPriority, ThisEvent);
}

ES_Event RunManeuverService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    ES_EventTyp_t curEvent;


    //post LeftTurnOver event
    switch (ThisEvent.EventType) {
        case ES_INIT:
            // No hardware initialization or single time setups, those
            // go in the init function above.

            // This section is used to reset service for some reason
            break;

        case ES_TIMERACTIVE:
            break;
        case ES_TIMERSTOPPED:
            break;

            //check for change in bumper state and post event if changed
        case ES_TIMEOUT:
            curEvent = MANEUVER_OVER;
            ReturnEvent.EventType = curEvent;
            ReturnEvent.EventParam = 0; //todo: do we need this to be anything?
#ifndef SIMPLESERVICE_TEST           // keep this as is for test harness
            PostRobotHSM(ReturnEvent);
#else
            PostBumperDebounceService(ReturnEvent);
#endif   
    }
    return ReturnEvent;
}

uint8_t InitWaitService(uint8_t Priority) {
    ES_Event WaitEvent;

    WaitPriority = Priority;

    // in here you write your initialization code
    // this includes all hardware and software initialization
    // that needs to occur.

    // post the initial transition event
    WaitEvent.EventType = ES_INIT;

    if (ES_PostToService(WaitPriority, WaitEvent) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

uint8_t PostWaitService(ES_Event ThisEvent) {
    return ES_PostToService(WaitPriority, ThisEvent);
}

ES_Event RunWaitService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    ES_EventTyp_t curEvent;


    //post LeftTurnOver event
    switch (ThisEvent.EventType) {
        case ES_INIT:
            // No hardware initialization or single time setups, those
            // go in the init function above.

            // This section is used to reset service for some reason
            break;

        case ES_TIMERACTIVE:
            break;
        case ES_TIMERSTOPPED:
            break;

            //check for change in bumper state and post event if changed
        case ES_TIMEOUT:
            curEvent = WAIT_OVER;
            ReturnEvent.EventType = curEvent;
            ReturnEvent.EventParam = 0; //todo: do we need this to be anything?
#ifndef SIMPLESERVICE_TEST           // keep this as is for test harness
            PostRobotHSM(ReturnEvent);
#else
            PostBumperDebounceService(ReturnEvent);
#endif   
    }
    return ReturnEvent;
}



uint8_t InitLostService(uint8_t Priority) {
    ES_Event LostEvent;

    LostPriority = Priority;

    // in here you write your initialization code
    // this includes all hardware and software initialization
    // that needs to occur.

    // post the initial transition event
    LostEvent.EventType = ES_INIT;

    if (ES_PostToService(LostPriority, LostEvent) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

uint8_t PostLostService(ES_Event ThisEvent) {
    return ES_PostToService(LostPriority, ThisEvent);
}

ES_Event RunLostService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    ES_EventTyp_t curEvent;


    //post LeftTurnOver event
    switch (ThisEvent.EventType) {
        case ES_INIT:
            // No hardware initialization or single time setups, those
            // go in the init function above.

            // This section is used to reset service for some reason
            break;

        case ES_TIMERACTIVE:
            break;
        case ES_TIMERSTOPPED:
            break;

            //check for change in bumper state and post event if changed
        case ES_TIMEOUT:
            curEvent = LOST_OVER;
            ReturnEvent.EventType = curEvent;
            ReturnEvent.EventParam = 0; //todo: do we need this to be anything?
#ifndef SIMPLESERVICE_TEST           // keep this as is for test harness
            PostRobotHSM(ReturnEvent);
#else
            //PostBumperDebounceService(ReturnEvent);
#endif   
    }
    return ReturnEvent;
}
