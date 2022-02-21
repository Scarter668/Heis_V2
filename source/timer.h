/**
 * @file
 * @brief Headerfile for timer
 */

#pragma once


#include <time.h>
#include "defines.h"



/**
 * @brief The timeout lenght in seconds
 */
#define TIMEOUT_LENGHT 3.0



/**
 * @brief Starts the timer by registering the current time.
 */
void timer_start();


/**
 * @brief Checks if a timer is active
 *
 * @return @c true or @c false depending on @p m_active_timer
 */
bool timer_isActive();

/**
 * @brief Checks if the timer has timed out, and deactivates it if so.
 *
 * @return @c true or @c false depending on the duration of the timer
 */
bool timer_isTimeout();


/**
 * @brief Initiates the static variables used by timer
 */
void timer_init();


/**
 * @brief Stop timer by setting the timer to false.
 */
void timer_stop();
