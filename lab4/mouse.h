#ifndef __MOUSE_H
#define __MOUSE_H

int mouse_subscribe_int();
int mouse_unsubscribe_int();
void setupMouse();
void displayPacket();
void processPacket (unsigned int counter);
int mouse_test_packet(unsigned short cnt);
int mouse_test_async(unsigned short idle_time);
int mouse_test_config();
int mouse_test_gesture(short length);

typedef enum {INIT, DRAW, COMP} state_t;
typedef enum {RDOWN, RUP, MOVE} ev_type_t;
void check_hor_line(ev_type_t *evt);

#endif
