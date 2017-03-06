#include <minix/syslib.h>
#include <minix/drivers.h>
#include <machine/int86.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "vbe.h"
#include "video_gr.h"
#include "pixmap.h"
#include "read_xpm.h"

/* Constants for VBE 0x105 mode */

/* The physical address may vary from VM to VM.
 * At one time it was 0xD0000000
 *  #define VRAM_PHYS_ADDR    0xD0000000 
 * Currently on lab B107 is 0xF0000000
 * Better run my version of lab5 as follows:
 *     service run `pwd`/lab5 -args "mode 0x105"
 */
#define VRAM_PHYS_ADDR	  0xE0000000
#define H_RES             1280
#define V_RES			  1024
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
int drawSquare(unsigned short x, unsigned short y, unsigned short size, unsigned long color){

	int i, j;
	for (i = 0; i < size; i++){
		for (j = 0; j < size; j++){
			switchPixel(x + i, y + j, color);
		}
	}

	return 0;
}
int drawLine(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, unsigned long color){
	//Fonte algoritmo: https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C.
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
	int err = (dx>dy ? dx : -dy)/2;
	int e2;

	while(1){
		switchPixel(x0, y0, color);
		if (x0==x1 && y0==y1) break;
		e2 = err;
		if (e2 >-dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
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
void clearScreen(unsigned short xi, unsigned short yi, char *xpm[]){
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
int moveSprite(unsigned short xi, unsigned short yi, char *xpm[], unsigned short hor, short delta, unsigned short time){
	int timeEachPixel = time * 60 / delta;
	int timeElapsed = 0;

	displayXPM(xi, yi, xpm);
	while (timeElapsed < time * 60){
		int tmpx = xi, tmpy = yi;
		if (hor == 1){
			if (delta > 0)
				xi++;
			else
				xi--;
		}
		else if (hor == 0){
			if (delta > 0)
				yi++;
			else
				yi--;
		}
		if (waitInterrupts(timeEachPixel) == 2){
			clearScreen(tmpx, tmpy, xpm);
			break;
		}
		clearScreen(tmpx, tmpy, xpm);
		displayXPM(xi, yi, xpm);
		timeElapsed += timeEachPixel;
	}
	return 0;
}
