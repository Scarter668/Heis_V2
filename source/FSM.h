/**
 * @file
 * @brief Headerfile for finite state machine
 */

#pragma once
#include "defines.h"
#include "driver/elevio.h"


/**
 * @brief Enum to hold the three different elevator directions.
 */
typedef enum ElevatorDirection{
    ElevatorDirectionDown = 0,
    ElevatorDirectionUp,
    ElevatorDirectionNeutral
} ElevatorDirection;


/**
 * @brief Defining enum whit all the different FSM states.
 */
typedef enum ElevatorState{
    ElevatorStateInit = 0,   //0
    ElevatorStateIdle,         //1
    ElevatorStateDown,          //2
    ElevatorStateUp,            //3
    ElevatorStateEmergency      //4
} ElevatorState;



/**
 * @brief Struct to hold the different variables required to operate the elevator
 */
typedef struct ElevatorVariables{
    bool door;
    bool obstruction;
    bool emergency_btn;
    ElevatorDirection direction;
    double floor_level;
} ElevatorVariables;

/**
 * @brief Initializes the finite state machine by setting all orders to 0,
 *        turning off all lamps, setting all elevatorvariables to 0/neutral.
 *        If @p floor_level is not defined, the elevator wil go up and stop
 *        at a floor where it is defined.
 */
void FSM_init();

/**
 * @brief Updates the finite-state machine by checking all the buttons,
 *        and then taking necessary action in accordance with the given elevator specifications.
 *        This includes setting the different button lights, all the logic (decision making),
 *        and changing the state. 
 */
void FSM_update();


