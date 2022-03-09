/*
 * File: TemplateSubHSM.c
 * Author: J. Edward Carryer
 * Modified: Gabriel H Elkaim
 *
 * Template file to set up a Heirarchical State Machine to work with the Events and
 * Services Framework (ES_Framework) on the Uno32 for the CMPE-118/L class. Note that
 * this file will need to be modified to fit your exact needs, and most of the names
 * will have to be changed to match your code.
 *
 * There is for a substate machine. Make sure it has a unique name
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
#include "TapeSubState.h"
#include "Robot.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/

typedef enum {
    InitPSubState,
    SaveTape,
    BackUp,
    LeftTurn,
    RightTurn,
    Forward,
} TapeSubState_t;

static const char *StateNames[] = {
	"InitPSubState",
	"SaveTape",
	"BackUp",
	"LeftTurn",
	"RightTurn",
	"Forward",
};



/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine */

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */

static TapeSubState_t CurrentState = InitPSubState; // <- change name to match ENUM
static uint8_t MyPriority;
static uint8_t last_tape = 0x0;


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitTemplateSubHSM(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunTemplateFSM function. Remember
 *        to rename this to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t InitTapeSubState(void) {
    ES_Event returnEvent;

    CurrentState = InitPSubState;
    returnEvent = RunTapeSubState(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

/**
 * @Function RunTemplateSubHSM(ES_Event ThisEvent)
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
ES_Event RunTapeSubState(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    TapeSubState_t nextState; // <- change type to correct enum

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case InitPSubState: // If current state is initial Psedudo State
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
                // this is where you would put any actions associated with the
                // transition from the initial pseudo-state into the actual
                // initial state
                //Roach_BothMtrSpeed(0); //stay still 

                // now put the machine into the actual initial state
                nextState = SaveTape;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;

        case SaveTape:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case TAPE_CHANGED:

                    //end SM if no tape sensors hit
                    if (!(ThisEvent.EventParam & ALL_FLOOR_TAPE_MASK)) {
                        //escaped!
                        ThisEvent.EventType = ES_NO_EVENT;

                        ES_Event newEvent;
                        newEvent.EventType = ESCAPED_TAPE;
                        newEvent.EventParam = last_tape;

                        PostRobotHSM(newEvent);
                    }
                    //if left or right tape detected, switch to next state. Keep moving forward until these are detected. Todo: add back tape handles
                    if ((ThisEvent.EventParam & FRONT_LEFT_TAPE_MASK) || (ThisEvent.EventParam & FRONT_RIGHT_TAPE_MASK)) {
                        //save tape state
                        last_tape = ThisEvent.EventParam;

                        //begin backing up
                        Robot_LeftMtrSpeed(-100);
                        Robot_RightMtrSpeed(-100);
                        ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 600);

                        nextState = BackUp;
                        makeTransition = TRUE;

                        //go forward if back tape hit (todo: implement seperate forward left and forward right)
                    } else if (ThisEvent.EventParam & (BACK_LEFT_TAPE_MASK | BACK_RIGHT_TAPE_MASK)) {
                        Robot_LeftMtrSpeed(100);
                        Robot_RightMtrSpeed(100);
                        nextState = Forward;
                        makeTransition = TRUE;
                        ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 800);

                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    ThisEvent.EventType = ES_NO_EVENT;

                    break;

                default: // all unhandled states fall into here
                    break;
            }
            break;
        case BackUp:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case TAPE_CHANGED:
                    //stop backing up and do turn early if tape detected in back
                    if ((BACK_RIGHT_TAPE_MASK & ThisEvent.EventParam) || (BACK_LEFT_TAPE_MASK & ThisEvent.EventParam)) {

                        ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 350); //todo: use rand function here

                        if (last_tape & FRONT_LEFT_TAPE_MASK) {
                            Robot_LeftMtrSpeed(75);
                            Robot_RightMtrSpeed(-75);
                            nextState = LeftTurn;

                        } else {
                            Robot_LeftMtrSpeed(-75);
                            Robot_RightMtrSpeed(75);
                            nextState = RightTurn;
                        }
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else {
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case MANEUVER_OVER:
                    ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 350);

                    if (last_tape & FRONT_LEFT_TAPE_MASK) {
                        Robot_LeftMtrSpeed(75);
                        Robot_RightMtrSpeed(-75);
                        nextState = LeftTurn;

                    } else {
                        Robot_LeftMtrSpeed(-75);
                        Robot_RightMtrSpeed(75);
                        nextState = RightTurn;
                    }
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case BUMPERS_CHANGED: //end back up early
                    if ((BACK_LEFT_BMP_MASK | BACK_RIGHT_BMP_MASK) & ThisEvent.EventParam) {
                        if (last_tape & FRONT_LEFT_TAPE_MASK) {
                            Robot_LeftMtrSpeed(75);
                            Robot_RightMtrSpeed(-75);
                            nextState = LeftTurn;

                        } else {
                            Robot_LeftMtrSpeed(-75);
                            Robot_RightMtrSpeed(75);
                            nextState = RightTurn;
                        }
                        ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 350);
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;
                    }
                    break;
                default:
                    break;

            }
            break;



        case LeftTurn:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case TAPE_CHANGED: //todo: add code to stop and back up again?
                    break;
                case MANEUVER_OVER:

                    Robot_LeftMtrSpeed(100);
                    Robot_RightMtrSpeed(100);
                    nextState = Forward;
                    makeTransition = TRUE;
                    ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 800);

                    ThisEvent.EventType = ES_NO_EVENT;
                    break;


                default:
                    break;
            }
            break;
        case RightTurn:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case TAPE_CHANGED: //todo: add code to stop and back up again?
                    break;
                case MANEUVER_OVER:

                    Robot_LeftMtrSpeed(100);
                    Robot_RightMtrSpeed(100);
                    nextState = Forward;
                    makeTransition = TRUE;
                    ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 800);

                    ThisEvent.EventType = ES_NO_EVENT;
                    break;


                default:
                    break;
            }
            break;


        case Forward:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                    //if run into tape again
                case TAPE_CHANGED:
                    if ((ThisEvent.EventParam & FRONT_LEFT_TAPE_MASK) || (ThisEvent.EventParam & FRONT_RIGHT_TAPE_MASK)) {
                        nextState = SaveTape;
                        PostRobotHSM(ThisEvent); //repost event for SaveTape state

                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }

                    break;
                case MANEUVER_OVER:
                    //escaped!
                    ThisEvent.EventType = ES_NO_EVENT;

                    ES_Event newEvent;
                    newEvent.EventType = ESCAPED_TAPE;
                    newEvent.EventParam = last_tape; // does this matter?

                    PostRobotHSM(newEvent);
                    break;


                default:
                    break;
            }
            break;


        default: // all unhandled states fall into here
            break;
    }
    // end switch on Current State

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunTapeSubState(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        RunTapeSubState(ENTRY_EVENT); // <- rename to your own Run function
    }

    ES_Tail(); // trace call stack end
    return ThisEvent;

}


//reset SM to desired init state upon exiting in top level (to renter this SM next time in correct init state)

int ResetTapeSubState(void) {
    CurrentState = SaveTape;
    return 1;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/


