/**
 * @file
 * @brief Implementation of funtions from FSM.h
 */

#include "FSM.h"
#include "timer.h"
#include <assert.h>

#include "stdio.h"


//Differnt defines for more readability
#define SHOULD_UP 1
#define SHOULD_STAY 0
#define SHOULD_DOWN -1
#define NO_DECISION -11


/**
 * @brief static global variabel for the state of elevator
 */
static ElevatorState m_elevator_state;
/**
 * @brief static global variabel for the ElevatorVariables struct
 */
static ElevatorVariables m_elevator_variables;

/**
 * @brief static array keeping all the buttons of the elevator
 *        Rows correspond to type of button: UP, DOWN, CAB
 *        Coloumns refer to the 0-indexed floor level: 0, 1, 2, 3
 */
static bool m_elevator_buttons[N_BUTTONS][N_FLOORS];

void printStates(){
    switch (m_elevator_state)
    {
    case ElevatorStateInit:
        printf("Elevator State:  Init\n");
        break;
    case ElevatorStateIdle:
        printf("Elevator State:  Idle\n");
        break;
    case ElevatorStateDown:
        printf("Elevator State:  Down\n");
        break;
    case ElevatorStateUp:
        printf("Elevator State:  Up\n");
        break;

    case ElevatorStateEmergency:
        printf("Elevator State:  Emergency\n");
        break;
    default:
        break;
    }
    printf("Door state:\t%d\n", m_elevator_variables.door);
    printf("Obstruction state:\t%d\n", m_elevator_variables.obstruction);
    printf("Emergency state:\t%d\n", m_elevator_variables.emergency_btn);
    printf("Door state:\t%d\n", m_elevator_variables.door);
    switch (m_elevator_variables.direction)
    {
    case ElevatorDirectionDown:
        printf("Elevator Direction:  Down\n");
        break;
    case ElevatorDirectionUp:
        printf("Elevator Direction:  Up\n");
        break;
    case ElevatorDirectionNeutral:
        printf("Elevator Direction:  Newtral\n");
        break;
    default:
        break;
    }
    printf("Floor level:\t%f\n\n", m_elevator_variables.floor_level);
}


void FSM_init(){
    for (int floor = 0; floor < N_FLOORS; floor++){
        for (int button = 0; button < N_BUTTONS; button++){
            m_elevator_buttons[button][floor] = false;
            elevio_buttonLamp(floor, button, false);
        }
    }

    m_elevator_variables.door = 0;
    m_elevator_variables.obstruction = 0;
    m_elevator_variables.emergency_btn = 0;
    m_elevator_variables.direction = ElevatorDirectionNeutral;
    int floor = elevio_floorSensor();

    if (floor == NO_FLOOR) elevio_motorDirection(DIRN_UP);

    m_elevator_state = ElevatorStateInit;
    elevio_stopLamp(false);
}


/**
 * @brief Updates @p m_elevator_buttons array with the new values from elevio.
 *        Only updates if there is not an active order. Also sets the corresponding 
 *        buttonlights according to the array.  
 */
void FSM_updateBtnsAndLights(){
    bool var_elevio_callButton;
    for (int button = 0; button < N_BUTTONS; button++){
        for (int floor = 0; floor < N_FLOORS; floor++){ 
            var_elevio_callButton = elevio_callButton(floor, button); 
            if (var_elevio_callButton){
                m_elevator_buttons[button][floor] = var_elevio_callButton;
                elevio_buttonLamp(floor, button, true);
            }
        }
    }
}

bool FSM_orderAbovefloor(int floor){
    for (int f = ++floor; f < N_FLOORS; f++){
        for (int button = 0; button < N_BUTTONS; button++){
            if (m_elevator_buttons[button][f]) return true;
        }
    }
    return false;
}

bool FSM_orderBelowfloor(int floor){
    for (int f = --floor; f >= 0; f--){
        for (int button = 0; button < N_BUTTONS; button++){
            if (m_elevator_buttons[button][f]) return true;
        }
    }
    return false;
}

void FSM_setDoor(bool open){
    
    if (open){// door = true, turn off light, start timer
        m_elevator_variables.door = true;
        elevio_doorOpenLamp(true);
        timer_start();
    
    } else{ // door = false, turn on light
        m_elevator_variables.door = false;
        elevio_doorOpenLamp(false);
    }
}


/**
 * @brief Checks if the elevator should stop and to enter an Idle state.
 *        Is elevator on a floor with orders.
 *        If not and on a floor, update the @p floor_level variable to floor pluss/minus 0.5
 *        depending on the direction.
 *    
 * @return @c true or @c false depending on whether the idle entry condition are met.
 */
bool FSM_IdleTrigger(){
    // check if we are on a floor with orders
    // if not -> Update floor_level to floor pluss/minus 0.5
    assert(m_elevator_variables.direction != ElevatorDirectionNeutral);
    int elevio_floor = elevio_floorSensor();

    if (elevio_floor != NO_FLOOR){ // if on floor
        elevio_floorIndicator(elevio_floor);
        if ((m_elevator_variables.direction == ElevatorDirectionUp 
                    && (m_elevator_buttons[BUTTON_HALL_UP][elevio_floor] 
                            || !FSM_orderAbovefloor(elevio_floor))) 

            || (m_elevator_buttons[BUTTON_CAB][elevio_floor]) 

            || (m_elevator_variables.direction == ElevatorDirectionDown 
                    && (m_elevator_buttons[BUTTON_HALL_DOWN][elevio_floor] 
                            || !FSM_orderBelowfloor(elevio_floor))))
        {
            // if on floor with order with same direction as elevator or furthest order in that diretion
            m_elevator_variables.floor_level = (double)elevio_floor;
            return true;         
        }
        
        // if condition above are not met
        if (m_elevator_variables.direction == ElevatorDirectionUp){
            
            m_elevator_variables.floor_level = (double)elevio_floor + 0.5;
        
        } else { // direction == down
            m_elevator_variables.floor_level = (double)elevio_floor - 0.5;
        }
    }
    return false;
}


/**
* @brief Completes the routine that should be done everytime the elevator enter Idle state.
*        That is to stop the motor, open the door, and remove orders on the floor
*        and turn off their corresponding lights
*/
void FSM_IdleEntry(){
    elevio_motorDirection(DIRN_STOP);
    FSM_setDoor(true);
    for (int button = 0; button < N_BUTTONS; button++){
        m_elevator_buttons[button][(int)m_elevator_variables.floor_level] = false;
        int floor = (int)m_elevator_variables.floor_level;
        elevio_buttonLamp(floor, button, false);
    }
}

/**
* @brief Completes the routine that should be done whenever emergency state is entered.
*        Thid includes stoping the motor, turnin on the stoplamp, deleting all orders in the @p m_elevator_buttons array.
*        If the elevator is on a floor, the door is also be opened.
*/
void FSM_EmergencyEntry(){
    elevio_motorDirection(DIRN_STOP);
    elevio_stopLamp(true);
    if (elevio_floorSensor() != NO_FLOOR){
        FSM_setDoor(true);
    }
    for (int button = 0; button < N_BUTTONS; button++){
        for (int floor = 0; floor < N_FLOORS; floor++){
            m_elevator_buttons[button][floor] = false;
            elevio_buttonLamp(floor, button, false);
        }
    }
}

/**
 * @brief Generates correct decision for elevator to be used in Idle state when makin a decition of what to do next.
 *        This loops throught the @p m_elevator_buttons array differently depending on what direction the elevator has.
 *        If the elevator has a direction up, it loops through the upper part of the array, and vice versa for direction down.
 *        If the elevator is in a nuetral direction, it loops through the array from the first floor and up (and return as soon as an order is found unless it is the floor you are).
 *        This means that floor lower floor levels will be prioritized.   
 *
 * @return @c SHOULD_UP , @c SHOULD_STAY , @c SHOULD_DOWN or @c NO_DECISION depending on the current orders.
 */
int FSM_IdleRoutine_decision(){

    int decision = NO_DECISION;
    switch (m_elevator_variables.direction){
    case ElevatorDirectionNeutral:
        for (int floor = 0; floor < N_FLOORS; floor++){
            for (int button = 0; button < N_BUTTONS; button++){
                if (m_elevator_buttons[button][floor]){

                    if (m_elevator_variables.floor_level < (double)floor){
                        return SHOULD_UP;
                    
                    } else if (m_elevator_variables.floor_level == (double)floor){
                        decision = SHOULD_STAY;
                    
                    } else{ // m_elevator_variables.floor_level > (double) floor)
                        return SHOULD_DOWN;
                    }
                }
            }
        }
        break;
    case ElevatorDirectionUp:
        for (int floor = (int)m_elevator_variables.floor_level + 1; floor < N_FLOORS; floor++){
            for (int button = 0; button < N_BUTTONS; button++){
                if (m_elevator_buttons[button][floor] && (button == BUTTON_CAB || button == BUTTON_HALL_UP || floor == (N_FLOORS - 1))){
                    decision = SHOULD_UP;
                }
            }
        }
        break;
    case ElevatorDirectionDown:
        for (int floor = (int)m_elevator_variables.floor_level - 1; floor >= 0; floor--){
            for (int button = 0; button < N_BUTTONS; button++){
                if (m_elevator_buttons[button][floor] && (button == BUTTON_CAB || button == BUTTON_HALL_DOWN || floor == 0)){
                    decision = SHOULD_DOWN;
                }
            }
        }
        break;

    default:
        break;
    }

    return decision;
}

void FSM_update(){

    m_elevator_variables.emergency_btn = elevio_stopButton();
    m_elevator_variables.obstruction = elevio_obstruction();

    switch (m_elevator_state){
        case ElevatorStateInit:
        {
            int floor = elevio_floorSensor();
            if (floor != NO_FLOOR){
                elevio_motorDirection(DIRN_STOP);
                m_elevator_variables.floor_level = (double)floor;
                m_elevator_state = ElevatorStateIdle;
                elevio_floorIndicator(floor);
            }
        } break;
        case ElevatorStateIdle:

            FSM_updateBtnsAndLights();
            if (m_elevator_variables.emergency_btn){
                FSM_EmergencyEntry();
                m_elevator_state = ElevatorStateEmergency;
                break;
            }

            if (m_elevator_variables.obstruction && m_elevator_variables.door) timer_start();

            if (timer_isActive() && timer_isTimeout()) FSM_setDoor(false);

            if (m_elevator_variables.door == false){

                int decision = FSM_IdleRoutine_decision();
                switch (decision){
                case SHOULD_UP:
                    m_elevator_variables.direction = ElevatorDirectionUp;
                    m_elevator_state = ElevatorStateUp;
                    elevio_motorDirection(DIRN_UP);
                    break;
                case SHOULD_STAY:
                    m_elevator_variables.direction = ElevatorDirectionNeutral;
                    FSM_setDoor(true);
                    break;
                case SHOULD_DOWN:
                    m_elevator_variables.direction = ElevatorDirectionDown;
                    m_elevator_state = ElevatorStateDown;
                    elevio_motorDirection(DIRN_DOWN);
                    break;
                case NO_DECISION:
                    m_elevator_variables.direction = ElevatorDirectionNeutral;
                    break;
                }
            } else { //close remove all orders in this floor 
                for (int button = 0; button < N_BUTTONS; button++){
                    m_elevator_buttons[button][(int)m_elevator_variables.floor_level] = false;
                    int floor = (int)m_elevator_variables.floor_level;
                    elevio_buttonLamp(floor, button, false);
                }
            }
            break;
        case ElevatorStateDown:

            FSM_updateBtnsAndLights();
            if (m_elevator_variables.emergency_btn){
                FSM_EmergencyEntry();
                m_elevator_state = ElevatorStateEmergency;
                break;
            }
            if (FSM_IdleTrigger()){
                FSM_IdleEntry();
                m_elevator_state = ElevatorStateIdle;
                break;
            }
            break;
        case ElevatorStateUp:

            FSM_updateBtnsAndLights();
            if (m_elevator_variables.emergency_btn){
                FSM_EmergencyEntry();
                m_elevator_state = ElevatorStateEmergency;
                break;
            }
            if (FSM_IdleTrigger()){
                FSM_IdleEntry();
                m_elevator_state = ElevatorStateIdle;
                break;
            }
            break;
        case ElevatorStateEmergency:

            if (m_elevator_variables.emergency_btn == false){
                m_elevator_state = ElevatorStateIdle;
                elevio_stopLamp(false);
                m_elevator_variables.direction = ElevatorDirectionNeutral;
                timer_start();
            }
            break;
        default:
            break;
        }

    printStates();
}
