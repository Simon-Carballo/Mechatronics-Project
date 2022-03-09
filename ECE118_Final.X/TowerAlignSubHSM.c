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
#include "TowerAlignSubHSM.h"
#include "Robot.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define TIMEOUT 20000

typedef enum {
    InitPSubState,
    Bump, //backup after bumping
    Adjust, //turn and bump again, trying to get more centered
    Aligned, //got front centered against tower and is now trying to turn 90 
    //degrees to line up side bumpers with tower
    Edge,
    Right,
    Check,

    //Traversal States, todo: move to new HSM
    LeftTurn,
    RightTurn,

} TowerAlignSubHSMState_t;

static const char *StateNames[] = {
	"InitPSubState",
	"Bump",
	"Adjust",
	"Aligned",
	"Edge",
	"Right",
	"Check",
	"LeftTurn",
	"RightTurn",
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

static TowerAlignSubHSMState_t CurrentState = InitPSubState; // <- change name to match ENUM
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
uint8_t InitTowerAlignSubHSM(void) {
    ES_Event returnEvent;

    CurrentState = InitPSubState;
    returnEvent = RunTowerAlignSubHSM(INIT_EVENT);
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
ES_Event RunTowerAlignSubHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    TowerAlignSubHSMState_t nextState; // <- change type to correct enum

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case InitPSubState: // If current state is initial Psedudo State
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
                // this is where you would put any actions associated with the
                // transition from the initial pseudo-state into the actual
                // initial state

                // now put the machine into the actual initial state
                nextState = Adjust;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;

        case Adjust:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case MANEUVER_OVER:

                    //todo: remove use of ==, change order of if statements to work with &
                    if ((FRONT_RIGHT_BMP_MASK | FRONT_LEFT_BMP_MASK) & last_bumped) { //todo: change to bitwise and and fix logic. leaving unchanged for now bc seems to be working
                        Robot_LeftMtrSpeed(85);
                        Robot_RightMtrSpeed(70);
                        nextState = Aligned;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else if ((FRONT_LEFT_BMP_MASK) & last_bumped) {
                        Robot_LeftMtrSpeed(80);
                        Robot_RightMtrSpeed(100);
                        nextState = Bump;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else if ((FRONT_RIGHT_BMP_MASK) & last_bumped) {
                        Robot_LeftMtrSpeed(100);
                        Robot_RightMtrSpeed(80);
                        nextState = Bump;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else {
                        Robot_LeftMtrSpeed(80);
                        Robot_RightMtrSpeed(70);
                        nextState = Bump;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }


                    break;

                default: // all unhandled states fall into here
                    break;
            }
            break;

        case Bump:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

                case BUMPERS_CHANGED:
                    Robot_LeftMtrSpeed(-100);
                    Robot_RightMtrSpeed(-100);
                    ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 350);

                    nextState = Adjust;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    last_bumped = ThisEvent.EventParam;
                    break;

                default:
                    break;
            }
            break;

        case Aligned:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case BUMPERS_CHANGED: // Initial Side Bump Detected
                    if ((SIDE_FRONT_BMP_MASK) == ThisEvent.EventParam) {
                        Robot_LeftMtrSpeed(0);
                        Robot_RightMtrSpeed(-100);
                        nextState = Edge;
                    } else {
                        Robot_LeftMtrSpeed(-60);
                        Robot_RightMtrSpeed(-90);
                        ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 250);
                        nextState = Right;
                    }
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                    
                case WAIT_OVER:
                    Robot_LeftMtrSpeed(0);
                    Robot_RightMtrSpeed(90);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

                default:
                    break;
            }
            break;

        case Right:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

                case MANEUVER_OVER:
                    Robot_LeftMtrSpeed(90);
                    Robot_RightMtrSpeed(60);
                    ES_Timer_InitTimer(WAIT_SERVICE_TIMER, 400);
                    nextState = Aligned;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

                default:
                    break;
            }
            break;

        case Edge:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

                case BUMPERS_CHANGED:
                    if ((SIDE_BACK_BMP_MASK) & ThisEvent.EventParam) {
                        Robot_LeftMtrSpeed(-60);
                        Robot_RightMtrSpeed(80);
                        nextState = Check;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }else if((BACK_LEFT_BMP_MASK) & ThisEvent.EventParam) {
                        Robot_LeftMtrSpeed(20);
                        Robot_RightMtrSpeed(90);
                        nextState = Check;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;

                case MANEUVER_OVER:
                    nextState = Check;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

                default:
                    break;
            }
            break;

        case Check:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    break;

                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

                case BUMPERS_CHANGED:
                    if (((SIDE_FRONT_BMP_MASK) & ThisEvent.EventParam)) {
                        Robot_LeftMtrSpeed(95);
                        Robot_RightMtrSpeed(-60);
                        ES_Event newEvent;
                        newEvent.EventType = BOT_ALIGNED;
                        newEvent.EventParam = 0;
                        PostRobotHSM(newEvent);
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else if ((FRONT_LEFT_BMP_MASK) & ThisEvent.EventParam) {
                        Robot_LeftMtrSpeed(0);
                        Robot_RightMtrSpeed(-90);
                        ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 200);
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                    
                case MANEUVER_OVER:
                    Robot_LeftMtrSpeed(80);
                    Robot_RightMtrSpeed(80);
                    ThisEvent.EventType = ES_NO_EVENT;
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
        RunTowerAlignSubHSM(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        RunTowerAlignSubHSM(ENTRY_EVENT); // <- rename to your own Run function
    }

    ES_Tail(); // trace call stack end
    return ThisEvent;

}


/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/


//reset SM to desired init state upon exiting in top level (to renter this SM next time in correct init state)
int ResetTowerAlignSubHSM(void){
    CurrentState = Adjust;
    
    return 1;
}
