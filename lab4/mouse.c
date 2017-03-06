#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/sysutil.h>
#include "i8042.h"
#include "i8254.h"
#include "timer.h"

static int mouse_hook_id = 1;
unsigned char packet[3];

//Subscribes mouse.
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

//Unsubscribes mouse.
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

//Enables mouse and sets stream mode.
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

//Displays the mouse's configuration in a user-friendly way.
void displayConfiguration(){
	printf("BYTE 1\n");
	if (packet[0] & BIT(6) == 1){
		printf("BIT 6: Remote (polled) mode\n");
	}else{
		printf("BIT 6: Stream mode\n");
	}
	if (packet[0] & BIT(5) == 1){
		printf("BIT 5: Data reporting enabled\n");
	}else{
		printf("BIT 5: Data reporting disabled\n");
	}
	if (packet[0] & BIT(4) == 1){
		printf("BIT 4: Scaling is 2:1\n");
	}else{
		printf("BIT 4: Scaling is 1:1\n");
	}
	if (packet[0] & BIT(2) == 1){
		printf("BIT 2: Left button currently pressed\n");
	}else{
		printf("BIT 2: Left button released\n");
	}
	if (packet[0] & BIT(1) == 1){
		printf("BIT 1: Middle button currently pressed\n");
	}else{
		printf("BIT 1: Middle button released\n");
	}
	if (packet[0] & BIT(0) == 1){
		printf("BIT 0: Right button currently pressed\n");
	}else{
		printf("BIT 0: Right button released\n");
	}

	printf("\nBYTE 2\n");
	if ((packet[1] & BIT(0) == 0) && (packet[1] & BIT(1) == 0)){
		printf("1 count per mm");
	}
	else if ((packet[1] & BIT(0) == 1) && (packet[1] & BIT(1) == 0)){
		printf("2 counts per mm");
	}
	else if ((packet[1] & BIT(0) == 0) && (packet[1] & BIT(1) == 1)){
		printf("4 counts per mm");
	}
	else if ((packet[1] & BIT(0) == 1) && (packet[1] & BIT(1) == 1)){
		printf("8 counts per mm");
	}

	printf("\nBYTE 3\n");
	printf("Sample rate: %d", packet[2]);
	return;
}

//Displays the packet's information in a user-friendly way.
void displayPacket(){
	printf("B1=0x%02x ", (unsigned char) packet[0]);
	printf("B2=0x%02x ", (unsigned char) packet[1]);
	printf("B3=0x%02x ", (unsigned char) packet[2]);
	printf("LB=%d ", ((packet[0] & LEFT_BUTTON) ? 1 : 0));
	printf("MB=%d ", ((packet[0] & MIDDLE_BUTTON) ? 1 : 0));
	printf("RB=%d ", ((packet[0] & RIGHT_BUTTON) ? 1 : 0));
	printf("XOV=%d ", ((packet[0] & X_OVERFLOW) ? 1 : 0));
	printf("YOV=%d ", ((packet[0] & Y_OVERFLOW) ? 1 : 0));
	printf("X=%1d ", packet[1]);
	printf("Y=%1d\n", packet[2]);
	return;
}

//Fills packet with the respective bytes.
void processPacket(unsigned int counter){
	long status, byte;

	while(1){
		sys_inb(STATUS_REG, &status);
		if (status & OBF){
			sys_inb(OUT_BUF, &byte);
			packet[counter] = byte;
			break;
		}
		printf(".");
	}
	return;
}

//MAIN TEST_PACKET FUNCTION
int mouse_test_packet(unsigned short cnt){

	int ipc_status;
	message msg;
	int irq_set = mouse_subscribe_int();
	unsigned int interruptsCounter = 0;
	long byte;

	setupMouse();

	while (cnt > 0){
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0){
			printf("\nERROR: driver_receive failed with: %d\n", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)){
			switch(_ENDPOINT_P(msg.m_source)){
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set){

					processPacket(interruptsCounter);
					interruptsCounter++;

					if ((packet[0] & BIT(3)) == 0){
						interruptsCounter = 0;
						continue;
					}
					if (interruptsCounter == 3){
						interruptsCounter = 0;

						displayPacket();
						cnt--;
					}
				}
				break;
			default:
				break;
			}
		}else{
		}
	}
	mouse_unsubscribe_int();
	sys_outb(STATUS_REG, BYTE_TO_MOUSE);
	sys_outb(IN_BUF, MOUSE_DISABLE_DATA);
	printf("\n\nTest completed!\n");
	return 0;
}

int mouse_test_async(unsigned short idle_time){
	int ipc_status;
	message msg;
	int mouse_irq_set = mouse_subscribe_int();
	int timer_irq_set = timer_subscribe_int();

	unsigned int interruptsCounter = 0;
	unsigned int ticksCounter = 0;

	setupMouse();

	while (ticksCounter < idle_time * 60){
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0){
			printf("\nERROR: driver_receive failed with: %d\n", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)){
			switch(_ENDPOINT_P(msg.m_source)){
			case HARDWARE:
				if (msg.NOTIFY_ARG & timer_irq_set){
					ticksCounter++;
				}
				if (msg.NOTIFY_ARG & mouse_irq_set){

					processPacket(interruptsCounter);
					interruptsCounter++;

					if ((packet[0] & BIT(3)) == 0){
						interruptsCounter = 0;
						continue;
					}
					if (interruptsCounter == 3){
						interruptsCounter = 0;
						displayPacket();
					}
				}
				break;
			default:
				break;
			}
		}else{
		}
	}
	mouse_unsubscribe_int();
	timer_unsubscribe_int();

	printf("\n\nTest completed!\n");
	return 0;
}
int mouse_test_config(){
	int ipc_status;
	message msg;
	int irq_set = mouse_subscribe_int();
	unsigned int interruptsCounter = 0;

	setupMouse();

	/*sys_outb(STATUS_REG, BYTE_TO_MOUSE);
	sys_outb(OUT_BUF, STATUS_REQUEST);*/

	while (interruptsCounter < 3) {
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0) {
			printf("\nERROR: driver_receive failed with: %d\n", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set) {
					if (interruptsCounter == 0) {
						if (((packet[0] & BIT(7) != 0)) || ((packet[0] & BIT(3)) != 0)) {
							interruptsCounter = 0;
							continue;
						}
					}
					else if (interruptsCounter == 1) {
						if (((packet[1] & BIT(7)) != 0) || ((packet[1] & BIT(6)) != 0) || ((packet[1] & BIT(5)) != 0) ||
							((packet[1] & BIT(4)) != 0) || ((packet[1] & BIT(3)) != 0) || ((packet[1] & BIT(2)) != 0)) {
							interruptsCounter = 0;
							continue;
						}
					}
					processPacket(interruptsCounter);
					interruptsCounter++;

					if (interruptsCounter == 3) {
						displayConfiguration();
						break;
					}
				}
				break;
			default:
				break;
			}
		} else {
		}
	}
	mouse_unsubscribe_int();

	printf("\n\nTest completed!\n");
	return 0;
}

//State Machine
int finalReached = 0;

void stateMachine(char transition) {
	//States: I (initial), D (drawing), C (complete);
	//Transition: D (RB down), U (RB up), R (reset), E (exit)
	static char state = 'I';

	if (state == 'I'){
		if (transition == 'D'){
			state = 'D';
			return;
		}
	}
	else if (state == 'D'){
		if (transition == 'U'){
			state = 'I';
		}
		else if (transition == 'E'){
			state = 'C';
		}
		return;
	}
	else if (state == 'C'){
		finalReached = 1;
	}
	return;
}

int mouse_test_gesture(short length) {
	int ipc_status;
	message msg;
	int irq_set = mouse_subscribe_int();
	int interruptsCounter = 0;
	int lengthCounter = 0;

	setupMouse();

	while (finalReached == 0) {
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0) {
			printf("\nERROR: driver_receive failed with: %d\n", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set) {
					processPacket(interruptsCounter);
					interruptsCounter++;

					if (packet[0] & BIT(3) == 0) {
						interruptsCounter = 0;
						continue;
					}
					if (interruptsCounter == 3) {
						interruptsCounter = 0;
						displayPacket();

						//Right click was pressed.
						if (packet[0] & BIT(1) == 1) {
							stateMachine('D');
						}
						//Right click was released.
						if (packet[0] & BIT(1) == 0) {
							stateMachine('U');

						//Positive slope
						if (packet[1] > 0 && packet[2] > 0){
							lengthCounter += packet[2];
						}
						else if (packet[1] < 0 && packet[2] < 0){
							lengthCounter -= packet[2];
						}
						//If the max length is exceeded.
						if (lengthCounter > length){
							stateMachine('E');
						}
					}
				}
				break;
			default:
				break;
			}
		}}else {
		}
	}
	mouse_unsubscribe_int();

	printf("\n\nTest completed!\n");
	return 0;
}
