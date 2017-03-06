#include <minix/syslib.h>
#include <minix/drivers.h>
#include "i8254.h"
#include "i8042.h"
#include "kbd.h"

static unsigned long interruptCounter = 0;
static int hook_id = 4;

int timer_subscribe_int() {

	int temp = hook_id;

	if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) != OK){
		printf("\nERROR: sys_irqsetpolicy failed!\n");
	}
	if (sys_irqenable(&hook_id) != OK)
		printf("\nERROR: sys_irqenable failed!\n");

	 return BIT(temp);
}
int timer_unsubscribe_int() {

	if (sys_irqdisable(&hook_id) != OK){
		printf("\nERROR: sys_irqdisable of timer_unsubscribe_int failed!\n");
		return 1;
	}
	if (sys_irqrmpolicy(&hook_id) != OK){
		printf("\nERROR: sys_irqrmpolicy of timer_unsubscribe_int failed!\n");
		return 1;
	}
	return 0;
}
int timer_get_conf(unsigned long timer, unsigned char *st) {

	unsigned long
			command = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer),
			save;

	if (sys_outb(TIMER_CTRL, command) != OK) {
		printf("\nERROR: sys_outb of timer_get_conf failed!\n");
		return 1;
	}
	if (sys_inb(TIMER_0 + timer, &save) != OK) {
		printf("\nERROR: sys_inb of timer_get_conf failed!\n");
		return 1;
	}

	*st = save;
	return 0;
}
int timer_wait_seconds(unsigned int delay){
	int ipc_status;
	message msg;
	int irq_set = timer_subscribe_int();
	int secondsCounter = 0, interruptsCounter = 0;

	while (secondsCounter < delay * 60){
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0){
			printf("\nERROR: driver_receive failed with: %d\n", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)){
			switch(_ENDPOINT_P(msg.m_source)){
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set){
					interruptsCounter++;
					if (interruptCounter % 60 == 0){
						secondsCounter++;
						interruptsCounter = 0;
					}
				}
				break;
			default:
				break;
			}
		}else{
		}
	}
	timer_unsubscribe_int();
}
int waitInterrupts(unsigned int delay){
	int ipc_status;
	message msg;
	int irq_set = timer_subscribe_int(), irq_set_kbd = kbd_subscribe_int();
	int interruptsCounter = 0;
	unsigned long key;

	while (interruptsCounter < delay){
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0){
			printf("\nERROR: driver_receive failed with: %d\n", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)){
			switch(_ENDPOINT_P(msg.m_source)){
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set){
					interruptsCounter++;
				}
				if (msg.NOTIFY_ARG & irq_set_kbd) {
					printf("detected");
					key = keyboard_int_handler_C();
				}
				break;
			default:
				break;
			}
			if (key == ESC_KEY) {
				printf("exited");
				kbd_unsubscribe_int();
				return 2;
			}
		}else{
		}
	}
	timer_unsubscribe_int();
	return 0;
}
