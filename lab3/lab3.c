#include "timer.h"
#include "i8254.h"
#include <limits.h>
#include <string.h>
#include <errno.h>

static int proc_args(int argc, char **argv);
static unsigned long parse_ulong(char *str, int base);
static void print_usage(char **argv);

//Completed.
int main(int argc, char **argv)
{
	sef_startup();
	sys_enable_iop();

	if (argc == 1) {
		print_usage(argv);
		return 0;
	}
	else return proc_args(argc, argv);
}

//Completed.
static void print_usage(char **argv)
{
	printf("Usage: one of the following:\n"
			"\t service run %s -args \"scan [unsigned short ass]\"\n"
			"\t service run %s -args \"leds [array *leds]\"\n"
			"\t service run %s -args \"timed_scan [unsigned short n]\"\n",
			argv[0], argv[0], argv[0]);
}

//Completed.
static int proc_args(int argc, char **argv)
{
	//Argument declarations
	unsigned short ass, size, *leds;
	unsigned long leftBits, rightBits;

	//kbd_test_scan
	if (strncmp(argv[1], "scan", strlen("scan")) == 0) {
		if (argc != 3) {
			printf("kbd: wrong no. of arguments for kbd_test_scan()\n");
			return 1;
		}
		//ass = parse_ulong(argv[2], 10);						/* Parses string to unsigned long */
		//if (ass == ULONG_MAX)
			//return 1;
		printf("kbd::kbd_test_scan(%hu)\n", ass);
		kbd_test_scan(ass);
		return 0;
	}

	//kbd_test_leds
	else if (strncmp(argv[1], "leds", strlen("leds")) == 0) {
		if (argc < 4) {
			printf("kbd: wrong no. of arguments for kbd_test_leds()\n");
			return 1;
		}
		if ((leftBits = parse_ulong(argv[2], 10)) == ULONG_MAX){
			return 1;
		}
		if ((rightBits = parse_ulong(argv[3], 10)) == ULONG_MAX){
			return 1;
		}
		printf("kbd::kbd_test_leds(%d, [", leftBits);
		unsigned short ledArray[leftBits];

		int i = 0;
		for (; i < leftBits; i++){
			if ((ledArray[i] = parse_ulong(argv[3+i], 10)) == ULONG_MAX){
				return 1;
			}
			printf("%d", ledArray[i]);
			if (i != leftBits - 1){
				printf(", ");
			}
		}
		printf("])\n");
		unsigned short temp = rightBits;

		return kbd_test_leds(leftBits, ledArray);
	}

	//kbd_test_timed_scan
	else if (strncmp(argv[1], "timed_scan", strlen("timed_scan")) == 0) {
		if (argc != 3) {
			printf("kbd: wrong no of arguments for kbd_test_timed_scan()\n");
			return 1;
		}
		size = parse_ulong(argv[2], 10);						/* Parses string to unsigned long */
		if (size == ULONG_MAX)
			return 1;
		printf("kbd::kbd_test_timed_scan(%hu)\n", size);
		return kbd_test_timed_scan(size);
	}
	else {
		printf("%s is not a valid function!\n", argv[1]);
		return 1;
	}
}

//Completed.
static unsigned long parse_ulong(char *str, int base)
{
	char *endptr;
	unsigned long val;

	/* Convert string to unsigned long */
	val = strtoul(str, &endptr, base);

	/* Check for conversion errors */
	if ((errno == ERANGE && val == ULONG_MAX) || (errno != 0 && val == 0)) {
		perror("strtoul");
		return ULONG_MAX;
	}

	if (endptr == str) {
		printf("kbd: parse_ulong: no digits were found in %s\n", str);
		return ULONG_MAX;
	}

	/* Successful conversion */
	return val;
}

