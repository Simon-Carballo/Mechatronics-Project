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
#include "TraverseSubHSM.h"
#include "Robot.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define CORNERTIME 2000
#define LOSTTIME 10000

typedef enum {
    InitPSubState,
    Straight, //Both Side Bumpers Pressed and active side 
    LeftTurn, //Turn left when Front Side Bumper is lost
    RightTurn, //Turn Right when Back Side Bumper is lost 
    Corner, //Hard left Turn when Both Bumpers & Timeout
} AtTowerSubHSMState_t;

static const char *StateNames[] = {
	"InitPSubState",
	"Straight",
	"LeftTurn",
	"RightTurn",
	"Corner",
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

static AtTowerSubHSMState_t CurrentState = InitPSubState; // <- change name to match ENUM
static uint8_t MyPriority;
static uint8_t last_bumped = 0x0;

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
uint8_t InitTraverseSubHSM(void) {
    ES_Event returnEvent;

    CurrentState = InitPSubState;
    returnEvent = RunTraverseSubHSM(INIT_EVENT);
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
ES_Event RunTraverseSubHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    AtTowerSubHSMState_t nextState; // <- change type to correct enum

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case InitPSubState: // If current state is initial Psedudo State
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
                // this is where you would put any actions associated with the
                // transition from the initial pseudo-state into the actual
                // initial state

                // now put the machine into the actual initial state
                nextState = Straight;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;

        case Straight:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case BUMPERS_CHANGED:

                    if (!((SIDE_FRONT_BMP_MASK) & ThisEvent.EventParam)) {
                        Robot_LeftMtrSpeed(60);
                        Robot_RightMtrSpeed(80);
                        ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, CORNERTIME);
                        nextState = LeftTurn;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        
                    } else if (!((SIDE_BACK_BMP_MASK) & ThisEvent.EventParam)) {
                        Robot_LeftMtrSpeed(80);
                        Robot_RightMtrSpeed(60);
                        nextState = RightTurn;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        
                    } else if (((SIDE_BACK_BMP_MASK) & ThisEvent.EventParam) && ((SIDE_FRONT_BMP_MASK) & ThisEvent.EventParam)) {
                        Robot_LeftMtrSpeed(80);
                        Robot_RightMtrSpeed(80);
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;

                default: // all unhandled states fall into here
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

                case BUMPERS_CHANGED:
                    if (((SIDE_BACK_BMP_MASK) & ThisEvent.EventParam) && ((SIDE_FRONT_BMP_MASK) & ThisEvent.EventParam)) {
                        Robot_LeftMtrSpeed(80);
                        Robot_RightMtrSpeed(80);
                        nextState = Straight;
                        ThisEvent.EventType = ES_NO_EVENT;
                        
                    }else if (!((SIDE_FRONT_BMP_MASK) & ThisEvent.EventParam)) {
                        Robot_LeftMtrSpeed(60);
                        Robot_RightMtrSpeed(80);  
                        ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, CORNERTIME);
                        nextState = LeftTurn;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;

                default:
                    break;
            }
            break;

        case LeftTurn:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    break;
                    
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case BUMPERS_CHANGED:
                    
                    if (((SIDE_BACK_BMP_MASK) & ThisEvent.EventParam) && ((SIDE_FRONT_BMP_MASK) & ThisEvent.EventParam)) {
                        Robot_LeftMtrSpeed(80);
                        Robot_RightMtrSpeed(80);
                        nextState = Straight;
                        ThisEvent.EventType = ES_NO_EVENT;
                        
                    }else if (!((SIDE_BACK_BMP_MASK) & ThisEvent.EventParam)) {
                        Robot_LeftMtrSpeed(80);
                        Robot_RightMtrSpeed(60);
                        nextState = RightTurn;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                    
                case MANEUVER_OVER:
                    if (!((SIDE_BACK_BMP_MASK) & ThisEvent.EventParam) && ((SIDE_FRONT_BMP_MASK) & ThisEvent.EventParam)) {
                        Robot_LeftMtrSpeed(10);
                        Robot_RightMtrSpeed(100);
                        nextState = Corner;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
//                    else{
//                        ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, CORNERTIME);
//                        ThisEvent.EventType = ES_NO_EVENT;
//                    }
                    break;
                    
                default:
                    break;
            }
            break;

        case Corner:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

                case BUMPERS_CHANGED:
                    if ((SIDE_FRONT_BMP_MASK) & ThisEvent.EventParam) {
                        nextState = LeftTurn;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;

                default:
                    break;
            }
            break;

        default: // all unhandled states fall into here
            break;
    }

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunTraverseSubHSM(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        RunTraverseSubHSM(ENTRY_EVENT); // <- rename to your own Run function
    }

    ES_Tail(); // trace call stack end
    return ThisEvent;

}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/

