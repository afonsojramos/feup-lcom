#include "i8254.h"
#include "i8042.h"
#include "kbd.h"
#include "video_gr.h"
#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/sysutil.h>

static int kbd_hook_id = 13;

//Subscribes keyboard.
int kbd_subscribe_int() {

	int temp = kbd_hook_id;

	if (sys_irqsetpolicy(KBD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &kbd_hook_id) != OK) {
		printf("\nERROR: sys_irqsetpolicy failed!\n");
		return 1;
	}
	if (sys_irqenable(&kbd_hook_id) != OK) {
		printf("\nERROR: sys_irqenable failed!\n");
		return 1;
	}
	return BIT(temp);
}

//Unsubscribes keyboard.
int kbd_unsubscribe_int() {

	if (sys_irqdisable(&kbd_hook_id) != OK) {
		printf("\nERROR: sys_irqdisable of kbd_unsubscribe_int failed!\n");
		return 1;
	}
	if (sys_irqrmpolicy(&kbd_hook_id) != OK) {
		printf("\nERROR: sys_irqrmpolicy of kbd_unsubscribe_int failed!\n");
		return 1;
	}
	return 0;
}
int keyboard_int_handler_C() {

	unsigned long data, stat;
	uint8_t timeout = TIMEOUT;

	while (timeout != 0) {
		sys_inb(STAT_REG, &stat);
		if (stat & OBF) {
			sys_inb(OUT_BUF, &data);
			if ((stat & (PAR_ERR | TO_ERR)) == 0){
				tickdelay(micros_to_ticks(WAIT_KBC));
				return data;
			}
			else{
				printf("\nERROR: keyboard_int_handler_C failed!\n");
				tickdelay(micros_to_ticks(WAIT_KBC));
				return 1;
			}
		}
		tickdelay(micros_to_ticks(WAIT_KBC));
		timeout--;
	}
	return 0;
}
void waitsForEscapeKey(){
	unsigned long key;
	message msg;
	int ipc_status, irq_set = kbd_subscribe_int();

	while (key != ESC_KEY) {
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0) {
			printf("\nERROR: driver_receive failed with: %d\n", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set) {
					key = keyboard_int_handler_C();
				}
				break;
			default:
				break;
			}
			if (key == ESC_KEY) {
				kbd_unsubscribe_int();
				return;
			}
		} else {
		}
	}
	kbd_unsubscribe_int();
	return;
}
