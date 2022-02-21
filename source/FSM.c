
#include "driver/elevio.h"

#include "FSM.h"
#include "timer.h"




static ElevatorState m_elevator_state;
static ElevatorVariables m_elevator_variables;

bool m_elevator_buttons[N_BUTTONS][N_FLOORS];


void FSM_updateBtns();

void FSM_emergencyStopRoutine_ifActivated();



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
        break;
    
    case ElevatorStateDown:
        break;
    
    case ElevatorStateUp:
        break;

    case ElevatorStateEmergency:
        break;

        
    default:
    exit();
        break;
    }


}








