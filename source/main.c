/**
 * @file
 * @brief main program of the application
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "driver/elevio.h"

#include "timer.h"
#include "FSM.h"
#include "defines.h"

//to test
#include "queue.h"







static bool stop = false;

int main(){
    
    FSM_init();
    
    printf("=== Example Program ===\n");
    printf("Press the stop button on the elevator panel to exit\n");

    //elevio_motorDirection(DIRN_UP);


    
    // Button_t b;
    // b.floor_level = 1;
    // b.button_type = BUTTON_CAB;
    // //printf("Val1 %d \n", b.floor_level_from);

    // queue_addOrder(&b);
    // b.floor_level = 2;
    // //printf("Val2 %d \n", b.floor_level_from);

    // queue_addOrder(&b);

    // b.floor_level = 3;
    // queue_addOrder(&b);

    // b.floor_level= 1;
    // queue_addOrder(&b);
    // //printf("Val3 %d \n", b.floor_level_from);


    int floor, stopBtn, ObsBtn;
    while(1){
        floor = elevio_floorSensor();
        //printf("floor: %d \n",floor);

        // if(floor == 0){
        //     elevio_motorDirection(DIRN_UP);
        // }

        // if(floor == N_FLOORS-1){
        //     elevio_motorDirection(DIRN_DOWN);
        // }


        for(int f = 0; f < N_FLOORS; f++){
            for(int b = 0; b < N_BUTTONS; b++){
                int btnPressed = elevio_callButton(f, b);
                

                FSM_registerButton(f,b,btnPressed);
                
                // if(btnPressed){
                //     if(f>floor && floor != -1){
                //         elevio_motorDirection(DIRN_UP);
                //     }
                //     if(f < floor &&  floor != -1){
                //         elevio_motorDirection(DIRN_DOWN);
                //     }
                // }
            }
        }

        FSM_registerFloor( floor);

        stopBtn = elevio_stopButton();
        if(stopBtn){
            stop = !stop;
        }

        FSM_registerEmergency(stop);
        

        ObsBtn = elevio_obstruction();
        FSM_registerObstruction(ObsBtn);

        FSM_updateState();

        queue_print_queue();
        FSM_printButtons();
        
        

        // if(elevio_obstruction()){
        //     elevio_stopLamp(1);
        //     elevio_doorOpenLamp(1);
        //     was_obstruction = true;
            
        // } else {
        //     elevio_stopLamp(0);
        //     if(was_obstruction){
        //         was_obstruction = false;
        //         timer_start();
        //     }
        //     if(!timer_isActive()){ // if not active timer
        //         elevio_doorOpenLamp(0);
        //     } else{ 

        //         if(timer_isTimeout()){ //if timeout
        //         elevio_doorOpenLamp(0);
        //         queue_print_queue();
        //         FSM_printButtons();
        //         }
            
        //     }

        // }
        
        
        // if(elevio_stopButton()){
        //     elevio_motorDirection(DIRN_STOP);
        //     //break;

        //     queue_clear_all();
        
        // }

        nanosleep(&(struct timespec){0, 20*1000*1000}, NULL);
    
    }

    return 0;
}
