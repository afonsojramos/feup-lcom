#ifndef __KBD_H
#define __KBD_H

#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/com.h>
#include <minix/sysutil.h>

int kbd_subscribe_int();
int kbd_unsubscribe_int();
int keyboard_int_handler_C();
unsigned long keyboard_int_handler_asm();
int kbd_scan_codes(unsigned short ass);
int kbd_sendArgument(unsigned long argument);
int kbd_led_test(unsigned short *lights, unsigned short change);

#endif
