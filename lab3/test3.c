#include "kbd.h"

int kbd_test_scan(unsigned short ass) {

	if (kbd_scan_codes(ass) == 1)
		return 1;

	printf("\n> KBD_TEST_SCAN COMPLETED <\n");
	return 0;

}
int kbd_test_leds(unsigned short n, unsigned short *leds) {

	unsigned short lights[] = {0, 0, 0};
	int arrayPos = 0;

	printf("\n> KBD_TEST_LEDS STARTED <\n");
	while (arrayPos < n){
		kbd_led_test(lights, leds[arrayPos]);
		arrayPos++;

	}
	printf("\n\n> KBD_TEST_LEDS COMPLETED <\n");
	return 0;
}
int kbd_test_timed_scan(unsigned short n) {
	if (itrpScan(n) == 1)
			return 1;

	printf("\n> KBD_TEST_TIMED_SCAN COMPLETED <\n");
	return 0;
}
