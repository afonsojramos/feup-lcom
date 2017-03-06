#ifndef __RTC_H
#define __RTC_H

#include "rtc.h"
#include "i8254.h"
#include <minix/syslib.h>
#include <minix/drivers.h>
#include <minix/com.h>

/* REGISTER DECLARATIONS*/
#define RTC_ADDR_REG			0x70
#define RTC_DATA_REG			0x71
#define RTC_IRQ					8

#define RTC_SECOND				0x00
#define RTC_MINUTE				0x02
#define RTC_HOUR				0x04
#define RTC_DAY					0x07
#define RTC_MONTH				0x08
#define RTC_YEAR				0x09

#define REGISTER_A				0x0A
#define REGISTER_B				0x0B
#define REGISTER_C				0x0C
#define REGISTER_D				0x0D

#define RTC_REGISTER_B_DM		BIT(2)
#define RTC_REGISTER_A_UIP		BIT(7)

/*!
 * @brief Checks whether the RTC is updating
 */
int updateCheck();

/*!
 * @brief Checks whether the RTC extracted values are coded in BCD.
 */
int BCDcheck();

/*!
 * @brief Converts a value coded in BCD to binary.
 * @param bcd The value to be converted.
 */
unsigned long binaryConverter(unsigned long* bcd);

/*!
 * @brief Extracts the values from the RTC.
 * @param hour,minute,second,day,month,year The values to be extracted.
 */
void extractDate(unsigned long *hour, unsigned long *minute, unsigned long *second, unsigned long *day, unsigned long *month, unsigned long *year);

#endif
