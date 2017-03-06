#include "i8254.h"
#include "i8042.h"
#include "kbd.h"
#include "timer.h"
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

//Handler for the keyboard in C.
int keyboard_int_handler_C() {

	unsigned long data, stat;

	while (1) {
		sys_inb(STAT_REG, &stat);
		if (stat & OBF) {
			sys_inb(OUT_BUF, &data);
			if ((stat & (PAR_ERR | TO_ERR)) == 0)
				return data;
			else{
				printf("\nERROR: keyboard_int_handler_C failed!\n");
				return 1;
			}
		}
		tickdelay(micros_to_ticks(WAIT_KBC));
	}
	return 0;
}

//Main scan codes function.
int kbd_scan_codes(unsigned short ass) {

	unsigned long key;
	message msg;
	int ipc_status, irq_set = kbd_subscribe_int();

	printf("\n> KBD_TEST_SCAN STARTED <\n");

	//This loop runs while the escape key isn't pressed.
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
					if (ass == 0){
						key = keyboard_int_handler_C();
					}
					else
						key = keyboard_int_handler_assembly();

					if (key & BIT(7))
						printf("Breakcode: 0x%02X\n", key);
					else
						printf("Makecode: 0x%02X\n", key);
				}
				break;
			default:
				break;
			}
		} else {
		}
	}
	kbd_unsubscribe_int();
	return 0;
}

//Sends leds argument to KBC.
int kbd_sendArgument(unsigned long argument) {
	unsigned long response = 0;

	if (sys_inb(IN_BUF,&response) != OK)
		return 1;

	if (response & IBF	|| response & KBD_STATUS_PAR || response & KBD_STATUS_TIMEOUT)
		return 1;

	do {
		if (sys_outb(OUT_BUF, argument) != OK)
			return 1;

		while(1){
			if (sys_inb(IN_BUF,&response) != OK)
				return 1;

			if(!(response & KBD_STATUS_TIMEOUT))
				break;

			tickdelay(micros_to_ticks(WAIT_KBC));
		}
		if (sys_inb(OUT_BUF, &response) != OK)
			return 1;

		if (response == KBD_BUFF_ACK )
			break;

	} while (1);
	return 0;
}

//Main led function.
int kbd_led_test(unsigned short *lights, unsigned short change){

	unsigned long counterTime = 0;
	int ipc_status, irq_set = timer_subscribe_int();
	message msg;

	//Timer sleep function (1 second).
	while (counterTime < 60) {
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0) {
			printf("\nERROR: driver_receive failed with: %d\n", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set) {
					counterTime++;
				}
				break;
			default:
				break;
			}
		} else {
		}
	}
	timer_unsubscribe_int();

	while (kbd_sendArgument(SET_KBD_LEDS));
	unsigned long response;

	while (1) {
		if (sys_inb(IN_BUF,&response) != OK)
			return 1;
		if (!(response & KBD_STATUS_TIMEOUT))
			break;
		tickdelay(micros_to_ticks(WAIT_KBC));
	}
	lights[change] = (lights[change]) ? 0 : 1;

	unsigned long pushLeds = 0;
	size_t i = 0;
	for (; i < 3; i++) {
		pushLeds |= (lights[i]) ? BIT(i) : 0;
	}

	printf("\n");
	size_t j = 0;

	//Displays led status in case the keyboard leds aren't responsive.
	for (; j != 3; j++) {
		if (lights[j] == 0 && change == j){
			if (change == 0)
				printf("SCROLL LOCK OFF");
			else if (change == 1)
				printf("NUM LOCK OFF");
			else if (change == 2)
				printf("CAPS LOCK OFF");
		}
		if (lights[j] == 1 && change == j) {
			if (change == 0)
				printf("SCROLL LOCK ON");
			else if (change == 1)
				printf("NUM LOCK ON");
			else if (change == 2)
				printf("CAPS LOCK ON");
		}
	}
	while (kbd_sendArgument(pushLeds));

	return 0;
}

//Main timed scan function.
int itrpScan(unsigned short n) {
	unsigned long key, counterTime = 0;
	message msg;
	int ipc_status, kbd_irq_set = kbd_subscribe_int(), timer_irq_set = timer_subscribe_int();

	printf("\n> KBD_TIMED_SCAN STARTED <\n");

	while (counterTime < 60 * n) {
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0) {
			printf("\nERROR: driver_receive failed with: %d\n", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.NOTIFY_ARG & timer_irq_set) {
					counterTime++;
				}
				if (msg.NOTIFY_ARG & kbd_irq_set) {
					key = keyboard_int_handler_C();
					if (key & BIT(7))
						printf("Breakcode: 0x%02X\n", key);
					else {
						printf("Makecode: 0x%02X\n", key);
					}
				}
				break;
			default:
				break;
			}
			if (key == ESC_KEY) {
				if (kbd_unsubscribe_int() == 1)
					return 1;
				if (timer_unsubscribe_int() == 1)
					return 1;
				return 0;
			}
		}
	}
	if (kbd_unsubscribe_int() == 1)
		return 1;
	if (timer_unsubscribe_int() == 1)
		return 1;
	return 0;
}
