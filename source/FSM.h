/**
 * @file
 * @brief Headerfile for finite state machine
 */

#pragma once
#include "defines.h"
#include "driver/elevio.h"


typedef enum ElevatorDirection{
    ElevatorDirectionDown = 0,
    ElevatorDirectionUp,
    ElevatorDirectionNeutral
} ElevatorDirection;

typedef enum ElevatorState{
    ElevatorStateInit = 0,   //0
    ElevatorStateIdle,         //1
    ElevatorStateDown,          //2
    ElevatorStateUp,            //3
    ElevatorStateEmergency      //4
} ElevatorState;

typedef struct ElevatorVariables{
    bool door;
    bool obstruction;
    bool emergency_btn;
    ElevatorDirection direction;
    double floor_level;
} ElevatorVariables;

/**
 * @brief Initializes the finite-state machine
 */
void FSM_init();

/**
 * @brief Updates the finite-state machine
 */
void FSM_update();


