#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "RobotHSM.h"
#include "TowerShootSubHSM.h"
#include "Robot.h"

#define SHOOT_INITSTATE Scoring

typedef enum {
    InitPSubState,
    Align,
    Position,
    Scoring,
    Halt,
    Jiggle,
} TowerShootSub_t;

static const char *StateNames[] = {
    "InitPSubState",
    "Align",
    "Position",
    "Scoring",
    "Halt",
    "Jiggle",
};



/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine */
void SlowRight(void);
void SlowLeft(void);
void Backwards(void);
void Forward(void);
/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
/* You will need MyPriority and the state variable; you may need others as well.
 * The type of state variable should match that of enum in header file. */

static TowerShootSub_t CurrentState = InitPSubState; // <- change name to match ENUM
static uint8_t MyPriority;
static uint8_t last_tape = 0x0;
static int jig = 0;


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
uint8_t InitTowerShootSubHSM(void) {
    ES_Event returnEvent;

    CurrentState = InitPSubState;
    returnEvent = RunTowerShootSubHSM(INIT_EVENT);
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
ES_Event RunTowerShootSubHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    TowerShootSub_t nextState; // <- change type to correct enum

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
                nextState = SHOOT_INITSTATE;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
            
        case Align:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                    
                case BUMPERS_CHANGED:
                    if(!((SIDE_FRONT_BMP_MASK)& ThisEvent.EventParam) && !((SIDE_BACK_BMP_MASK) & ThisEvent.EventParam)){  
                        ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 300);
                        Backwards();
                        nextState = Position;
                        makeTransition = TRUE;
                    }
                    else if (!((SIDE_FRONT_BMP_MASK) & ThisEvent.EventParam)) {
                        SlowRight();      
                    }else if (!((SIDE_BACK_BMP_MASK) & ThisEvent.EventParam)) {
                        SlowLeft();
                    }else if (((SIDE_BACK_BMP_MASK) & ThisEvent.EventParam) && ((SIDE_FRONT_BMP_MASK) & ThisEvent.EventParam)) {
                        Backwards();
                    }
                    
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                
//                case TAPE_CHANGED:
//                    Forward();
//                    nextState = Scoring;
//                    makeTransition = TRUE;
//                    ThisEvent.EventType = ES_NO_EVENT;
//                    break;
//                
                default: // all unhandled states fall into here
                    break;
            }
            break;
            
        case Position:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                
                case BUMPERS_CHANGED:
                    if ((SIDE_FRONT_BMP_MASK) & ThisEvent.EventParam){
                        SlowLeft();
                        makeTransition = TRUE;
                        nextState = Align;
                    }else if ((SIDE_BACK_BMP_MASK) & ThisEvent.EventParam) {
                        SlowRight();
                        makeTransition = TRUE;
                        nextState = Align; 
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                    
                case MANEUVER_OVER:
                    Forward();
                    ES_Timer_InitTimer(WAIT_SERVICE_TIMER, 500);
                    ThisEvent.EventType = ES_NO_EVENT;
                    
                case WAIT_OVER:
                    nextState = Scoring;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    
                default:
                    break;
            }
            break;

        case Scoring:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                    
                case BUMPERS_CHANGED:
                    if (((SIDE_BACK_BMP_MASK) & ThisEvent.EventParam) && ((SIDE_FRONT_BMP_MASK) & ThisEvent.EventParam)) {
                        Forward();
                    }
                    else if (!((SIDE_FRONT_BMP_MASK) & ThisEvent.EventParam)){
                        // Slight Left
                        Robot_LeftMtrSpeed(75);
                        Robot_RightMtrSpeed(90);
                        
                    }else if (!((SIDE_BACK_BMP_MASK) & ThisEvent.EventParam)) {
                        // Slight Right
                        Robot_LeftMtrSpeed(90);
                        Robot_RightMtrSpeed(70);
                    }

                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                    
                    
                case TAPE_CHANGED:
                    if ((ThisEvent.EventParam & (SIDE_BACK_TAPE_MASK)) && (ThisEvent.EventParam & (SIDE_FRONT_TAPE_MASK))) {
                        Robot_LeftMtrSpeed(0);
                        Robot_RightMtrSpeed(0);
                        ES_Timer_InitTimer(WAIT_SERVICE_TIMER, 800);
                        ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 2500);
                        nextState = Halt;
                        makeTransition = TRUE;
                    }else if (ThisEvent.EventParam & (SIDE_FRONT_TAPE_MASK)) {
                        Robot_LeftMtrSpeed(80);
                        Robot_RightMtrSpeed(80);
                    }else if (ThisEvent.EventParam & (SIDE_BACK_TAPE_MASK)) {
                        Robot_LeftMtrSpeed(-70);
                        Robot_RightMtrSpeed(-70);
                    }
                    ThisEvent.EventType = ES_NO_EVENT; 
                    break;
                    
                default: // all unhandled states fall into here;
                    break;
            }
            break;

        case Halt:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

                case MANEUVER_OVER:
                    Robot_LeftMtrSpeed(-90);
                    Robot_RightMtrSpeed(-90);
                    ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 150);
                    nextState = Jiggle;
                    jig++;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
//                    ThisEvent.EventType = ES_NO_EVENT;
//                    ES_Event newEvent;
//                    newEvent.EventType = BALL_DEPOSITED;
//                    newEvent.EventParam = 0;
//                    PostRobotHSM(newEvent);
                    break;

                case WAIT_OVER:
                    Robot_SolenoidPopBall();
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                    
                default: // all unhandled states fall into here;
                    break;
            }
            break;

        case Jiggle:
            switch (ThisEvent.EventType) {
                case ES_NO_EVENT:
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

                case MANEUVER_OVER:
                    if (jig == 3) {
                        ThisEvent.EventType = ES_NO_EVENT;
                        ES_Event newEvent;
                        newEvent.EventType = BALL_DEPOSITED;
                        newEvent.EventParam = 0;
                        PostRobotHSM(newEvent);
                        jig = 0;
                    } else if (jig % 2 == 0){
                        Robot_LeftMtrSpeed(-80);
                        Robot_RightMtrSpeed(-80);
                        ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 150);
                        jig++;
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else {
                        Robot_LeftMtrSpeed(80);
                        Robot_RightMtrSpeed(80);
                        ES_Timer_InitTimer(MANEUVER_SERVICE_TIMER, 150);
                        jig++;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;

                default: // all unhandled states fall into here;
                    break;
            }
            break;




        default: // all unhandled states fall into here;
            break;


    }
    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunTowerShootSubHSM(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        RunTowerShootSubHSM(ENTRY_EVENT); // <- rename to your own Run function
    }

    ES_Tail(); // trace call stack end
    return ThisEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/

void SlowRight(void){
    Robot_LeftMtrSpeed(-60);
    Robot_RightMtrSpeed(-85);
}

void SlowLeft(void){
    Robot_LeftMtrSpeed(-85);
    Robot_RightMtrSpeed(-60);
}

void Backwards(void){
    Robot_LeftMtrSpeed(-85);
    Robot_RightMtrSpeed(-85);
}

void Forward(void) {
    Robot_LeftMtrSpeed(83);
    Robot_RightMtrSpeed(83);
}


//reset SM to desired init state upon exiting in top level (to renter this SM next time in correct init state)
int ResetTowerShootSubHSM(void){
    CurrentState = SHOOT_INITSTATE;
    return 1;
}
