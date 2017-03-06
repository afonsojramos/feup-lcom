#include "rtc.h"

int updateCheck() {
	unsigned long token = 0;

	sys_outb(RTC_ADDR_REG, REGISTER_A);
	sys_inb(RTC_DATA_REG, &token);

	if ((token & RTC_REGISTER_A_UIP) != 0){
		return 1;
	}
	else{
		return 0;
	}
}

int BCDcheck() {
	unsigned long token = 0;

	sys_outb(RTC_ADDR_REG, REGISTER_B);
	sys_inb(RTC_DATA_REG, &token);

	if (token & RTC_REGISTER_B_DM){
		return 0;
	}
	else{
		return 1;
	}
}

unsigned long binaryConverter(unsigned long* bcd){
	unsigned long binary;

	binary = (((*bcd) & 0xF0) >> 4) * 10 + ((*bcd) & 0x0F);

	return binary;
}

void extractDate(unsigned long *hour, unsigned long *minute, unsigned long *second, unsigned long *day, unsigned long *month, unsigned long *year){

	/* HOUR EXTRACTION */
	sys_outb(RTC_ADDR_REG, RTC_HOUR);
	sys_inb(RTC_DATA_REG, hour);

	/* MINUTE EXTRACTION */
	sys_outb(RTC_ADDR_REG, RTC_MINUTE);
	sys_inb(RTC_DATA_REG, minute);

	/* SECOND EXTRACTION */
	sys_outb(RTC_ADDR_REG, RTC_SECOND);
	sys_inb(RTC_DATA_REG, second);

	/* DAY EXTRACTION */
	sys_outb(RTC_ADDR_REG, RTC_DAY);
	sys_inb(RTC_DATA_REG, day);

	/* MONTH EXTRACTION */
	sys_outb(RTC_ADDR_REG, RTC_MONTH);
	sys_inb(RTC_DATA_REG, month);

	/* YEAR EXTRACTION */
	sys_outb(RTC_ADDR_REG, RTC_YEAR);
	sys_inb(RTC_DATA_REG, year);

	if (BCDcheck()){
		(*hour) = binaryConverter(hour);
		(*minute) = binaryConverter(minute);
		(*second) = binaryConverter(second);
		(*day) = binaryConverter(day);
		(*month) = binaryConverter(month);
		(*year) = binaryConverter(year);
	}
	return;
}
