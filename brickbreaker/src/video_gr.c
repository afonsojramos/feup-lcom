#include <minix/syslib.h>
#include <minix/drivers.h>
#include <machine/int86.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "vbe.h"
#include "video_gr.h"
#include "read_xpm.h"
#include "timer.h"

#include "statemachine.h"
#include "settings.h"

#include "sprites.h"
#include "stableSprites.h"

/* Constants for VBE 0x105 mode */

/* The physical address may vary from VM to VM.
 * At one time it was 0xD0000000
 *  #define VRAM_PHYS_ADDR    0xD0000000 
 * Currently on lab B107 is 0xF0000000
 * Better run my version of lab5 as follows:
 *     service run `pwd`/lab5 -args "mode 0x105"
 */
#define VRAM_PHYS_ADDR	  0xE0000000
#define H_RES             1024
#define V_RES			  768
#define BITS_PER_PIXEL	  8

/* Private global variables */

static char *video_mem;		/* Process address to which VRAM is mapped */

static unsigned h_res;		/* Horizontal screen resolution in pixels */
static unsigned v_res;		/* Vertical screen resolution in pixels */
static unsigned bits_per_pixel; /* Number of VRAM bits per pixel */

int vg_exit() {
  struct reg86u reg86;

  reg86.u.b.intno = 0x10; /* BIOS video services */

  reg86.u.b.ah = 0x00;    /* Set Video Mode function */
  reg86.u.b.al = 0x03;    /* 80x25 text mode*/

  if( sys_int86(&reg86) != OK ) {
      printf("\tvg_exit(): sys_int86() failed \n");
      return 1;
  } else
      return 0;
}
void *vg_init(unsigned short mode){

	unsigned int vram_size;  /*VRAM’s size, but you can use the frame-buffer size, instead*/
	struct reg86u r;

	r.u.w.ax = 0x4F02; // VBE call, function 02 -- set VBE mode
	r.u.w.bx = 1<<14 | mode; // set bit 14: linear framebuffer
	r.u.b.intno = 0x10;
	if (sys_int86(&r) != OK) {
		printf("set_vbe_mode: sys_int86() failed \n");
		return NULL;
	}

	vram_size = H_RES * V_RES * (BITS_PER_PIXEL / 8);
	int k;
	struct mem_range mr;

	/*Allow memory mapping*/
	mr.mr_base = (phys_bytes) VRAM_PHYS_ADDR;
	mr.mr_limit = mr.mr_base + vram_size;
	if (OK != (k = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
		panic("sys_privctl (ADD_MEM) failed: %d\n", k);

	/*Map memory*/
	video_mem = vm_map_phys(SELF, (void*)mr.mr_base, vram_size);
	if (video_mem == MAP_FAILED)
		panic("couldn’t map video memory");

	h_res = H_RES;
	v_res = V_RES;
	bits_per_pixel = BITS_PER_PIXEL;

	return video_mem;
}
int switchPixel(unsigned short x, unsigned short y, unsigned long color){

	if (x>=h_res || y>=v_res)
		return 1;
	char *gpu_ptr = video_mem;

	gpu_ptr += (x + h_res * y) * (bits_per_pixel / 8);
	*gpu_ptr = color;

	return 0;
}
int drawRectangle(unsigned short x, unsigned short y, unsigned short xSize, unsigned short ySize, unsigned long color){
	int i, j;
	for (i = 0; i < xSize; i++) {
		for (j = 0; j < ySize; j++) {
			switchPixel(x + i, y + j, color);
		}
	}
	return 0;
}
int displayXPM(unsigned short xi, unsigned short yi, char *xpm[]){
	int width, height;
	char *sprite = read_xpm(xpm, &width, &height);

	int i, j;
	for (i = 0; i < height; i++){
		for (j = 0; j < width; j++){
			switchPixel(xi + j, yi + i, *(sprite + i * width + j));
		}
	}
	free(sprite);
	return 0;
}
void clearSprite(unsigned short xi, unsigned short yi, char *xpm[]){
	int width, height;
	char *sprite = read_xpm(xpm, &width, &height);

	int i, j;
	for (i = 0; i < height; i++){
		for (j = 0; j < width; j++){
			switchPixel(xi + j, yi + i, 0);
		}
	}
	return;
}
void dropPowerup(unsigned short x, unsigned short y){

	int i;
	for (i = 0; i < 5; i++){
		if (allPowerups[i].dropping == 0){
			if (rand() % flow.powDropRate == 0){
				powerup.x = x;
				powerup.y = y;

				unsigned short temp = rand() % NUM_POWERUPS;
				if (temp == 0){
					powerup.sprite = pwupx2;
				}
				else if (temp == 1){
					powerup.sprite = pwupx4;
				}
				else if (temp == 2){
					powerup.sprite = pwupextraballs;
				}
				powerup.dropping = 1;

				allPowerups[i] = powerup;
				break;
			}
		}
	}
	return;
}

void activateEffect(int index){
	char** temp = allPowerups[index].sprite;

	if (temp == pwupx2 && allPowerups[index].active == 0){
		allPowerups[index].active = 1;
		flow.multiplier = 1;
		flow.multiplier = flow.multiplier * 2;
	}
	else if (temp == pwupx4 && allPowerups[index].active == 0){
		allPowerups[index].active = 1;
		flow.multiplier = 1;
		flow.multiplier = flow.multiplier * 4;
	}
	else if (temp == pwupextraballs && allPowerups[index].active == 0){
		ball2.x = 400; ball2.y = 600;
		ball2.speedx = 2; ball2.speedy = 2;
		ball2.dir = rand() % 4 + 1;
		ball2.sprite = ballSprite;
		render.ball2 = 1;
	}
}
int testBallCol(){
	if (ball2.y >= 720)
		render.ball2 = 0;
	if (ball.y >= 720){
		return 1;
	}
	return 0;
}
int testCollision(){
	int i;
	for (i = 0; i < getBrickSize() ; i++){
		if (allBricks[i].visible == 1){
			if (((ball.y - BALL_RADIUS == allBricks[i].y + BRICK_HEIGHT / 2) && (ball.x - BALL_RADIUS < allBricks[i].x + BRICK_WIDTH /2) && (ball.x + BALL_RADIUS > allBricks[i].x - BRICK_WIDTH / 2)) ||
					((ball.y + BALL_RADIUS == allBricks[i].y - BRICK_HEIGHT / 2) && (ball.x - BALL_RADIUS < allBricks[i].x + BRICK_WIDTH /2) && (ball.x + BALL_RADIUS > allBricks[i].x - BRICK_WIDTH / 2))){
				clearSprite(allBricks[i].x - BRICK_WIDTH / 2, allBricks[i].y - BRICK_HEIGHT / 2, redBrick);

				dropPowerup(allBricks[i].x, allBricks[i].y);

				allBricks[i].visible = 0;

				if (ball.dir == 1) { 		//cima esq
					ball.dir = 2;
				}
				else if (ball.dir == 2) {	//baixo esq
					ball.dir = 1;
				}
				else if (ball.dir == 3) {	//cima dire
					ball.dir = 4;
				}
				else if (ball.dir == 4) {	//baixo dire
					ball.dir = 3;
				}
				updateScore();
				return 0;
			}
			else if (((ball.x + BALL_RADIUS == allBricks[i].x - BRICK_WIDTH / 2) && (ball.y + BALL_RADIUS > allBricks[i].y - BRICK_HEIGHT /2) && (ball.y - BALL_RADIUS < allBricks[i].y + BRICK_HEIGHT / 2)) ||
					((ball.x - BALL_RADIUS == allBricks[i].x + BRICK_WIDTH / 2) && (ball.y + BALL_RADIUS > allBricks[i].y - BRICK_HEIGHT /2) && (ball.y - BALL_RADIUS < allBricks[i].y + BRICK_HEIGHT / 2))){
				clearSprite(allBricks[i].x - BRICK_WIDTH / 2, allBricks[i].y - BRICK_HEIGHT / 2, redBrick);

				dropPowerup(allBricks[i].x, allBricks[i].y);

				allBricks[i].visible = 0;

				if (ball.dir == 1) { 		//cima esq
					ball.dir = 3;
				}
				else if (ball.dir == 2) {	//baixo esq
					ball.dir = 4;
				}
				else if (ball.dir == 3) {	//cima dire
					ball.dir = 1;
				}
				else if (ball.dir == 4) {	//baixo dire
					ball.dir = 2;
				}
				updateScore();
				return 0;
			}
		}
	}
	return 1;
}

int testPowerUpCollision(){
	int j;
	for (j = 0; j < 5; j++){
		if (allPowerups[j].dropping == 1){
			if (allPowerups[j].y >= V_RES){
				allPowerups[j].dropping = 0; //Despawns powerup.
			}
			if ((allPowerups[j].x + 23 >= paddle.x && allPowerups[j].x <= paddle.x + 125) &&
				(allPowerups[j].y + 14 >= paddle.y && allPowerups[j].y <= paddle.y - 10)){
				allPowerups[j].dropping = 0;
				clearSprite(allPowerups[j].x, allPowerups[j].y, allPowerups[j].sprite);
				activateEffect(j);
			}
		}
	}
	return 1;
}

int testCollision2(){
	if (render.ball2 == 1){
		int i;
		for (i = 0; i < getBrickSize() ; i++){
			if (allBricks[i].visible == 1){
				if (((ball2.y - BALL_RADIUS == allBricks[i].y + BRICK_HEIGHT / 2) && (ball2.x - BALL_RADIUS < allBricks[i].x + BRICK_WIDTH /2) && (ball2.x + BALL_RADIUS > allBricks[i].x - BRICK_WIDTH / 2)) ||
						((ball2.y + BALL_RADIUS == allBricks[i].y - BRICK_HEIGHT / 2) && (ball2.x - BALL_RADIUS < allBricks[i].x + BRICK_WIDTH /2) && (ball2.x + BALL_RADIUS > allBricks[i].x - BRICK_WIDTH / 2))){
					clearSprite(allBricks[i].x - BRICK_WIDTH / 2, allBricks[i].y - BRICK_HEIGHT / 2, redBrick);

					dropPowerup(allBricks[i].x, allBricks[i].y);

					allBricks[i].visible = 0;

					if (ball2.dir == 1) { 		//cima esq
						ball2.dir = 2;
					}
					else if (ball2.dir == 2) {	//baixo esq
						ball2.dir = 1;
					}
					else if (ball2.dir == 3) {	//cima dire
						ball2.dir = 4;
					}
					else if (ball2.dir == 4) {	//baixo dire
						ball2.dir = 3;
					}
					updateScore();
					return 0;
				}
				else if (((ball2.x + BALL_RADIUS == allBricks[i].x - BRICK_WIDTH / 2) && (ball2.y + BALL_RADIUS > allBricks[i].y - BRICK_HEIGHT /2) && (ball2.y - BALL_RADIUS < allBricks[i].y + BRICK_HEIGHT / 2)) ||
						((ball2.x - BALL_RADIUS == allBricks[i].x + BRICK_WIDTH / 2) && (ball2.y + BALL_RADIUS > allBricks[i].y - BRICK_HEIGHT /2) && (ball2.y - BALL_RADIUS < allBricks[i].y + BRICK_HEIGHT / 2))){
					clearSprite(allBricks[i].x - BRICK_WIDTH / 2, allBricks[i].y - BRICK_HEIGHT / 2, redBrick);

					dropPowerup(allBricks[i].x, allBricks[i].y);

					allBricks[i].visible = 0;

					if (ball2.dir == 1) { 		//cima esq
						ball2.dir = 3;
					}
					else if (ball2.dir == 2) {	//baixo esq
						ball2.dir = 4;
					}
					else if (ball2.dir == 3) {	//cima dire
						ball2.dir = 1;
					}
					else if (ball2.dir == 4) {	//baixo dire
						ball2.dir = 2;
					}
					updateScore();
					return 0;
				}
			}
		}
	}
	return 1;
}
char** alphanumericMatch(char c){
	if (c == 'A'){
		return aFont;
	}
	else if (c == 'B'){
		return bFont;
	}
	else if (c == 'C'){
		return cFont;
	}
	else if (c == 'D'){
		return dFont;
	}
	else if (c == 'E'){
		return eFont;
	}
	else if (c == 'F'){
		return fFont;
	}
	else if (c == 'G'){
		return gFont;
	}
	else if (c == 'H'){
		return hFont;
	}
	else if (c == 'I'){
		return iFont;
	}
	else if (c == 'J'){
		return jFont;
	}
	else if (c == 'K'){
		return kFont;
	}
	else if (c == 'L'){
		return lFont;
	}
	else if (c == 'M'){
		return mFont;
	}
	else if (c == 'N'){
		return nFont;
	}
	else if (c == 'O'){
		return oFont;
	}
	else if (c == 'P'){
		return pFont;
	}
	else if (c == 'Q'){
		return qFont;
	}
	else if (c == 'R'){
		return rFont;
	}
	else if (c == 'S'){
		return sFont;
	}
	else if (c == 'T'){
		return tFont;
	}
	else if (c == 'U'){
		return uFont;
	}
	else if (c == 'V'){
		return vFont;
	}
	else if (c == 'W'){
		return wFont;
	}
	else if (c == 'X'){
		return xFont;
	}
	else if (c == 'Y'){
		return yFont;
	}
	else if (c == 'Z'){
		return zFont;
	}
	else if (c == '0'){
		return n0Font;
	}
	else if (c == '1'){
		return n1Font;
	}
	else if (c == '2'){
		return n2Font;
	}
	else if (c == '3'){
		return n3Font;
	}
	else if (c == '4'){
		return n4Font;
	}
	else if (c == '5'){
		return n5Font;
	}
	else if (c == '6'){
		return n6Font;
	}
	else if (c == '7'){
		return n7Font;
	}
	else if (c == '8'){
		return n8Font;
	}
	else if (c == '9'){
		return n9Font;
	}
}

/* HIGHSCORE FUNCTIONS */
void sortHighscore(){

	//Creates new highscore.
	highscore.alpha1 = alphabet[fontcycle.iterator1];
	highscore.alpha2 = alphabet[fontcycle.iterator2];
	highscore.alpha3 = alphabet[fontcycle.iterator3];

	highscore.alpha1sprite = alphanumericMatch(highscore.alpha1);
	highscore.alpha2sprite = alphanumericMatch(highscore.alpha2);
	highscore.alpha3sprite = alphanumericMatch(highscore.alpha3);
	highscore.value = score.value;

	char c1 = (highscore.value % 10)  + '0';
	char c2 = (highscore.value % 100)/10  + '0';
	char c3 = (highscore.value % 1000)/100  + '0';

	highscore.digit0sprite = alphanumericMatch(c1);
	highscore.digit1sprite = alphanumericMatch(c2);
	highscore.digit2sprite = alphanumericMatch(c3);

	highscore.date = clock.month * 100 + clock.day;
	char d1 = (highscore.date % 10)  + '0';
	char d2 = (highscore.date % 100)/10  + '0';
	char d3 = (highscore.date % 1000)/100  + '0';
	char d4 = (highscore.date % 10000)/1000  + '0';

	highscore.day0sprite = alphanumericMatch(d1);
	highscore.day1sprite = alphanumericMatch(d2);
	highscore.month0sprite = alphanumericMatch(d3);
	highscore.month1sprite = alphanumericMatch(d4);

	//Finds the swap position.
	int swapPos, i;
	for (i = 0; i < 8; i++){
		if (allHighscores[i].value < highscore.value){
			swapPos = i;
			break;
		}
	}

	//Fill the new temporary vector.
	Highscore temp[8];
	int j, k = 0;
	for (j = 0; j < 8; j++){
		if (swapPos == j){
			temp[j] = highscore;
		}else{
			temp[j] = allHighscores[k];
			k++;
		}
	}
	memcpy(allHighscores, temp, sizeof(allHighscores));

	return;
}
void launchHighscore(){
	if (score.value > allHighscores[7].value){
		render.ball2 = 0;
		updateState(EVENT_NEXT);
	}else{
		render.ball2 = 0;
		updateState(EVENT_FORCE_QUIT);
	}
	return;
}
void saveHighscores(){
	FILE *file = fopen("home/lcom/lcom1617-t1g13/brickbreaker/src/highscores.txt", "w");

	int i;
	for (i = 0; i < 8; i++){
		fprintf(file, "%c%c%c-", allHighscores[i].alpha1, allHighscores[i].alpha2, allHighscores[i].alpha3);
		fprintf(file, "%c%c%c-", (allHighscores[i].value % 1000)/100, (allHighscores[i].value % 100)/10, allHighscores[i].value % 10);
		fprintf(file, "%c%c%c%c ", (allHighscores[i].date % 10000)/1000, (allHighscores[i].value % 1000)/100, (allHighscores[i].value % 100)/10, allHighscores[i].value % 10);
	}
	fclose(file);
	return;
}

int testCursorCol(){
	if (sm.state == GAME_MENU){
		if (cursor.x >= 380 && cursor.x <= 662 && cursor.y >= 500 && cursor.y <= 560){
			return 1;
		}
		else if (cursor.x >= 290 && cursor.x <= 766 && cursor.y >= 600 && cursor.y <= 640){
			return 2;
		}
		else if (cursor.x >= 425 && cursor.x <= 613 && cursor.y >= 675 && cursor.y <= 715){
			return 3;
		}
	}
	else if (sm.state == GAME_HIGHSCORES){
		if (cursor.x >= 300 && cursor.x <= 696 && cursor.y >= 630 && cursor.y <= 714){
			return 4;
		}
	}
	else if (sm.state == GAME_SUBMIT_SCORE){
		//displayXPM(355, 600, submitHighscore);
		if (cursor.x >= 355 && cursor.x <= 710 && cursor.y >= 600 && cursor.y <= 650){
			return 8;
		}
	}
	return 0;
}

void movePaddle(char direction){
	clearSprite(paddle.x, paddle.y, paddleSprite);	//Clears the previous paddle.
	if (direction == 'L' && paddle.x > 40){
		paddle.x -= paddle.speed;
	}
	else if (direction == 'R' && paddle.x < 862) {
		paddle.x += paddle.speed;
	}
	return;
}
void moveCursor(char x, char y){
	clearSprite(cursor.x, cursor.y, mouseCursor);

	cursor.x += x * cursor.sensitivity;
	cursor.y -= y * cursor.sensitivity;

	return;
}


void moveBall() {
	unsigned short x0 = ball.x, y0 = ball.y;

	if (ball.dir == 1) {//cima esq
		if (ball.x < 36 + BALL_RADIUS)
			ball.dir = 3;
		else if (ball.y < 136 + BALL_RADIUS)
			ball.dir = 2;
		ball.x = ball.x - ball.speedx;
		ball.y = ball.y - ball.speedy;

	} else if (ball.dir == 2) {	//baixo esq

		if (ball.x < 36 + BALL_RADIUS)
			ball.dir = 4;
		else if (ball.y > 730)
			ball.dir = 1;
		else if ((ball.y < paddle.y) && (ball.y > paddle.y - 10) && (ball.x > paddle.x) && (ball.x < paddle.x + 125) && (ball.x + 10 > paddle.x) && (ball.x + 10 < paddle.x + 125))
			ball.dir = 1;
		ball.x = ball.x - ball.speedx;
		ball.y = ball.y + ball.speedy;

	} else if (ball.dir == 3) {	//cima dire

		if (ball.x > 984 - BALL_RADIUS)
			ball.dir = 1;
		else if (ball.y < 136 + BALL_RADIUS)
			ball.dir = 4;
		ball.x = ball.x + ball.speedx;
		ball.y = ball.y - ball.speedy;

	} else if (ball.dir == 4) {	//baixo dire

		if (ball.x > 984 - BALL_RADIUS)
			ball.dir = 2;
		else if (ball.y > 730)
			ball.dir = 3;
		else if ((ball.y < paddle.y) && (ball.y > paddle.y - 10) && (ball.x > paddle.x) && (ball.x < paddle.x + 125) && (ball.x + 10 > paddle.x) && (ball.x + 10 < paddle.x + 125))
			ball.dir = 3;
		ball.x = ball.x + ball.speedx;
		ball.y = ball.y + ball.speedy;

	}
	clearSprite(x0 - BALL_RADIUS, y0 - BALL_RADIUS, ball.sprite);

	if (render.ball2 == 1) {
		unsigned short x2 = ball2.x, y2 = ball2.y;
		if (ball2.dir == 1) {	//cima esq
			if (ball2.x < 36 + BALL_RADIUS)
				ball2.dir = 3;
			else if (ball2.y < 136 + BALL_RADIUS)
				ball2.dir = 2;
			ball2.x = ball2.x - ball2.speedx;
			ball2.y = ball2.y - ball2.speedy;

		} else if (ball2.dir == 2) {	//baixo esq

			if (ball2.x < 36 + BALL_RADIUS)
				ball2.dir = 4;
			else if (ball2.y > 730)
				ball2.dir = 1;
			else if ((ball2.y < paddle.y) && (ball2.y > paddle.y - 10)
					&& (ball2.x > paddle.x) && (ball2.x < paddle.x + 125)
					&& (ball2.x + 10 > paddle.x)
					&& (ball2.x + 10 < paddle.x + 125))
				ball2.dir = 1;
			ball2.x = ball2.x - ball2.speedx;
			ball2.y = ball2.y + ball2.speedy;

		} else if (ball2.dir == 3) {	//cima dire

			if (ball2.x > 984 - BALL_RADIUS)
				ball2.dir = 1;
			else if (ball2.y < 136 + BALL_RADIUS)
				ball2.dir = 4;
			ball2.x = ball2.x + ball2.speedx;
			ball2.y = ball2.y - ball2.speedy;

		} else if (ball2.dir == 4) {	//baixo dire

			if (ball2.x > 984 - BALL_RADIUS)
				ball2.dir = 2;
			else if (ball2.y > 730)
				ball2.dir = 3;
			else if ((ball2.y < paddle.y) && (ball2.y > paddle.y - 10)
					&& (ball2.x > paddle.x) && (ball2.x < paddle.x + 125)
					&& (ball2.x + 10 > paddle.x)
					&& (ball2.x + 10 < paddle.x + 125))
				ball2.dir = 3;
			ball2.x = ball2.x + ball2.speedx;
			ball2.y = ball2.y + ball2.speedy;

		}
		clearSprite(x2 - BALL_RADIUS, y2 - BALL_RADIUS, ball2.sprite);
	}
	return;
}
void updateTime(unsigned long hour, unsigned long minute, unsigned long second, unsigned long day, unsigned long month, unsigned long year){

	clock.hour = hour;
	clock.minute = minute;
	clock.second = second;
	clock.day = day;
	clock.month = month;
	clock.year = year;

	unsigned long temp1, temp2, temp3, temp4;

	//Assign hour sprites.
	temp1 = clock.hour % 10;
	temp2 = (clock.hour % 100)/10;

	if (temp1 == 0)
		clock.hourDigitR = smallZero;
	else if (temp1 == 1)
		clock.hourDigitR = smallOne;
	else if (temp1 == 2)
		clock.hourDigitR = smallTwo;
	else if (temp1 == 3)
		clock.hourDigitR = smallThree;
	else if (temp1 == 4)
		clock.hourDigitR = smallFour;
	else if (temp1 == 5)
		clock.hourDigitR = smallFive;
	else if (temp1 == 6)
		clock.hourDigitR = smallSix;
	else if (temp1 == 7)
		clock.hourDigitR = smallSeven;
	else if (temp1 == 8)
		clock.hourDigitR = smallEight;
	else if (temp1 == 9)
		clock.hourDigitR = smallNine;

	if (temp2 == 0)
		clock.hourDigitL = smallZero;
	else if (temp2 == 1)
		clock.hourDigitL = smallOne;
	else if (temp2 == 2)
		clock.hourDigitL = smallTwo;
	else if (temp2 == 3)
		clock.hourDigitL = smallThree;
	else if (temp2 == 4)
		clock.hourDigitL = smallFour;
	else if (temp2 == 5)
		clock.hourDigitL = smallFive;
	else if (temp2 == 6)
		clock.hourDigitL = smallSix;
	else if (temp2 == 7)
		clock.hourDigitL = smallSeven;
	else if (temp2 == 8)
		clock.hourDigitL = smallEight;
	else if (temp2 == 9)
		clock.hourDigitL = smallNine;

	//Assign minute sprites.
	temp1 = clock.minute % 10;
	temp2 = (clock.minute % 100)/10;

	if (temp1 == 0)
		clock.minuteDigitR = smallZero;
	else if (temp1 == 1)
		clock.minuteDigitR = smallOne;
	else if (temp1 == 2)
		clock.minuteDigitR = smallTwo;
	else if (temp1 == 3)
		clock.minuteDigitR = smallThree;
	else if (temp1 == 4)
		clock.minuteDigitR = smallFour;
	else if (temp1 == 5)
		clock.minuteDigitR = smallFive;
	else if (temp1 == 6)
		clock.minuteDigitR = smallSix;
	else if (temp1 == 7)
		clock.minuteDigitR = smallSeven;
	else if (temp1 == 8)
		clock.minuteDigitR = smallEight;
	else if (temp1 == 9)
		clock.minuteDigitR = smallNine;

	if (temp2 == 0)
		clock.minuteDigitL = smallZero;
	else if (temp2 == 1)
		clock.minuteDigitL = smallOne;
	else if (temp2 == 2)
		clock.minuteDigitL = smallTwo;
	else if (temp2 == 3)
		clock.minuteDigitL = smallThree;
	else if (temp2 == 4)
		clock.minuteDigitL = smallFour;
	else if (temp2 == 5)
		clock.minuteDigitL = smallFive;
	else if (temp2 == 6)
		clock.minuteDigitL = smallSix;
	else if (temp2 == 7)
		clock.minuteDigitL = smallSeven;
	else if (temp2 == 8)
		clock.minuteDigitL = smallEight;
	else if (temp2 == 9)
		clock.minuteDigitL = smallNine;

	//Assign second sprites.
	temp1 = clock.second % 10;
	temp2 = (clock.second % 100)/10;

	if (temp1 == 0)
		clock.secondDigitR = smallZero;
	else if (temp1 == 1)
		clock.secondDigitR = smallOne;
	else if (temp1 == 2)
		clock.secondDigitR = smallTwo;
	else if (temp1 == 3)
		clock.secondDigitR = smallThree;
	else if (temp1 == 4)
		clock.secondDigitR = smallFour;
	else if (temp1 == 5)
		clock.secondDigitR = smallFive;
	else if (temp1 == 6)
		clock.secondDigitR = smallSix;
	else if (temp1 == 7)
		clock.secondDigitR = smallSeven;
	else if (temp1 == 8)
		clock.secondDigitR = smallEight;
	else if (temp1 == 9)
		clock.secondDigitR = smallNine;

	if (temp2 == 0)
		clock.secondDigitL = smallZero;
	else if (temp2 == 1)
		clock.secondDigitL = smallOne;
	else if (temp2 == 2)
		clock.secondDigitL = smallTwo;
	else if (temp2 == 3)
		clock.secondDigitL = smallThree;
	else if (temp2 == 4)
		clock.secondDigitL = smallFour;
	else if (temp2 == 5)
		clock.secondDigitL = smallFive;
	else if (temp2 == 6)
		clock.secondDigitL = smallSix;
	else if (temp2 == 7)
		clock.secondDigitL = smallSeven;
	else if (temp2 == 8)
		clock.secondDigitL = smallEight;
	else if (temp2 == 9)
		clock.secondDigitL = smallNine;

	//Assign day sprites.
	temp1 = clock.day % 10;
	temp2 = (clock.day % 100)/10;

	if (temp1 == 0)
		clock.dayDigitR = smallZero;
	else if (temp1 == 1)
		clock.dayDigitR = smallOne;
	else if (temp1 == 2)
		clock.dayDigitR = smallTwo;
	else if (temp1 == 3)
		clock.dayDigitR = smallThree;
	else if (temp1 == 4)
		clock.dayDigitR = smallFour;
	else if (temp1 == 5)
		clock.dayDigitR = smallFive;
	else if (temp1 == 6)
		clock.dayDigitR = smallSix;
	else if (temp1 == 7)
		clock.dayDigitR = smallSeven;
	else if (temp1 == 8)
		clock.dayDigitR = smallEight;
	else if (temp1 == 9)
		clock.dayDigitR = smallNine;

	if (temp2 == 0)
		clock.dayDigitL = smallZero;
	else if (temp2 == 1)
		clock.dayDigitL = smallOne;
	else if (temp2 == 2)
		clock.dayDigitL = smallTwo;
	else if (temp2 == 3)
		clock.dayDigitL = smallThree;
	else if (temp2 == 4)
		clock.dayDigitL = smallFour;
	else if (temp2 == 5)
		clock.dayDigitL = smallFive;
	else if (temp2 == 6)
		clock.dayDigitL = smallSix;
	else if (temp2 == 7)
		clock.dayDigitL = smallSeven;
	else if (temp2 == 8)
		clock.dayDigitL = smallEight;
	else if (temp2 == 9)
		clock.dayDigitL = smallNine;

	//Assign month sprites.
	temp1 = clock.month % 10;
	temp2 = (clock.month % 100)/10;

	if (temp1 == 0)
		clock.monthDigitR = smallZero;
	else if (temp1 == 1)
		clock.monthDigitR = smallOne;
	else if (temp1 == 2)
		clock.monthDigitR = smallTwo;
	else if (temp1 == 3)
		clock.monthDigitR = smallThree;
	else if (temp1 == 4)
		clock.monthDigitR = smallFour;
	else if (temp1 == 5)
		clock.monthDigitR = smallFive;
	else if (temp1 == 6)
		clock.monthDigitR = smallSix;
	else if (temp1 == 7)
		clock.monthDigitR = smallSeven;
	else if (temp1 == 8)
		clock.monthDigitR = smallEight;
	else if (temp1 == 9)
		clock.monthDigitR = smallNine;

	if (temp2 == 0)
		clock.monthDigitL = smallZero;
	else if (temp2 == 1)
		clock.monthDigitL = smallOne;
	else if (temp2 == 2)
		clock.monthDigitL = smallTwo;
	else if (temp2 == 3)
		clock.monthDigitL = smallThree;
	else if (temp2 == 4)
		clock.monthDigitL = smallFour;
	else if (temp2 == 5)
		clock.monthDigitL = smallFive;
	else if (temp2 == 6)
		clock.monthDigitL = smallSix;
	else if (temp2 == 7)
		clock.monthDigitL = smallSeven;
	else if (temp2 == 8)
		clock.monthDigitL = smallEight;
	else if (temp2 == 9)
		clock.monthDigitL = smallNine;

	//Assign year sprites.
	temp1 = clock.year % 10;
	temp2 = (clock.year % 100)/10;
	temp3 = (clock.year % 1000)/100;
	temp4 = (clock.year % 10000)/1000;

	if (temp1 == 0)
		clock.yearDigitRM = smallZero;
	else if (temp1 == 1)
		clock.yearDigitRM = smallOne;
	else if (temp1 == 2)
		clock.yearDigitRM = smallTwo;
	else if (temp1 == 3)
		clock.yearDigitRM = smallThree;
	else if (temp1 == 4)
		clock.yearDigitRM = smallFour;
	else if (temp1 == 5)
		clock.yearDigitRM = smallFive;
	else if (temp1 == 6)
		clock.yearDigitRM = smallSix;
	else if (temp1 == 7)
		clock.yearDigitRM = smallSeven;
	else if (temp1 == 8)
		clock.yearDigitRM = smallEight;
	else if (temp1 == 9)
		clock.yearDigitRM = smallNine;

	if (temp2 == 0)
		clock.yearDigitR = smallZero;
	else if (temp2 == 1)
		clock.yearDigitR = smallOne;
	else if (temp2 == 2)
		clock.yearDigitR = smallTwo;
	else if (temp2 == 3)
		clock.yearDigitR = smallThree;
	else if (temp2 == 4)
		clock.yearDigitR = smallFour;
	else if (temp2 == 5)
		clock.yearDigitR = smallFive;
	else if (temp2 == 6)
		clock.yearDigitR = smallSix;
	else if (temp2 == 7)
		clock.yearDigitR = smallSeven;
	else if (temp2 == 8)
		clock.yearDigitR = smallEight;
	else if (temp2 == 9)
		clock.yearDigitR = smallNine;

	if (temp3 == 0)
		clock.yearDigitL = smallZero;
	else if (temp3 == 1)
		clock.yearDigitL = smallOne;
	else if (temp3 == 2)
		clock.yearDigitL = smallTwo;
	else if (temp3 == 3)
		clock.yearDigitL = smallThree;
	else if (temp3 == 4)
		clock.yearDigitL = smallFour;
	else if (temp3 == 5)
		clock.yearDigitL = smallFive;
	else if (temp3 == 6)
		clock.yearDigitL = smallSix;
	else if (temp3 == 7)
		clock.yearDigitL = smallSeven;
	else if (temp3 == 8)
		clock.yearDigitL = smallEight;
	else if (temp3 == 9)
		clock.yearDigitL = smallNine;

	if (temp4 == 0)
		clock.yearDigitLM = smallZero;
	else if (temp4 == 1)
		clock.yearDigitLM = smallOne;
	else if (temp4 == 2)
		clock.yearDigitLM = smallTwo;
	else if (temp4 == 3)
		clock.yearDigitLM = smallThree;
	else if (temp4 == 4)
		clock.yearDigitLM = smallFour;
	else if (temp4 == 5)
		clock.yearDigitLM = smallFive;
	else if (temp4 == 6)
		clock.yearDigitLM = smallSix;
	else if (temp4 == 7)
		clock.yearDigitLM = smallSeven;
	else if (temp4 == 8)
		clock.yearDigitLM = smallEight;
	else if (temp4 == 9)
		clock.yearDigitLM = smallNine;

	return;
}

void highlightOption(unsigned short menuOption){
	drawRectangle(0, 0, 1024, 768, 0); //Screen wipe.
	if (menuOption == 1){
		highlight.h1 = 1;
		highlight.h2 = 0;
		highlight.h3 = 0;
		highlight.h4 = 0;
		highlight.h5 = 0;
		highlight.h6 = 0;
		highlight.h7 = 0;
		highlight.h8 = 0;
	}
	else if (menuOption == 2){
		highlight.h1 = 0;
		highlight.h2 = 1;
		highlight.h3 = 0;
		highlight.h4 = 0;
		highlight.h5 = 0;
		highlight.h6 = 0;
		highlight.h7 = 0;
		highlight.h8 = 0;
	}
	else if (menuOption == 3){
		highlight.h1 = 0;
		highlight.h2 = 0;
		highlight.h3 = 1;
		highlight.h4 = 0;
		highlight.h5 = 0;
		highlight.h6 = 0;
		highlight.h7 = 0;
		highlight.h8 = 0;
	}
	else if (menuOption == 4){
		highlight.h1 = 0;
		highlight.h2 = 0;
		highlight.h3 = 0;
		highlight.h4 = 1;
		highlight.h5 = 0;
		highlight.h6 = 0;
		highlight.h7 = 0;
		highlight.h8 = 0;
	}
	else if (menuOption == 5){
		highlight.h1 = 0;
		highlight.h2 = 0;
		highlight.h3 = 0;
		highlight.h4 = 0;
		highlight.h5 = 1;
		highlight.h6 = 0;
		highlight.h7 = 0;
		highlight.h8 = 0;
	}
	else if (menuOption == 6){
		highlight.h1 = 0;
		highlight.h2 = 0;
		highlight.h3 = 0;
		highlight.h4 = 0;
		highlight.h5 = 0;
		highlight.h6 = 1;
		highlight.h7 = 0;
		highlight.h8 = 0;
	}
	else if (menuOption == 7){
		highlight.h1 = 0;
		highlight.h2 = 0;
		highlight.h3 = 0;
		highlight.h4 = 0;
		highlight.h5 = 0;
		highlight.h6 = 0;
		highlight.h7 = 1;
		highlight.h8 = 0;
	}
	else if (menuOption == 8){
		highlight.h1 = 0;
		highlight.h2 = 0;
		highlight.h3 = 0;
		highlight.h4 = 0;
		highlight.h5 = 0;
		highlight.h6 = 0;
		highlight.h7 = 0;
		highlight.h8 = 1;
	}
	else if (menuOption == 9){
		highlight.h1 = 0;
		highlight.h2 = 0;
		highlight.h3 = 0;
		highlight.h4 = 0;
		highlight.h5 = 0;
		highlight.h6 = 0;
		highlight.h7 = 0;
		highlight.h8 = 0;
	}
	return;
}
void cycleAlphabet(int iterator, char direction){
	if (iterator == 5){
		if (direction == 'U'){
			if (fontcycle.iterator1 == 0){
				fontcycle.iterator1 = 25;
			}else{
				fontcycle.iterator1--;
			}
		}
		else if (direction == 'D'){
			if (fontcycle.iterator1 == 25){
				fontcycle.iterator1 = 0;
			}else{
				fontcycle.iterator1++;
			}
		}
	}
	else if (iterator == 6){
		if (direction == 'U'){
			if (fontcycle.iterator2 == 0){
				fontcycle.iterator2 = 25;
			}else{
				fontcycle.iterator2--;
			}
		}
		else if (direction == 'D'){
			if (fontcycle.iterator2 == 25){
				fontcycle.iterator2 = 0;
			}else{
				fontcycle.iterator2++;
			}
		}
	}
	else if (iterator == 7){
		if (direction == 'U'){
			if (fontcycle.iterator3 == 0){
				fontcycle.iterator3 = 25;
			}else{
				fontcycle.iterator3--;
			}
		}
		else if (direction == 'D'){
			if (fontcycle.iterator3 == 25){
				fontcycle.iterator3 = 0;
			}else{
				fontcycle.iterator3++;
			}
		}
	}
	return;
}

void loadHighscores(){
	char c;
	FILE *file;
	file = fopen("home/lcom/lcom1617-t1g13/brickbreaker/src/highscores.txt", "rt");

	int i;

	for (i = 0; i < 8; i++){

		c = getc(file);
		allHighscores[i].alpha1 = c;
		allHighscores[i].alpha1sprite = alphanumericMatch(c);

		c = getc(file);
		allHighscores[i].alpha2 = c;
		allHighscores[i].alpha2sprite = alphanumericMatch(c);

		c = getc(file);
		allHighscores[i].alpha3 = c;
		allHighscores[i].alpha3sprite = alphanumericMatch(c);

		c = getc(file); //Hifen.

		c = getc(file);
		int x = c - '0';
		allHighscores[i].value = 0;
		allHighscores[i].value += (x * 100);
		allHighscores[i].digit2sprite = alphanumericMatch(c);

		c = getc(file);
		x = c - '0';
		allHighscores[i].value += (x * 10);
		allHighscores[i].digit1sprite = alphanumericMatch(c);

		c = getc(file);
		x = c - '0';
		allHighscores[i].value += x;
		allHighscores[i].digit0sprite = alphanumericMatch(c);

		c = getc(file); //Hifen.

		c = getc(file);
		x = c - '0';
		allHighscores[i].date = 0;
		allHighscores[i].date += (x * 1000);
		allHighscores[i].day1sprite = alphanumericMatch(c);

		c = getc(file);
		x = c - '0';
		allHighscores[i].date += (x * 100);
		allHighscores[i].day0sprite = alphanumericMatch(c);

		c = getc(file);
		x = c - '0';
		allHighscores[i].date += (x * 10);
		allHighscores[i].month1sprite = alphanumericMatch(c);

		c = getc(file);
		x = c - '0';
		allHighscores[i].date += x;
		allHighscores[i].month0sprite = alphanumericMatch(c);

		c = getc(file); //Space.
	}
	fclose(file);
}
void loadSprites(){

	/* BRICKS */
	short brickCounter = 0;

	int i, j;
	for (i = 0; i < 6; i++){
		for (j = 0; j < 16; j++){
			Brick brick;
			if (j < 15)
				brick.x = 32 + (BRICK_WIDTH / 2) + BRICK_WIDTH * j;
			else
				brick.x = 32 + (BRICK_WIDTH / 2) + BRICK_WIDTH * j - 4;
			brick.y = 215 + i * BRICK_HEIGHT;

			if (i == 0)
				brick.sprite = pinkBrick;
			else if (i == 1)
				brick.sprite = redBrick;
			else if (i == 2)
				brick.sprite = orangeBrick;
			else if (i == 3)
				brick.sprite = yellowBrick;
			else if (i == 4)
				brick.sprite = greenBrick;
			else if (i == 5)
				brick.sprite = blueBrick;

			brick.visible = 1;
			allBricks[brickCounter] = brick;
			brickCounter++;
		}
	}

	/* PADDLE */
	paddle.x = 442; paddle.y = 680;
	paddle.speed = 10;
	paddle.sprite = paddleSprite;

	/* BALL */
	ball.x = 400; ball.y = 600;
	ball.speedx = 2; ball.speedy = 2;
	ball.dir = rand() % 4 + 1;
	ball.sprite = ballSprite;

	/* CURSOR */
	cursor.x = 900;
	cursor.y = 600;
	cursor.sprite = mouseCursor;
	cursor.sensitivity = 0.15;

	/* GUI */
	score.digit0 = numberZero; score.digit1 = numberZero; score.digit2 = numberZero;
	score.value = 0;

	timer.digit0 = numberZero; timer.digit1 = numberZero; timer.digit2 = numberZero; timer.digit3 = numberZero;
	timer.seconds = 0; timer.minutes = 0;

	/* POWERUP */
	powerup.dropping = 0;
	powerup.active = 0;

	/* HIGHLIGHTS */
	highlight.h1 = 1; highlight.h2 = 1; highlight.h3 = 1;

	/* HIGHSCORES */
	int index;
	for (index = 0; index < 8; index++){
		allHighscores[index].alpha1 = 'X';
		allHighscores[index].alpha2 = 'X';
		allHighscores[index].alpha3 = 'X';
		allHighscores[index].alpha1sprite = xFont;
		allHighscores[index].alpha2sprite = xFont;
		allHighscores[index].alpha3sprite = xFont;
		allHighscores[index].value = 0;
		allHighscores[index].digit0sprite = n0Font;
		allHighscores[index].digit1sprite = n0Font;
		allHighscores[index].digit2sprite = n0Font;
		allHighscores[index].day0sprite = n0Font;
		allHighscores[index].day1sprite = n0Font;
		allHighscores[index].month0sprite = n0Font;
		allHighscores[index].month1sprite = n0Font;
	}

	/* ALPHABET */
	alphabet[0] = 'A'; alphabet[1] = 'B'; alphabet[2] = 'C'; alphabet[3] = 'D'; alphabet[4] = 'E';
	alphabet[5] = 'F'; alphabet[6] = 'G'; alphabet[7] = 'H'; alphabet[8] = 'I'; alphabet[9] = 'J';
	alphabet[10] = 'K'; alphabet[11] = 'L'; alphabet[12] = 'M'; alphabet[13] = 'N'; alphabet[14] = 'O';
	alphabet[15] = 'P'; alphabet[16] = 'Q'; alphabet[17] = 'R'; alphabet[18] = 'S'; alphabet[19] = 'T';
	alphabet[20] = 'U'; alphabet[21] = 'V'; alphabet[22] = 'W'; alphabet[23] = 'X'; alphabet[24] = 'Y';
	alphabet[25] = 'Z';
	fontcycle.iterator1 = 0;
	fontcycle.iterator2 = 0;
	fontcycle.iterator3 = 0;

	/* CLOCK */
	clock.hour = 0; clock.hourDigitL = smallZero; clock.hourDigitR = smallZero;
	clock.minute = 0; clock.minuteDigitL = smallZero; clock.minuteDigitR = smallZero;
	clock.second = 0; clock.secondDigitL = smallZero; clock.secondDigitR = smallZero;
	clock.day = 0; clock.dayDigitL = smallZero; clock.dayDigitR = smallZero;
	clock.month = 0; clock.monthDigitL = smallZero; clock.monthDigitR = smallZero;
	clock.year = 0; clock.yearDigitLM = smallZero; clock.yearDigitL = smallZero; clock.yearDigitR = smallZero; clock.yearDigitRM = smallZero;

	/* FLOW */
	flow.multiplier = 1;
	flow.powDropSpeed = 1;
	flow.powDropRate = 6; // 33% chance of drop.
}

int getBrickSize(){
	int i;
	i = sizeof(allBricks) / sizeof(int);
	return i;
}

void updateTimer(){
	timer.seconds++;
	if (timer.seconds == 60){
		timer.seconds = 0;
		timer.minutes++;
	}

	int temp0 = timer.seconds % 10;
	int temp1 = (timer.seconds % 100)/10;
	int temp2 = timer.minutes % 10;
	int temp3 = (timer.minutes % 100)/10;

	if (temp0 == 0){
		timer.digit0 = numberZero;
	}
	else if (temp0 == 1){
		timer.digit0 = numberOne;
	}
	else if (temp0 == 2){
		timer.digit0 = numberTwo;
	}
	else if (temp0 == 3){
		timer.digit0 = numberThree;
	}
	else if (temp0 == 4){
		timer.digit0 = numberFour;
	}
	else if (temp0 == 5){
		timer.digit0 = numberFive;
	}
	else if (temp0 == 6){
		timer.digit0 = numberSix;
	}
	else if (temp0 == 7){
		timer.digit0 = numberSeven;
	}
	else if (temp0 == 8){
		timer.digit0 = numberEight;
	}
	else if (temp0 == 9){
		timer.digit0 = numberNine;
	}

	if (temp1 == 0){
		timer.digit1 = numberZero;
	}
	else if (temp1 == 1){
		timer.digit1 = numberOne;
	}
	else if (temp1 == 2){
		timer.digit1 = numberTwo;
	}
	else if (temp1 == 3){
		timer.digit1 = numberThree;
	}
	else if (temp1 == 4){
		timer.digit1 = numberFour;
	}
	else if (temp1 == 5){
		timer.digit1 = numberFive;
	}
	else if (temp1 == 6){
		timer.digit1 = numberSix;
	}
	else if (temp1 == 7){
		timer.digit1 = numberSeven;
	}
	else if (temp1 == 8){
		timer.digit1 = numberEight;
	}
	else if (temp1 == 9){
		timer.digit1 = numberNine;
	}

	if (temp2 == 0){
		timer.digit2 = numberZero;
	}
	else if (temp2 == 1){
		timer.digit2 = numberOne;
	}
	else if (temp2 == 2){
		timer.digit2 = numberTwo;
	}
	else if (temp2 == 3){
		timer.digit2 = numberThree;
	}
	else if (temp2 == 4){
		timer.digit2 = numberFour;
	}
	else if (temp2 == 5){
		timer.digit2 = numberFive;
	}
	else if (temp2 == 6){
		timer.digit2 = numberSix;
	}
	else if (temp2 == 7){
		timer.digit2 = numberSeven;
	}
	else if (temp2 == 8){
		timer.digit2 = numberEight;
	}
	else if (temp2 == 9){
		timer.digit2 = numberNine;
	}
	if (temp3 == 0){
		timer.digit3 = numberZero;
	}
	else if (temp3 == 1){
		timer.digit3 = numberOne;
	}
	else if (temp3 == 2){
		timer.digit3 = numberTwo;
	}
	else if (temp3 == 3){
		timer.digit3 = numberThree;
	}
	else if (temp3 == 4){
		timer.digit3 = numberFour;
	}
	else if (temp3 == 5){
		timer.digit3 = numberFive;
	}
	else if (temp3 == 6){
		timer.digit3 = numberSix;
	}
	else if (temp3 == 7){
		timer.digit3 = numberSeven;
	}
	else if (temp3 == 8){
		timer.digit3 = numberEight;
	}
	else if (temp3 == 9){
		timer.digit3 = numberNine;
	}
}

void updateScore(){
	score.value += flow.multiplier;

	int temp0 = score.value % 10;
	int temp1 = (score.value % 100)/10;
	int temp2 = (score.value % 1000)/100;

	if (temp0 == 0){
		score.digit0 = numberZero;
	}
	else if (temp0 == 1){
		score.digit0 = numberOne;
	}
	else if (temp0 == 2){
		score.digit0 = numberTwo;
	}
	else if (temp0 == 3){
		score.digit0 = numberThree;
	}
	else if (temp0 == 4){
		score.digit0 = numberFour;
	}
	else if (temp0 == 5){
		score.digit0 = numberFive;
	}
	else if (temp0 == 6){
		score.digit0 = numberSix;
	}
	else if (temp0 == 7){
		score.digit0 = numberSeven;
	}
	else if (temp0 == 8){
		score.digit0 = numberEight;
	}
	else if (temp0 == 9){
		score.digit0 = numberNine;
	}

	if (temp1 == 0){
		score.digit1 = numberZero;
	}
	else if (temp1 == 1){
		score.digit1 = numberOne;
	}
	else if (temp1 == 2){
		score.digit1 = numberTwo;
	}
	else if (temp1 == 3){
		score.digit1 = numberThree;
	}
	else if (temp1 == 4){
		score.digit1 = numberFour;
	}
	else if (temp1 == 5){
		score.digit1 = numberFive;
	}
	else if (temp1 == 6){
		score.digit1 = numberSix;
	}
	else if (temp1 == 7){
		score.digit1 = numberSeven;
	}
	else if (temp1 == 8){
		score.digit1 = numberEight;
	}
	else if (temp1 == 9){
		score.digit1 = numberNine;
	}

	if (temp2 == 0){
		score.digit2 = numberZero;
	}
	else if (temp2 == 1){
		score.digit2 = numberOne;
	}
	else if (temp2 == 2){
		score.digit2 = numberTwo;
	}
	else if (temp2 == 3){
		score.digit2 = numberThree;
	}
	else if (temp2 == 4){
		score.digit2 = numberFour;
	}
	else if (temp2 == 5){
		score.digit2 = numberFive;
	}
	else if (temp2 == 6){
		score.digit2 = numberSix;
	}
	else if (temp2 == 7){
		score.digit2 = numberSeven;
	}
	else if (temp2 == 8){
		score.digit2 = numberEight;
	}
	else if (temp2 == 9){
		score.digit2 = numberNine;
	}
}

void firstRender(){
	if (sm.state == GAME_MENU){
		render.ball = 0;
		render.ball2 = 0;
		render.paddle = 0;
		render.bricks = 0;
		render.boundaries = 0;
		render.cursor = 1;
		render.gui = 0;
		render.logos = 1;
		render.powerup = 0;
		render.highscores = 0;
		render.highlights = 1;
		render.submit = 0;
		render.clock = 1;
	}
	if (sm.state == GAME_HIGHSCORES){
		render.ball = 0;
		render.ball2 = 0;
		render.paddle = 0;
		render.bricks = 0;
		render.boundaries = 0;
		render.cursor = 1;
		render.gui = 0;
		render.logos = 0;
		render.powerup = 0;
		render.highscores = 1;
		render.highlights = 1;
		render.submit = 0;
		render.clock = 0;
	}
	if (sm.state == GAME_RUN){
		render.ball = 1;
		render.paddle = 1;
		render.bricks = 1;
		render.boundaries = 1;
		render.cursor = 0;
		render.gui = 1;
		render.logos = 0;
		render.powerup = 1;
		render.highscores = 0;
		render.highlights = 0;
		render.submit = 0;
		render.clock = 0;
	}
	if (sm.state == GAME_SUBMIT_SCORE){
		render.ball = 0;
		render.paddle = 0;
		render.bricks = 0;
		render.boundaries = 0;
		render.cursor = 1;
		render.gui = 0;
		render.logos = 0;
		render.powerup = 0;
		render.highscores = 0;
		render.highlights = 1;
		render.submit = 1;
		render.clock = 0;
	}
	return;
}

void renderFrame(){
	if (render.ball == 1)
		displayXPM(ball.x - BALL_RADIUS, ball.y - BALL_RADIUS, ball.sprite);

	if (render.ball2 == 1)
		displayXPM(ball2.x - BALL_RADIUS, ball2.y - BALL_RADIUS, ball2.sprite);

	if (render.paddle == 1)
		displayXPM(paddle.x, paddle.y, paddle.sprite);

	if (render.bricks == 1){
		int i;
		for (i = 0; i < 96; i++){
			if (allBricks[i].visible == 1){
				displayXPM(allBricks[i].x - (BRICK_WIDTH / 2), allBricks[i].y - (BRICK_HEIGHT / 2), allBricks[i].sprite);
			}
		}
	}

	if (render.boundaries == 1){
		drawRectangle(0, 100, 1024, 32, 56);	//Upper boundary.
		drawRectangle(0, 100, 32, 678, 56); 	//Left boundary.
		drawRectangle(988, 100, 36, 678, 56);	//Right boundary.
	}

	if (render.cursor == 1)
		displayXPM(cursor.x, cursor.y, cursor.sprite);

	if (render.gui == 1){
		displayXPM(20, 20, scoreSprite);
		displayXPM(160, 20, score.digit2);
		displayXPM(230, 20, score.digit1);
		displayXPM(300, 20, score.digit0);

		displayXPM(650, 20, timer.digit3);
		displayXPM(720, 20, timer.digit2);
		displayXPM(792, 40, colon);
		displayXPM(810, 20, timer.digit1);
		displayXPM(880, 20, timer.digit0);
	}

	if (render.logos == 1){
		displayXPM(400, 50, aturi);
		displayXPM(360, 270, brickiet);
		drawRectangle(385, 275, 140, 3, 63);
		drawRectangle(545, 275, 25, 3, 63);
		drawRectangle(590, 275, 53, 3, 63);

		displayXPM(380, 500, playMenu);
		displayXPM(290, 600, highscoresMenu);
		displayXPM(425, 675, exitMenu);
	}

	if (render.highlights == 1){
		if (highlight.h1 == 1){
			drawRectangle(380, 485, 282, 5, 63);
			drawRectangle(380, 570, 282, 5, 63);
		}
		else if (highlight.h2 == 1){
			drawRectangle(290, 585, 476, 5, 63);
			drawRectangle(290, 650, 476, 5, 63);
		}
		else if (highlight.h3 == 1){
			drawRectangle(425, 660, 188, 5, 63);
			drawRectangle(425, 725, 188, 5, 63);
		}
		else if (highlight.h4 == 1){
			drawRectangle(300, 615, 396, 5, 63);
			drawRectangle(300, 725, 396, 5, 63);
		}
		else if (highlight.h5 == 1){
			displayXPM(406, 340, upArrow); displayXPM(406, 435, downArrow);
		}
		else if (highlight.h6 == 1){
			displayXPM(506, 340, upArrow); displayXPM(506, 435, downArrow);
		}
		else if (highlight.h7 == 1){
			displayXPM(606, 340, upArrow); displayXPM(606, 435, downArrow);
		}
		else if (highlight.h8 == 1){
			drawRectangle(355, 590, 355, 5, 63);
			drawRectangle(355, 660, 355, 5, 63);
		}
	}

	if (render.powerup == 1){
		int i;
		for (i = 0; i < 5; i++){
			if (allPowerups[i].dropping == 1){
				clearSprite(allPowerups[i].x, allPowerups[i].y, allPowerups[i].sprite);
				allPowerups[i].y += flow.powDropSpeed;
				displayXPM(allPowerups[i].x, allPowerups[i].y, allPowerups[i].sprite);
			}
		}
	}
	if (render.highscores == 1){
		int i, yOffset = 60;
		for (i = 0; i < 8; i++){
			if (i == 0){
				displayXPM(70, 50 + yOffset * i, n1Font);
			}
			else if (i == 1){
				displayXPM(70, 50 + yOffset * i, n2Font);
			}
			else if (i == 2){
				displayXPM(70, 50 + yOffset * i, n3Font);
			}
			else if (i == 3){
				displayXPM(70, 50 + yOffset * i, n4Font);
			}
			else if (i == 4){
				displayXPM(70, 50 + yOffset * i, n5Font);
			}
			else if (i == 5){
				displayXPM(70, 50 + yOffset * i, n6Font);
			}
			else if (i == 6){
				displayXPM(70, 50 + yOffset * i, n7Font);
			}
			else if (i == 7){
				displayXPM(70, 50 + yOffset * i, n8Font);
			}
			displayXPM(130, 90 + yOffset * i, dot);

			displayXPM(170, 50 + yOffset * i, allHighscores[i].alpha1sprite);
			displayXPM(230, 50 + yOffset * i, allHighscores[i].alpha2sprite);
			displayXPM(290, 50 + yOffset * i, allHighscores[i].alpha3sprite);
			displayXPM(410, 50 + yOffset * i, allHighscores[i].digit2sprite);
			displayXPM(470, 50 + yOffset * i, allHighscores[i].digit1sprite);
			displayXPM(530, 50 + yOffset * i, allHighscores[i].digit0sprite);
			displayXPM(660, 50 + yOffset * i, allHighscores[i].day1sprite);
			displayXPM(720, 50 + yOffset * i, allHighscores[i].day0sprite);
			displayXPM(780, 50 + yOffset * i, slash);
			displayXPM(840, 50 + yOffset * i, allHighscores[i].month1sprite);
			displayXPM(900, 50 + yOffset * i, allHighscores[i].month0sprite);
		}
		displayXPM(300, 630, returnToMainMenu);
	}
	if (render.submit == 1){
		displayXPM(135, 200, newHighscore);

		displayXPM(400, 375, alphanumericMatch(alphabet[fontcycle.iterator1]));
		displayXPM(500, 375, alphanumericMatch(alphabet[fontcycle.iterator2]));
		displayXPM(600, 375, alphanumericMatch(alphabet[fontcycle.iterator3]));

		displayXPM(355, 600, submitHighscore);
	}
	if (render.clock == 1){
		displayXPM(800, 20, clock.hourDigitL);
		displayXPM(825, 20, clock.hourDigitR);
		displayXPM(857, 23, smallColon);
		displayXPM(875, 20, clock.minuteDigitL);
		displayXPM(900, 20, clock.minuteDigitR);
		displayXPM(927, 23, smallColon);
		displayXPM(950, 20, clock.secondDigitL);
		displayXPM(975, 20, clock.secondDigitR);
	}
}
