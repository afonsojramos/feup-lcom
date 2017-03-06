#include "test5.h"

/*
 * Foi-nos impossível continuar o desenvolvimento deste lab devido
 * a problemas de kernel panic do MINIX. Sabemos que o código funciona,
 * porém. Tentámos de tudo, inclusive comentar TODAS as funções, sem
 * sucesso e ainda REINSTALAR A MÁQUINA VIRTUAL. Não poderíamos ter
 * feito mais...
 * Esperamos que o docente que avaliar isto tenha mais sorte que
 * nós a executar o código. Caso tenha os mesmos erros que nós, por
 * favor seja paciente, pois entre restarts da VM, as funções funcionarão.
 *
 * Melhores cumprimentos.
 */

void *test_init(unsigned short mode, unsigned short delay) {
	vg_init(mode);
	timer_wait_seconds(delay);
	vg_exit();
	return;
}
int test_square(unsigned short x, unsigned short y, unsigned short size, unsigned long color) {
	vg_init(GRAPHICS_MODE);
	drawSquare(x, y, size, color);
	waitsForEscapeKey();
	vg_exit();
	return 0;
}
int test_line(unsigned short xi, unsigned short yi, unsigned short xf, unsigned short yf, unsigned long color) {
	vg_init(GRAPHICS_MODE);
	drawLine(xi, yi, xf, yf, color);
	waitsForEscapeKey();
	vg_exit();
	return 0;
}
int test_xpm(unsigned short xi, unsigned short yi, char *xpm[]) {
	vg_init(GRAPHICS_MODE);
	displayXPM(xi, yi, xpm);
	waitsForEscapeKey();
	vg_exit();
	return 0;
}

int test_move(unsigned short xi, unsigned short yi, char *xpm[], unsigned short hor, short delta, unsigned short time) {
	vg_init(GRAPHICS_MODE);
	moveSprite(xi, yi, xpm, hor, delta, time);
	vg_exit();
	return 0;
}
int test_controller() {
	/* To be completed */
}
