
#ifndef ZEPHYR_LIB_LIBC_MINIMAL_INCLUDE_TIME_H_
#define ZEPHYR_LIB_LIBC_MINIMAL_INCLUDE_TIME_H_

#include <sys/types.h>
#include <zephyr/toolchain.h>

#include_next <time.h>

#ifdef __cplusplus
extern "C" {
#endif

struct tm *gmtime_r(const time_t *ZRESTRICT timep,
		    struct tm *ZRESTRICT result);

#ifdef __cplusplus
}
#endif

#endif  /* ZEPHYR_LIB_LIBC_MINIMAL_INCLUDE_TIME_H_ */
