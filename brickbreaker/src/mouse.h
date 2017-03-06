#ifndef __MOUSE_H
#define __MOUSE_H

/*!
 * @brief Subscribes the mouse.
 */
int mouse_subscribe_int();

/*!
 * @brief Unsubscribes the mouse.
 */
int mouse_unsubscribe_int();

/*!
 * @brief Enables mouse and sets stream mode.
 */
void setupMouse();

/*!
 * @brief Fills packet with the respective bytes.
 * @param counter The packet index to be filled with a byte.
 */
void processPacket (unsigned int counter);

#endif
