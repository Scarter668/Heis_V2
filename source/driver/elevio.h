/**
 * @file
 * @brief Headerfile for controlling the elevator
 */

#pragma once


/**
 * @brief Amount of floors
 */
#define N_FLOORS 4


/**
 * @brief Struct for the three different motor states, UP, DOWN and STOP.
 */
typedef enum { 
    DIRN_DOWN   = -1,
    DIRN_STOP   = 0,
    DIRN_UP     = 1
} MotorDirection;

/**
 * @brief Amount of buttontypes 
 */
#define N_BUTTONS 3


/**
 * @brief Struct for the three different buttons, UP, DOWN and CAB
 */
typedef enum { 
    BUTTON_HALL_UP      = 0,
    BUTTON_HALL_DOWN    = 1,
    BUTTON_CAB          = 2
} ButtonType;



/**
 * @brief Initializes the elevator
 */
void elevio_init(void);


/**
 * @brief Sets the direction of the motor according to the @p MotorDirection enum
 *
 * @param[in] dirn      Direction of the motor from enum struct.    
 */
void elevio_motorDirection(MotorDirection dirn);


/**
 * @brief Turns the button lamp on or of defined by @p floor and the @p button that is pressed. 
 *
 * @param[in] floor    Floor level
 * @param[in] button     Type of button defined by struct @p Buttontype
 * @param[in] value     0 or 1 to set the lamp
 *
 * @warning @p floor and @p button are asserted to be within their range! Make sure to pass in correct values.
 */
void elevio_buttonLamp(int floor, ButtonType button, int value);

/**
 * @brief Swithces the floor lamp outside of the elevator to the floor defined by @p floor . There is only
 * one light at a time.
 * 
 * @param[in] floor    Floor level
 *
 * @warning @p floor is asserted to be within its range! Make sure to pass in correct values.
 */
void elevio_floorIndicator(int floor);


/**
 * @brief Turns the door open lamp on or off depending on @p value .
 * 
 * @param[in] value    1 or 0 for light on or off. 
 *
 */
void elevio_doorOpenLamp(int value);



/**
 * @brief Turns the stop lamp on or off depending on @p value . This is to indicate that the elevator has been stopped
 *
 * @param[in] value    1 or 0 for light on or off. 
 *
 */
void elevio_stopLamp(int value);



/**
 * @brief Check if the button on a given @p floor with a given type, @p button 
 * , has been pressed. 
 * 
 * @param[in] floor    Floor level
 * @param[in] button    Type of button defined by struct @p Buttontype
 *
 * 
 * @return 0 or 1 depending on the state of the button
 */
int elevio_callButton(int floor, ButtonType button);

/**
 * @brief Checks what floor the elevator is currently at. 
 *
 * @return Floor level (-1 if not on a floor)
 */
int elevio_floorSensor(void);


/**
 * @brief Check if the Stop button has been pressed. 
 * 
 * @return 0 or 1 depending on the state of the button
 */
int elevio_stopButton(void);


/**
 * @brief Check if there is an obstruction preventing the doors from closing.
 *  
 * @return 0 or 1 depending on obstruction
 */
int elevio_obstruction(void);

