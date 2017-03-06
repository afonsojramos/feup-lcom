#include <minix/syslib.h>
#include <minix/drivers.h>
#include "i8254.h"
#include "i8042.h"
#include "kbd.h"
#include "video_gr.h"
#include "mouse.h"
#include "settings.h"
#include "timer.h"
#include "statemachine.h"

static unsigned long interruptCounter = 0;
static int hook_id = 4;
unsigned char packet[3];

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
					key = keyboard_int_handler_C();
				}
				break;
			default:
				break;
			}
			if (key == ESC_KEY) {
				kbd_unsubscribe_int();
				return 2;
			}
		}else{
		}
	}
	timer_unsubscribe_int();
	return 0;
}
/* LOCAL VARIABLES NECESSARY FOR MASTERCLOCK */
char savedClick;
unsigned short leftKeyPressed = 0, rightKeyPressed = 0, leftKeyPressedMouse = 0, rightKeyPressedMouse = 0;
unsigned short highscorePos = 5;

void keyboardProcessing(unsigned long key){

	/* KEYBOARD IN MAIN MENU */
	if (sm.state == GAME_MENU){
		if (key == ESC_KEY_REL){
			updateState(EVENT_FORCE_QUIT);
		}
	}
	/* KEYBOARD IN GAME RUN */
	else if (sm.state == GAME_RUN){
		if (key == ESC_KEY_REL){
			drawRectangle(0, 0, 1024, 768, 0); //Screen wipe.
			updateState(EVENT_FORCE_QUIT);
		}
		else if (key == A_KEY) {
			leftKeyPressed = 1;
		}
		else if (key == D_KEY) {
			rightKeyPressed = 1;
		}
		else if (key == A_KEY_REL) {
			leftKeyPressed = 0;
		}
		else if (key == D_KEY_REL) {
			rightKeyPressed = 0;
		}
	}
	/* KEYBOARD IN SUBMIT SCORE */
	else if (sm.state == GAME_SUBMIT_SCORE){
		if (key == A_KEY && highscorePos > 5){
			highscorePos--;
			highlightOption(highscorePos);
		}
		else if (key == D_KEY && highscorePos < 7){
			highscorePos++;
			highlightOption(highscorePos);
		}
		else if (key == W_KEY){
			cycleAlphabet(highscorePos, 'U');
		}
		else if (key == S_KEY){
			cycleAlphabet(highscorePos, 'D');
		}
	}
	return;
}

void mouseProcessing(char packet[3], short option){

	/* MOUSE IN MAIN MENU */
	if (sm.state == GAME_MENU){
		if (packet[0] & LEFT_BUTTON){
			if (option == 1){
				drawRectangle(0, 0, 1024, 768, 0); //Screen wipe.
				loadSprites();
				updateState(EVENT_NEXT);
				highlightOption(9);
			}
			else if (option == 2){
				drawRectangle(0, 0, 1024, 768, 0); //Screen wipe.
				updateState(EVENT_SEE_SCORES);
				highlightOption(9);
			}
			else if (option == 3){
				updateState(EVENT_FORCE_QUIT);
				highlightOption(9);
			}
		}else{
			moveCursor(packet[1], packet[2]);
		}
	}
	/* MOUSE IN HIGHSCORES */
	else if (sm.state == GAME_HIGHSCORES){
		if (packet[0] & LEFT_BUTTON){
			if (option == 4){
				drawRectangle(0, 0, 1024, 768, 0); //Screen wipe.
				updateState(EVENT_NEXT);
				highlightOption(9);
			}
		}else{
			moveCursor(packet[1], packet[2]);
		}
	}
	/* MOUSE IN GAME RUN */
	else if (sm.state == GAME_RUN){
		if (packet[0] & LEFT_BUTTON){
			savedClick = 'L';
		}
		else if (packet[0] & RIGHT_BUTTON){
			savedClick = 'R';
		}else{
			savedClick = 'X';
		}
	}
	/* MOUSE IN SUBMIT SCORE */
	else if (sm.state == GAME_SUBMIT_SCORE){
		if (packet[0] & LEFT_BUTTON){
			if (option == 8){
				drawRectangle(0, 0, 1024, 768, 0); //Screen wipe.
				sortHighscore();
				updateState(EVENT_NEXT);
				highlightOption(9);
			}
		}else{
			moveCursor(packet[1], packet[2]);
		}
	}
	return;
}

void rtcProcessing(){
	unsigned long hour, minute, second, day, month, year;

	do{
		if (!updateCheck()){
			extractDate(&hour, &minute, &second, &day, &month, &year);
			updateTime(hour, minute, second, day, month, year);
		}
	} while (updateCheck());

	return;
}

int masterClock(){
	int ipc_status;
	message msg;

	/* SUBSCRIPTIONS */
	int irq_set = timer_subscribe_int();
	int irq_set_kbd = kbd_subscribe_int();
	int irq_set_mouse = mouse_subscribe_int();

	int interruptsCounter = 0, mouseCounter = 0;
	unsigned long key;

	unsigned long data;
	char packet[3];
	unsigned short currentPacket = 0;

	unsigned short wKP = 0, sKP = 0;
	unsigned short mayReturn = 1;

	unsigned short highlightedOption = 9;

	uint8_t isSynced = FALSE; //o MINIX ja vem com TRUE/FALSE em C
	uint32_t byte;

	setupMouse();

	FILE *fp;
	fp = fopen("/tmp/log", "a+");

	while (1) {
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0) {
			printf("\nERROR: driver_receive failed with: %d\n", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) {
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE:
				if (msg.NOTIFY_ARG & irq_set) {
					interruptsCounter++;

					if (interruptsCounter % 1 == 0){
						if (sm.state == GAME_MENU){
							firstRender();
							if (key == ESC_KEY && mayReturn == 1){
								return;
							}
							int temp = testCursorCol();

							if (temp != highlightedOption){
								highlightedOption = temp;
								highlightOption(highlightedOption);
							}
							rtcProcessing();
						}
						else if (sm.state == GAME_HIGHSCORES){
							firstRender();
							int temp = testCursorCol();

							if (temp != highlightedOption){
								highlightedOption = temp;
								highlightOption(highlightedOption);
							}
						}
						else if (sm.state == GAME_RUN){
							mayReturn = 0;

							firstRender();
							moveBall();
							testCollision();
							testCollision2();
							if (testBallCol() == 1){
								highlightOption(9);
								drawRectangle(0, 0, 1024, 768, 0); //Screen wipe.
								launchHighscore();
							}
							testPowerUpCollision();

							if (leftKeyPressed == 1 && key != 0x9E){
								movePaddle('L');
							}
							if (rightKeyPressed == 1 && key != 0xA0) {
								movePaddle('R');
							}
							if (savedClick == 'L'){
								movePaddle('L');
							}
							else if (savedClick == 'R'){
								movePaddle('R');
							}
						}
						else if (sm.state == GAME_SUBMIT_SCORE){
							firstRender();
							int temp = testCursorCol();

							if (temp != highlightedOption){
								highlightedOption = temp;
								highlightOption(highlightedOption);
							}
						}
						else if (sm.state == GAME_EXIT){
							return 0;
						}
						renderFrame();
					}
					if (interruptsCounter % 60 == 0 && sm.state == GAME_RUN){
						updateTimer();
					}
				}
				if (msg.NOTIFY_ARG & irq_set_kbd) {
					key = keyboard_int_handler_C();

					keyboardProcessing(key);

					//fprintf(fp, "Code: 0x%02X\n", key);
				}
				if (msg.NOTIFY_ARG & irq_set_mouse) {

					sys_inb(OUT_BUF, &byte);
					packet[currentPacket] = byte;
					if ((packet[0] & BIT(3)) == 0){
						currentPacket = 0;
					}else{
						if (currentPacket == 2){
							mouseProcessing(packet, highlightedOption);
							currentPacket = 0;
						}else{
							currentPacket++;
						}
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
	kbd_unsubscribe_int();
	timer_unsubscribe_int();

	fclose(fp);
	return 0;
}

