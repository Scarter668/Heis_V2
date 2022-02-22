#include "driver/elevio.h"

#include "FSM.h"
#include "timer.h"
#include <assert.h>




static ElevatorState m_elevator_state;
static ElevatorVariables m_elevator_variables;

bool m_elevator_buttons[N_BUTTONS][N_FLOORS];


void FSM_updateBtnsAndLights() {
    bool var_elevio_callButton;
    for ( int button = 0; button < N_BUTTONS; button++ ) {
        for ( int floor = 0; floor < N_FLOORS; floor++ ) {
            var_elevio_callButton = elevio_callButton(floor, button);
            if ( var_elevio_callButton ) {
                m_elevator_buttons[button][floor] = var_elevio_callButton;
                elevio_buttonLamp(floor, button, true);
            }
        }
    }
}





// Rader: UP, DOWN, CAB
// Kolonner: 0, 1, 2, 3

bool FSM_IdleTrigger() {
    // Sjekke om vi er på floor med orders
    // Hvis ikke -> Oppdatere floor level til floor pluss/minus en halv
    assert( m_elevator_variables.direction != ElevatorDirectionNeutral );
    int elevio_floor = elevio_floorSensor();
    if ( elevio_floor != NO_FLOOR ) { // hvis floor 
        elevio_floorIndicator( (int) elevio_floor );
        if ( (m_elevator_variables.direction == ElevatorDirectionUp && m_elevator_buttons[BUTTON_HALL_UP][elevio_floor])
            || (m_elevator_buttons[BUTTON_CAB][elevio_floor]) 
            || (m_elevator_variables.direction == ElevatorDirectionDown && m_elevator_buttons[BUTTON_HALL_DOWN][elevio_floor])) {
                // hvis floor med bestilling med samme retning
                m_elevator_variables.floor_level = (double) elevio_floor;
                return true; // kjører videre arrivedAtRequestedFloorRoutine()
        }
        // hvis ikke floor med bestilling i samme retning
        if ( m_elevator_variables.direction == ElevatorDirectionUp ) {
            m_elevator_variables.floor_level = (double) elevio_floor + 0.5;
        } else { // direction == down
            m_elevator_variables.floor_level = (double) elevio_floor - 0.5;
        }
    }
    return false;
}

void FSM_setDoor(bool b) {
    // door = true, sette lyset, starte timer
    if ( b ) {
        m_elevator_variables.door = true;
        elevio_doorOpenLamp(true);
        timer_start();
    } else {
        m_elevator_variables.door = false;
        elevio_doorOpenLamp(false);
    }
}


void FSM_IdleEntry() {
    // Stopp motoren, åpne dør/start timer, slette bestillinger til floor
    elevio_motorDirection(DIRN_STOP);
    elevio_doorOpenLamp(m_elevator_variables.floor_level);
    for ( int button = 0; button < N_BUTTONS; button++ ) {
        m_elevator_buttons[button][(int) m_elevator_variables.floor_level] = false;
        elevio_buttonLamp((int) m_elevator_variables.floor_level, button, false);
    }

}


bool FSM_EmergencyEntry() {
    // stopp motoren, skru på stopp-lys, åpne dørene hvis etasje
    elevio_motorDirection(DIRN_STOP);
    elevio_stopLamp(true);
    if ( elevio_floorSensor() != NO_FLOOR ) {
        FSM_setDoor(true);
    }
}


void FSM_init(){


    elevio_init();
    timer_init();

    for(int floor = 0; floor < N_FLOORS; floor++ ){
        
        for(int button = 0; button < N_BUTTONS; button++){

            m_elevator_buttons[button][floor] = 0;

        }
    }
    
    m_elevator_buttons.door = 0;
    m_elevator_buttons.obstruction = 0;
    m_elevator_buttons.emergency_btn = 0;
    m_elevator_buttons.direction = ElevatorNoDirection;

    int floor = elevio_floorSensor();
    if( floor  == NO_FLOOR){
        elevio_motorDirection(DIRN_UP);
    }

        
    m_elevator_state = ElevatorStateInit;

}








void FSM_update(){


    FSM_updateBtns();


    switch(m_elevator_state)
    {
    case ElevatorStateInit:
        
        int floor = elevio_floorSensor();
        if( floor  != NO_FLOOR){
            elevio_motorDirection(DIRN_STOP);
            m_elevator_buttons.floor_level = (double) floor;
            m_elevator_state = ElevatorStateIdle;
        }   

        break;

    case ElevatorStateIdle:

        if(FSM_emergencyStopRoutine_ifActivated()){
            m_elevator_state = ElevatorStateEmergency;
            break;
        }

        if(FSM_checkIfArrivedAtFloor()){
            m_elevator_state = ElevatorStateIdle;
            break;
        }




        break;
    
    case ElevatorStateDown:
        if(FSM_emergencyStopRoutine_ifActivated()){
            m_elevator_state = ElevatorStateEmergency;
            break;
        }

        if(FSM_checkIfArrivedAtFloor()){
            m_elevator_state = ElevatorStateIdle;
            break;
        }

        break;
    
    case ElevatorStateUp:
        if(FSM_emergencyStopRoutine_ifActivated()){
            m_elevator_state = ElevatorStateEmergency;
            break;
        }

        break;

    case ElevatorStateEmergency:
        
        break;

        
    default:
        exit();
        break;
    }


}








