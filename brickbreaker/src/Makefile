# Makefile for Brick Breaker

COMPILER_TYPE= gnu

CC= gcc

PROG= brickbreaker
SRCS= breaker.c video_gr.c vbe.c read_xpm.c timer.c kbd.c mouse.c rtc.c

CCFLAGS= -Wall

DPADD+= ${LIBDRIVER} ${LIBSYS}
LDADD+= -ldriver -lsys

LDFLAGS+= -L .

MAN=

.include <bsd.prog.mk>
.include <bsd.gcc.mk>