/*
 * File: TemplateSubHSM.c
 * Author: J. Edward Carryer
 * Modified: Gabriel Elkaim and Soja-Marie Morgens
 *
 * Template file to set up a Heirarchical State Machine to work with the Events and
 * Services Framework (ES_Framework) on the Uno32 for the CMPE-118/L class. Note that
 * this file will need to be modified to fit your exact needs, and most of the names
 * will have to be changed to match your code.
 *
 * There is another template file for the SubHSM's that is slightly differet, and
 * should be used for all of the subordinate state machines (flat or heirarchical)
 *
 * This is provided as an example and a good place to start.
 *
 * History
 * When           Who     What/Why
 * -------------- ---     --------
 * 09/13/13 15:17 ghe      added tattletail functionality and recursive calls
 * 01/15/12 11:12 jec      revisions for Gen2 framework
 * 11/07/11 11:26 jec      made the queue static
 * 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
 * 10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
 */


/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "RobotHSM.h"
#include "TowardsTowerSubHSM.h"
#include "AtTowerSubHSM.h"
#include "OnTapeSubHSM.h"
#include "TapeSubState.h"
#include "robot.h"
#include "TowerAlignSubHSM.h"
#include "TowerTraverseSubHSM.h"
#include "TowerShootSubHSM.h"
#include "stdio.h"
/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/


typedef enum {
    InitPState,
    Set_Up, //bot waits for 0.5 seconds to wait for false HIGH signal from beacon detector to go away
    Spin_Scan, //spinning in place to scan for beacon
    Towards_Tower, //approaching tower, adjusting direction to stay on course
    At_Tower, //at tower and working to put ball into correct hole
    Perimeter_Scan, //at tower after ball deposit, following perimeter while scanning with side beacon detector
    On_Tape, //tape following around corners, scanning for beacon on corners
    Lost, //any state where moving and not going towards tower, ie waiting to bounce of tape
    Traverse_Scan,
} TemplateHSMState_t;

static const char *StateNames[] = {
	"InitPState",
	"Set_Up",
	"Spin_Scan",
	"Towards_Tower",
	"At_Tower",
	"Perimeter_Scan",
	"On_Tape",
	"Lost",
	"Traverse_Scan",
};


/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine
   Example: char RunAway(uint_8 seconds);*/
/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */

static TemplateHSMState_t CurrentState = InitPState; // <- change enum name to match ENUM
static uint8_t MyPriority;


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitTemplateHSM(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunTemplateFSM function. Remember
 *        to rename this to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t InitRobotHSM(uint8_t Priority) {
    MyPriority = Priority;
    // put us into the Initial PseudoState
    CurrentState = InitPState;
    // post the initial transition event
    if (ES_PostToService(MyPriority, INIT_EVENT) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function PostTemplateHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief This function is a wrapper to the queue posting function, and its name
 *        will be used inside ES_Configure to point to which queue events should
 *        be posted to. Remember to rename to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t PostRobotHSM(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function RunTemplateHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This function is where you implement the whole of the heirarchical state
 *        machine, as this is called any time a new event is passed to the event
 *        queue. This function will be called recursively to implement the correct
 *        order for a state transition to be: exit current state -> enter next state
 *        using the ES_EXIT and ES_ENTRY events.
 * @note Remember to rename to something appropriate.
 *       The lower level state machines are run first, to see if the event is dealt
 *       with there rather than at the current level. ES_EXIT and ES_ENTRY events are
 *       not consumed as these need to pass pack to the higher level state machine.
 * @author J. Edward Carryer, 2011.10.23 19:25
 * @author Gabriel H Elkaim, 2011.10.23 19:25 */
ES_Event RunRobotHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    TemplateHSMState_t nextState; // <- change type to correct enum

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case InitPState: // If current state is initial Pseudo State
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
                printf("Init State!\n");

                // Init Transition Actions
                ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 400);

                // Initialize all sub-state machines
                InitTowardsTowerSubHSM();
                //InitOnTapeSubHSM();
                InitTapeSubState();
                InitTowerAlignSubHSM();
                InitTowerTraverseSubHSM();
                InitTowerShootSubHSM();
                InitAtTowerSubHSM();

                // now put the machine into the actual initial state
                nextState = Set_Up;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;

            }
            break;
        case Set_Up:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case MANEUVER_OVER: //maybe only begin manuever timer if battery connected

                    //begin spinning in place
                    Robot_LeftMtrSpeed(80);
                    Robot_RightMtrSpeed(-80);


                    nextState = Spin_Scan;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

                default:
                    break;
            }
            break;
        case Spin_Scan:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case BEACON_CHANGED:
                    //if front beacon is detecting
                    if (ThisEvent.EventParam & FRONT_BEACON_MASK) {
                        //begin approaching beacon
                        Robot_LeftMtrSpeed(100);
                        Robot_RightMtrSpeed(100);

                        nextState = Towards_Tower;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case BUMPERS_CHANGED: //edge cases: bot runs into us or we are near beacon
                    break;
                default:
                    break;
            }
            break;
        case Towards_Tower:
            ThisEvent = RunTowardsTowerSubHSM(ThisEvent);


            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case BUMPERS_CHANGED: //need to account for hitting other bot case
                    //might want to only react to front bumpers, back would not make sense
                    //stop running (might change depending on at tower behavior)
                    Robot_LeftMtrSpeed(-80);
                    Robot_RightMtrSpeed(-80);
                    ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 500);

                    nextState = At_Tower;
                    makeTransition = TRUE;
                    PostRobotHSM(ThisEvent); //todo: see if this is a valid way to repost event. Check for tape as well

                    ResetTowardsTowerSubHSM(); //todo; check with jason if this is correct place
                    ThisEvent.EventType = ES_NO_EVENT;

                    break;
                case TAPE_CHANGED:
                    if (ThisEvent.EventParam & (ALL_FLOOR_TAPE_MASK)) {

                        nextState = On_Tape;
                        makeTransition = TRUE;
                        PostRobotHSM(ThisEvent); //todo: check if this is valid way to repost

                        ThisEvent.EventType = ES_NO_EVENT;
                        ResetTowardsTowerSubHSM(); //todo; check with jason if this is correct place
                    }
                    break;
                default:
                    break;
                    //  }
            }
            break;
        case At_Tower:

            ThisEvent = RunAtTowerSubHSM(ThisEvent);


            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case DEAD_BOT_DETECTED:
                    Robot_LeftMtrSpeed(100);
                    Robot_RightMtrSpeed(100);
                    nextState = Lost;
                    makeTransition = TRUE;
                    ResetAtTowerSubHSM();
                    break;
                    //case TAPE_CHANGED:

                    //THIS SHOULD NOT BE REACHED: TAPE EVENTS SHOULD BE HANDLED AND
                    //CONSUMED IN THE AT TOWER SM. Having this case here is a fail safe
                    //                    if (ThisEvent.EventParam & (ALL_FLOOR_TAPE_MASK)) {
                    //                        nextState = On_Tape;
                    //                        makeTransition = TRUE;
                    //                        PostRobotHSM(ThisEvent); //todo: check if this is valid way to repost
                    //
                    //                        ThisEvent.EventType = ES_NO_EVENT;
                    //                        //todo: reset at tower SM
                    //                        ResetAtTowerSubHSM();
                    //                    }
                    //break;

                    //                case BALL_DEPOSITED: //to-do: change. The attower state macine
                    //                    //should search for a beacon after
                    //                    //depositing, then after finding one we
                    //                    //should enter approaching state here
                    //                    Robot_LeftMtrSpeed(80);
                    //                    Robot_RightMtrSpeed(-80);
                    //                    nextState = Spin_Scan;
                    //                    makeTransition = TRUE;
                    //
                    //                    ThisEvent.EventType = ES_NO_EVENT;
                    //                    break;

                    //                case TAPE_CHANGED:
                    //                    nextState = On_Tape;
                    //                    makeTransition = TRUE;
                    //                    PostRobotHSM(ThisEvent); //todo: check if this is valid way to repost
                    //
                    //                    ThisEvent.EventType = ES_NO_EVENT;
                    //                    break;
                    //                    //todo: reset at tower SM
                case BALL_DEPOSITED:
                    Robot_LeftMtrSpeed(90);
                    Robot_RightMtrSpeed(100);
                    nextState = Traverse_Scan;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    ResetAtTowerSubHSM();
                    break;
                    
                case LOST_OVER:
                    Robot_LeftMtrSpeed(100);
                    Robot_RightMtrSpeed(100);
                    nextState = Lost;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    ResetAtTowerSubHSM();

                    break;
                default:
                    break;
                    //  }
            }
            break;
        case On_Tape:
            ThisEvent = RunTapeSubState(ThisEvent);
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case BUMPERS_CHANGED:
                    if ((FRONT_LEFT_BMP_MASK | FRONT_RIGHT_BMP_MASK) & ThisEvent.EventParam) { //if any bumper pressed
                        Robot_LeftMtrSpeed(-80);
                        Robot_RightMtrSpeed(-80);
                        ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 500);

                        nextState = At_Tower;
                        makeTransition = TRUE;
                        PostRobotHSM(ThisEvent); //todo: see if this is a valid way to repost event. Check for tape as well

                        ResetTapeSubState(); //todo; check with jason if this is correct place
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    ThisEvent.EventType = ES_NO_EVENT;

                    break;

                case ESCAPED_TAPE:
                    //begin spinning towards bumped side
                    if (ThisEvent.EventParam & FRONT_LEFT_TAPE_MASK) { //to-do: see if this works?
                        //spin to left
                        Robot_LeftMtrSpeed(-80);
                        Robot_RightMtrSpeed(80);
                    } else {
                        Robot_RightMtrSpeed(-80);
                        Robot_LeftMtrSpeed(80);
                    }

                    ResetTapeSubState(); //todo- check with jason if should be here or in exit event case
                    nextState = Spin_Scan;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default:
                    break;
            }
            break;
        case Lost:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case TAPE_CHANGED:

                    //                    if (ThisEvent.EventParam & (ALL_FLOOR_TAPE_MASK)) {
                    //                        nextState = On_Tape;
                    //                        makeTransition = TRUE;
                    //                        PostRobotHSM(ThisEvent); //todo: check if this is valid way to repost
                    //
                    //                        ThisEvent.EventType = ES_NO_EVENT;
                    //                    }
                    if (ThisEvent.EventParam & (ALL_FLOOR_TAPE_MASK)) {
                        Robot_LeftMtrSpeed(-80);
                        Robot_RightMtrSpeed(80);

                        nextState = Spin_Scan;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    ThisEvent.EventType = ES_NO_EVENT;

                    break;
                case BEACON_CHANGED:
                    //if front beacon is detecting
                    if (ThisEvent.EventParam & FRONT_BEACON_MASK) {
                        //begin approaching beacon
                        Robot_LeftMtrSpeed(100);
                        Robot_RightMtrSpeed(100);

                        nextState = Towards_Tower;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                default: // all unhandled states fall into here
                    break;

            } // end switch on Current State
            break;
        case Traverse_Scan:
            ThisEvent = RunTowerTraverseSubHSM(ThisEvent);

            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    //ES_Timer_InitTimer(LOST_SERVICE_TIMER, TIMEOUT);
                    break;

                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    //ES_Timer_StopTimer(LOST_SERVICE_TIMER);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case BEACON_CHANGED:
                    //if front beacon is detecting
                    if (ThisEvent.EventParam & FRONT_BEACON_MASK) {
                        //begin approaching beacon
                        Robot_LeftMtrSpeed(100);
                        Robot_RightMtrSpeed(90);

                        nextState = Towards_Tower;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        ResetTowerTraverseSubHSM();

                    }
                default:
                    break;
            }
            break;
    }
    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunRobotHSM(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        RunRobotHSM(ENTRY_EVENT); // <- rename to your own Run function
    }

    ES_Tail(); // trace call stack end
    return ThisEvent;
}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
