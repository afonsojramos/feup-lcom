#ifndef _I8042_H_
#define _I8042_H_

#define MOUSE_IRQ 				12
#define STATUS_REG              0x64
#define OUT_BUF					0x60
#define IN_BUF					0x64
#define MOUSE_ENABLE			0xA8
#define MOUSE_ENABLE_DATA		0xF4
#define MOUSE_DISABLE_DATA		0xF5
#define SET_STREAM_MODE 		0xEA
#define STATUS_REQUEST			0xE9
#define OBF						BIT(0)
#define BYTE_TO_MOUSE			0xD4
#define DEFAULT_WAIT			20000
#define BIT(n) 					(0x01<<(n))

#define LEFT_BUTTON 			BIT(0)
#define RIGHT_BUTTON 			BIT(1)
#define MIDDLE_BUTTON 			BIT(2)
#define X_SIGN 					BIT(4)
#define Y_SIGN 					BIT(5)
#define X_OVERFLOW 				BIT(6)
#define Y_OVERFLOW 				BIT(7)

#endif
