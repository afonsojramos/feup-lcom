#include <minix/syslib.h>
#include <minix/drivers.h>
#include "i8254.h"

static unsigned long interruptCounter = 0;
static int hook_id = 4;

int timer_set_square(unsigned long timer, unsigned long freq) {

	unsigned long LSB = TIMER_FREQ / freq;
	unsigned long MSB = (TIMER_FREQ / freq) >> 8;

	unsigned char conf;

	if (timer_get_conf(timer, &conf) == 1){
		printf("\nERROR: timer_get_conf failed!\n");
		return 1;
	}

	unsigned long command = TIMER_LSB_MSB | TIMER_SQR_WAVE | TIMER_BCD | TIMER_0;
	if (sys_outb(TIMER_CTRL, command) != OK){
		printf("\nERROR: sys_outb failed!\n");
		return 1;
	}

	if (timer == 0) {
		sys_outb(TIMER_0, LSB);
		sys_outb(TIMER_0, MSB);
	} else if (timer == 1) {
		sys_outb(TIMER_1, LSB);
		sys_outb(TIMER_1, MSB);
	} else if (timer == 2) {
		sys_outb(TIMER_2, LSB);
		sys_outb(TIMER_2, MSB);
	}

	printf("\nSuccessfully set timer %d to run on %d Hz.\n", timer, freq);
	return 0;
}

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

void timer_int_handler() {
	interruptCounter++;
	return;
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

int timer_display_conf(unsigned char conf) {

	if (conf & BIT(0))
		printf("\nCOUNTING MODE : BCD");
	else
		printf("\nCOUNTING MODE : Binary");

	printf("\nOPERATING MODE : ");
	if ((conf & BIT(1))) {
		if (conf & BIT(2))
			printf("3 (Square Wave Mode)\n");
		else {
			if (conf & BIT(3))
				printf("5 (Hardware Triggered Strobe - Retriggerable)\n");
			else
				printf("1 (Hardware Retriggerable One-Shot)\n");
		}
	} else {
		if (conf & BIT(2))
			printf("2 (Rate Generator)\n");
		else {
			if (conf & BIT(3))
				printf("4 (Software Triggered Strobe)\n");
			else
				printf("0 (Interrupt On Terminal Count)\n");
		}
	}

	if (conf & TIMER_LSB_MSB)
		printf("REGISTER SELECTION : LSB first & MSB afterwards\n");
	else if (conf & TIMER_LSB)
		printf("REGISTER SELECTION : LSB only\n");
	else if (conf & TIMER_MSB)
		printf("REGISTER SELECTION : MSB only\n");


	if (conf & BIT(6))
		printf("NULL COUNT : 1\n");
	else
		printf("NULL COUNT : 0\n");

	if (conf & BIT(7))
			printf("OUTPUT : 1\n");
		else
			printf("OUTPUT : 0\n");

	return 0;
}

int timer_test_square(unsigned long freq) {

	if (freq < 19) {
		printf("\nERROR: Frequency too low! Must be higher than 18Hz.\n"); //Overflow (1193182/18 occupies 5 bytes - not supported by unsigned long.
		return 1;
	}
	else if (TIMER_FREQ < freq) {
		printf("\nERROR: Frequency must be lower than the clock signal frequency.\n"); //The value is lower than 1193182.
		return 1;
	}

	if(timer_set_square(0, freq) == 1){
		printf("\nERROR: timer_set_square failed!\n");
		return 1;
	}
	return 0;
}

int timer_test_int(unsigned long time) {

	int ipc_status;
	message msg;
	int irq_set = timer_subscribe_int();
	interruptCounter = 0;
	int secondsCounter = 0;

	if (timer_set_square(0, 60) == 1){ //Resets the timer frequency.
		printf("\nERROR: timer_set_square failed on timer_test_int!\n");
		return 1;
	}

	while (interruptCounter < time * 60){
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0){
			printf("\nERROR: driver_receive failed with: %d\n", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)){
			switch(_ENDPOINT_P(msg.m_source)){
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set){
					timer_int_handler();
					if (interruptCounter % 60 == 0){
						secondsCounter++;
						printf("\n> %d interruptions elapsed. (%d/%d seconds)", interruptCounter, secondsCounter, time);
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

	printf("\n\nTest completed!\n");
	return 0;
}

int timer_test_config(unsigned long timer) {

	unsigned char conf;

	if (timer < 0 || timer > 2){
		printf("\nERROR: %d is not a valid timer! Please select an integer ranging from 0 to 2.\n", timer);
		return 1;
	}

	if (timer_get_conf(timer, &conf) == 1){
		printf("\nERROR: timer_get_conf failed!\n");
		return 1;
	}
	if (timer_display_conf(conf) == 1){
		printf("\nERROR: timer_display_conf failed!\n");
		return 1;
	}

	return 0;
}
