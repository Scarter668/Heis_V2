#include "driver/elevio.h"

#include "FSM.h"
#include "timer.h"




static ElevatorState m_elevator_state;
static ElevatorVariables m_elevator_variables;

bool m_elevator_buttons[N_BUTTONS][N_FLOORS];


void FSM_updateBtns();

bool FSM_emergencyStopRoutine_ifActivated();





bool FSM_checkIfArrivedAtFloor();





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








