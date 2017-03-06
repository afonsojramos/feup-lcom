#include "mouse.h"

int test_packet(unsigned short cnt){
	mouse_test_packet(cnt);
}

int test_async(unsigned short idle_time) {
	mouse_test_async(idle_time);
}

int test_config(void) {
    mouse_test_config();
}

int test_gesture(short length) {
    mouse_test_gesture(length);
}
