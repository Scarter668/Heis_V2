/**
 * @file
 * @brief main program of the application
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "timer.h"

#include "FSM.h"

int main()
{

    elevio_init();
    timer_init();
    FSM_init();

    while (1)
    {

        FSM_update();

        nanosleep(&(struct timespec){0, 20 * 1000 * 1000}, NULL);
    }

    return 0;
}
