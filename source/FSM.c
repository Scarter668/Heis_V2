#include "driver/elevio.h"

#include "FSM.h"
#include "timer.h"
#include <assert.h>

#include "stdio.h"

#define SHOULD_UP 1
#define SHOULD_STAY 0
#define SHOULD_DOWN -1
#define NO_DECISION -11

static ElevatorState m_elevator_state;
static ElevatorVariables m_elevator_variables;

bool m_elevator_buttons[N_BUTTONS][N_FLOORS];

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

void FSM_updateBtnsAndLights(){
    bool var_elevio_callButton;
    for (int button = 0; button < N_BUTTONS; button++){
        for (int floor = 0; floor < N_FLOORS; floor++){ 
            var_elevio_callButton = elevio_callButton(floor, button); //går bra å sjekke knapper som ikke finnes da elevio returnerer 0 uansett
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
    // door = true, sette lyset, starte timer
    if (open){
        m_elevator_variables.door = true;
        elevio_doorOpenLamp(true);
        timer_start();
    
    } else{
        m_elevator_variables.door = false;
        elevio_doorOpenLamp(false);
    }
}
// Rader: UP, DOWN, CAB
// Kolonner: 0, 1, 2, 3

bool FSM_IdleTrigger(){
    // Sjekke om vi er på floor med orders
    // Hvis ikke -> Oppdatere floor level til floor pluss/minus en halv
    assert(m_elevator_variables.direction != ElevatorDirectionNeutral);
    int elevio_floor = elevio_floorSensor();
    if (elevio_floor != NO_FLOOR){ // hvis floor
        elevio_floorIndicator(elevio_floor);
        if ((m_elevator_variables.direction == ElevatorDirectionUp 
                    && (m_elevator_buttons[BUTTON_HALL_UP][elevio_floor] 
                            || !FSM_orderAbovefloor(elevio_floor))) 

            || (m_elevator_buttons[BUTTON_CAB][elevio_floor]) 

            || (m_elevator_variables.direction == ElevatorDirectionDown 
                    && (m_elevator_buttons[BUTTON_HALL_DOWN][elevio_floor] 
                            || !FSM_orderBelowfloor(elevio_floor))))
        {
            // // hvis floor med bestilling med samme retning
            m_elevator_variables.floor_level = (double)elevio_floor;
            return true; // kjører videre arrivedAtRequestedFloorRoutine()
        }
        
        // hvis ikke floor med bestilling i samme retning
        if (m_elevator_variables.direction == ElevatorDirectionUp){
            
            m_elevator_variables.floor_level = (double)elevio_floor + 0.5;
        
        } else { // direction == down
            m_elevator_variables.floor_level = (double)elevio_floor - 0.5;
        }
    }
    return false;
}

void FSM_IdleEntry(){
    //  Stopp motoren, åpne dør/start timer, slette bestillinger til floor
    elevio_motorDirection(DIRN_STOP);
    FSM_setDoor(true);
    for (int button = 0; button < N_BUTTONS; button++){
        m_elevator_buttons[button][(int)m_elevator_variables.floor_level] = false;
        int floor = (int)m_elevator_variables.floor_level;
        elevio_buttonLamp(floor, button, false);
    }
}

void FSM_EmergencyEntry(){
    // stopp motoren, skru på stopp-lys, åpne dørene hvis etasje, slett alle bestillinger
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
 * @brief Generates correct decision for elevator.
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
