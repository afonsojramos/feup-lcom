#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/sysutil.h>
#include "i8042.h"
#include "i8254.h"
#include "timer.h"

static int mouse_hook_id = 1;
unsigned char packet[3];

int mouse_subscribe_int() {

	int temp = mouse_hook_id;

	if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &mouse_hook_id) != OK) {
		printf("\nERROR: sys_irqsetpolicy failed!\n");
		return 1;
	}
	if (sys_irqenable(&mouse_hook_id) != OK) {
		printf("\nERROR: sys_irqenable failed!\n");
		return 1;
	}
	return BIT(temp);
}
int mouse_unsubscribe_int() {

	if (sys_irqdisable(&mouse_hook_id) != OK) {
		printf("\nERROR: sys_irqdisable of kbd_unsubscribe_int failed!\n");
		return 1;
	}
	if (sys_irqrmpolicy(&mouse_hook_id) != OK) {
		printf("\nERROR: sys_irqrmpolicy of kbd_unsubscribe_int failed!\n");
		return 1;
	}
	return 0;
}
void setupMouse(){
	sys_outb(STATUS_REG, BYTE_TO_MOUSE);
	sys_outb(OUT_BUF, SET_STREAM_MODE);

	tickdelay(micros_to_ticks(DEFAULT_WAIT));

	sys_outb(STATUS_REG, BYTE_TO_MOUSE);
	sys_outb(OUT_BUF, MOUSE_ENABLE);

	tickdelay(micros_to_ticks(DEFAULT_WAIT));

	sys_outb(STATUS_REG, BYTE_TO_MOUSE);
	sys_outb(OUT_BUF, MOUSE_ENABLE_DATA);

	return;
}
void processPacket(unsigned int counter){
	long status, byte;

	while(1){
		sys_inb(STATUS_REG, &status);
		if (status & OBF){
			sys_inb(OUT_BUF, &byte);
			packet[counter] = byte;
			break;
		}
	}
	return;
}
