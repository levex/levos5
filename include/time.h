#ifndef __TIME_H_
#define __TIME_H_

struct tm {
	uint32_t tm_sec;
	uint32_t tm_min;
	uint32_t tm_hour;
	uint32_t tm_mday;
	uint32_t tm_mon;
	uint32_t tm_year;
	uint32_t tm_wday;
	uint32_t tm_yday;
	uint32_t tm_isdst;
}; /*36 bytes */

#endif
