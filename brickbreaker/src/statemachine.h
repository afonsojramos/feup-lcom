#ifndef _STATEMACHINE_H
#define _STATEMACHINE_H

typedef enum{
	GAME_MENU,
	GAME_HIGHSCORES,
	GAME_RUN,
	GAME_SUBMIT_SCORE,
	GAME_EXIT
} state_t;

typedef enum{
	EVENT_NEXT,
	EVENT_SEE_SCORES,
	EVENT_SKIP,
	EVENT_FORCE_QUIT
} event_t;

typedef struct{
	state_t state;
} StateMachine;

StateMachine sm;

/*!
 * @brief Updates the state on the state machine.
 * @param evt Updates the state based on the current state and this input.
 */
static void updateState(event_t evt){

	switch (sm.state){
	case GAME_MENU:
		if (evt == EVENT_NEXT){
			sm.state = GAME_RUN;
		}
		if (evt == EVENT_SEE_SCORES){
			sm.state = GAME_HIGHSCORES;
		}
		else if (evt == EVENT_FORCE_QUIT){
			sm.state = GAME_EXIT;
		}
		return;
	case GAME_HIGHSCORES:
		if (evt == EVENT_NEXT){
			sm.state = GAME_MENU;
		}
		return;
	case GAME_RUN:
		if (evt == EVENT_NEXT){
			sm.state = GAME_SUBMIT_SCORE;
		}
		else if (evt == EVENT_FORCE_QUIT){
			sm.state = GAME_MENU;
		}
		return;
	case GAME_SUBMIT_SCORE:
		if (evt == EVENT_NEXT){
			sm.state = GAME_MENU;
		}
		return;
	}
}
#endif
