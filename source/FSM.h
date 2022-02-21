/**
 * @file
 * @brief Headerfile for finite state machine
 */

#pragma once
#include <defines.h>
#include <driver/elevio.h>


typedef enum ElevatorDirection{
    ElevatorDirectionDown = 0,
    ElevatorDirectionUp,
    ElevatorNoDirection
} ElevatorDirection;

typedef enum ElevatorState{
    ElevatorStateInit = 0,
    ElevatorStateIdle,
    ElevatorStateDown,
    ElevatorStateUp,
    ElevatorStateEmergency
} ElevatorState;

typedef struct ElevatorVariables{
    bool door;
    bool obstruction;
    bool emergency_btn;
    ElevatorDirection direction;
    double floor_level;
} ElevatorVariables;

void FSM_init();
void FSM_update();


