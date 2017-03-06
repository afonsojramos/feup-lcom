#ifndef __KBD_H
#define __KBD_H

#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/com.h>
#include <minix/sysutil.h>

/*!
 * @brief Subscribes the keyboard.
 */
int kbd_subscribe_int();

/*!
 * @brief Unsubscribes the keyboard.
 */
int kbd_unsubscribe_int();

/*!
 * @brief Extracts a keycode from the keyboard.
 */
int keyboard_int_handler_C();

/*!
 * @brief This function stays in a loop until the ESC key is pressed.
 */
void waitsForEscapeKey();

#endif
