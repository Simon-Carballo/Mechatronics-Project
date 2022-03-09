/* 
 * File:   robot_services.h
 * Author: achemish
 *
 * Created on November 29, 2021, 10:02 PM
 */

#ifndef ROBOT_SERVICES_H
#define	ROBOT_SERVICES_H

#include "robot.h"




//bumpers
uint8_t InitBumperDebounceService(uint8_t Priority);
uint8_t PostBumperDebounceService(ES_Event ThisEvent);
ES_Event RunBumperDebounceService(ES_Event ThisEvent);



//track wire detector
//uint8_t InitTrackWireService(uint8_t Priority);
//uint8_t PostTrackWireService(ES_Event ThisEvent);
//ES_Event RunTrackWireService(ES_Event ThisEvent);

//beacon detector
//uint8_t InitBeaconService(uint8_t Priority);
//uint8_t PostBeaconService(ES_Event ThisEvent);
//ES_Event RunBeaconService(ES_Event ThisEvent);

//maneuver timer
uint8_t InitManeuverService(uint8_t Priority); //Handles maneuver timer
uint8_t PostManeuverService(ES_Event ThisEvent); //Handles maneuver timer
ES_Event RunManeuverService(ES_Event ThisEvent); //Handles maneuver timer


//wait timer
uint8_t InitWaitService(uint8_t Priority);
uint8_t PostWaitService(ES_Event ThisEvent); 
ES_Event RunWaitService(ES_Event ThisEvent); 


//lost timer
uint8_t InitLostService(uint8_t Priority); 
uint8_t PostLostService(ES_Event ThisEvent); 
ES_Event RunLostService(ES_Event ThisEvent); 


//temp timer
uint8_t InitTempService(uint8_t Priority); 
uint8_t PostTempService(ES_Event ThisEvent); 
ES_Event RunTempService(ES_Event ThisEvent); 

#endif