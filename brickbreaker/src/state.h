#ifndef __STATE_H
#define __STATE_H

typedef enum{
	GAME_MENU,
	GAME_RUN,
	GAME_EXIT
} StatesA;

typedef enum{
	RENDER_FRAME
} StatesB;

static StatesA currentStateA;
static StatesB currentStateB;

#endif
