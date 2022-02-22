/**
 * @file
 * @brief main program of the application
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>



#include "FSM.h"





// test BENJAMIN




int main(){
    
    FSM_init();
    
    printf("=== Example Program ===\n");
    printf("Press the stop button on the elevator panel to exit\n");

    while(1){
        
        FSM_update();

        nanosleep(&(struct timespec){0, 20*1000*1000}, NULL);
    
    }

    return 0;
}
