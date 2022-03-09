/* 
 * File:   TowerShootSubHSM.h
 * Author: zwang219
 *
 * Created on December 2, 2021, 6:33 PM
 */

#ifndef TOWERSHOOTSUBHSM_H
#define	TOWERSHOOTSUBHSM_H

#include "ES_Configure.h"
uint8_t InitTowerShootSubHSM(void);
ES_Event RunTowerShootSubHSM(ES_Event ThisEvent);
int ResetTowerShootSubHSM(void);

#endif	/* TOWERSHOOTSUBHSM_H */

