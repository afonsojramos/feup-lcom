#ifndef __TIMER_H
#define __TIMER_H

#include "statemachine.h"

/*!
 * @brief Subscribes the timer.
 */
int timer_subscribe_int();

/*!
 * @brief Unsubscribes the timer.
 */
int timer_unsubscribe_int();

/*!
 * @brief This function stays in a loop for x seconds. Serves as a sort of sleep function.
 * @param delay A period of time, in seconds.
 */
int timer_wait_seconds(unsigned int delay);

/*!
 * @brief This function stays in a loop for x interruptions. Serves as a sort of sleep function and is more precise than the timer_wait_seconds function.
 * @param delay A period of time, in interruptions.
 */
int waitInterrupts(unsigned int delay);

/*!
 * @brief Handles keyboard functions, based on the currently selected state of the state machine.
 * @param key The scancode to be handled.
 */
void keyboardProcessing(unsigned long key);

/*!
 * @brief Handles mouse functions, based on the currently selected state of the state machine.
 * @param packet[3] The mouse packet to be handled.
 * @param option The currently highlighted option on the current menu. Alters the highlight struct.
 */
void mouseProcessing(char packet[3], short option);

/*!
 * @brief Extracts the date from the RTC and updates the clock struct.
 */
void rtcProcessing();

/*!
 * @brief This is the main loop which handles every interrupt, the core of the program.
 * Both the timer and the keyboard, as well as the mouse are subscribed in this loop (and in this loop only).
 * The timer allows for frame rendering at a 60 fps and collision testing every timer tick.
 */
int masterClock();

#endif
