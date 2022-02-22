#include "driver/elevio.h"

#include "FSM.h"
#include "timer.h"


#define SHOULD_UP 1
#define SHOULD_STAY 0
#define SHOULD_DOWN -1
#define NO_DECISION -11





static ElevatorState m_elevator_state;
static ElevatorVariables m_elevator_variables;

bool m_elevator_buttons[N_BUTTONS][N_FLOORS];


void FSM_updateBtnsAndLights() {
    for ( int b = 0; b < N_BUTTONS; b++ ) {
        for ( int f = 0; f < N_FLOORS; f++ ) {
            if ( m_elevator_buttons[b][f] == 0 ) {
                m_elevator_buttons[b][f] = elevio_callButton(f,b);
            }
            
        }
    }
}




bool FSM_checkIfArrivedAtFloor() {
    // Stoppe motoren, åpne døren/start timer , slette bestillinger til floor, oppdatere floor level
    // Sjekke om bestilling til floor, ellers returner false

}


bool FSM_emergencyTrigger();


void FSM_init(){


    elevio_init();
    timer_init();

    for(int floor = 0; floor < N_FLOORS; floor++ ){
        
        for(int button = 0; button < N_BUTTONS; button++){

            m_elevator_buttons[button][floor] = 0;

        }
    }
    
    m_elevator_variables.door = 0;
    m_elevator_variables.obstruction = 0;
    m_elevator_variables.emergency_btn = 0;
    m_elevator_variables.direction = ElevatorDirectionNewtral;

    int floor = elevio_floorSensor();
    if( floor  == NO_FLOOR){
        elevio_motorDirection(DIRN_UP);
    }

        
    m_elevator_state = ElevatorStateInit;

}








int FSM_IdleRoutine_direction(){

        int decision = NO_DECISION;

        if(m_elevator_variables.direction == ElevatorDirectionNewtral){      
            for(int floor = 0; floor < N_FLOORS; floor++ ){

                for( int button = 0; button < N_BUTTONS; button++){

                    if(m_elevator_buttons[button][floor]){

                        if(m_elevator_variables.floor_level < (double) floor){

                            return decision = SHOULD_UP;
                            
                        } else if(m_elevator_variables.floor_level == (double) floor){

                            decision = SHOULD_STAY;

                        } else{ //m_elevator_variables.floor_level > (double) floor)

                            return SHOULD_DOWN;
                        }
                    
                    
                    }
                }
            }

        } else if( m_elevator_variables.direction == ElevatorDirectionUp){
            for(int floor = (int) m_elevator_variables.floor_level + 1 ; floor < N_FLOORS; floor++ ){

                for( int button = 0; button < N_BUTTONS; button++){
                    
                    
                    if(m_elevator_buttons[button][floor]   &&  (button == BUTTON_CAB ||  button == BUTTON_HALL_UP || floor == (N_FLOORS - 1) )){
                            
                        decision = SHOULD_UP;                       

                    }

                }
            }

        } else { // ElevatorDirectionDown

            for(int floor = (int) m_elevator_variables.floor_level -1 ; floor >= 0; floor-- ){

                for( int button = 0; button < N_BUTTONS; button++){
                    
                    
                    if(m_elevator_buttons[button][floor]   &&  (button == BUTTON_CAB ||  button == BUTTON_HALL_DOWN || floor == 0 )){
                            
                        decision = SHOULD_DOWN;                       

                    }

                }
            }


        }

    return decision;


}



void FSM_update(){

    m_elevator_variables.emergency_btn = elevio_stopButton();
    m_elevator_variables.obstruction = elevio_obstruction();

    switch(m_elevator_state)
    {
    case ElevatorStateInit:
        
        int floor = elevio_floorSensor();
        if( floor  != NO_FLOOR){
            elevio_motorDirection(DIRN_STOP);
            m_elevator_variables.floor_level = (double) floor;
            m_elevator_state = ElevatorStateIdle;
        }   

        break;


    case ElevatorStateIdle:

        FSM_updateBtnsAndLights();

        if(m_elevator_variables.emergency_btn){
            FSM_EmergencyEntry();
            m_elevator_state = ElevatorStateEmergency;
            break;
        }

        if(m_elevator_variables.obstruction && m_elevator_variables.door){
            timer_start();
        }


        if(timer_isActive() && timer_isTimeout()){

            FSM_setDoor(false);

        }

        if(m_elevator_variables.door == false){
            int decision = FSM_IdleRoutine_direction();

            switch(decision){
                case SHOULD_UP:
                    m_elevator_variables.direction = ElevatorDirectionUp;
                    m_elevator_state = ElevatorStateUp;
                    elevio_motorDirection(DIRN_UP);
                    break;

                case SHOULD_STAY:
                    m_elevator_variables.direction = ElevatorDirectionNewtral;
                    FSM_setDoors(true);

                    break;

                case SHOULD_DOWN:
                    m_elevator_variables.direction = ElevatorDirectionDown;
                    m_elevator_state = ElevatorStateDown;
                    elevio_motorDirection(DIRN_DOWN);

                    break;


                case NO_DECISION:
                    m_elevator_variables.direction = ElevatorDirectionNewtral;
                    break;

            }


        }   


        break;
    
    case ElevatorStateDown:

        FSM_updateBtnsAndLights();

        if(m_elevator_variables.emergency_btn){
            FSM_EmergencyEntry();
            m_elevator_state = ElevatorStateEmergency;
            break;
        }

        if(FSM_IdleTrigger()){
            
            FSM_IdleEntry();
            m_elevator_state = ElevatorStateIdle;
            break;
        }

        break;
    
    case ElevatorStateUp:

        FSM_updateBtnsAndLights();

        if(m_elevator_variables.emergency_btn){
            FSM_EmergencyEntry();
            m_elevator_state = ElevatorStateEmergency;
            break;
        }

        if(FSM_IdleTrigger()){
            
            FSM_IdleEntry();
            m_elevator_state = ElevatorStateIdle;
            break;
        }



        break;

    case ElevatorStateEmergency:
        
        if(m_elevator_variables.emergency_btn == false){
            m_elevator_state = ElevatorStateIdle;
            timer_start();
        }
        break;

        
    default:
        exit();
        break;
    }


}








