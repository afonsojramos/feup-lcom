#include <minix/syslib.h>
#include <minix/drivers.h>
#include <machine/int86.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "timer.h"
#include "video_gr.h"
#include "kbd.h"
#include "sprites.h"
#include "vbe.h"
#include "statemachine.h"

int main(){

	sef_startup();
	srand(time(NULL));
	vg_init(0x105);

	loadSprites();
	loadHighscores();

	sm.state = GAME_MENU;
	masterClock();

	vg_exit();

	return 0;
}
