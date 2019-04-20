// Fichier qui permet d'avoir une meme fonction de "delay" pour plusieurs OS

#include "custom_sleep.h"


#ifdef _WIN32
	#include <windows.h>
	void custom_sleep(unsigned int delay_ms){
		Sleep(delay_ms);
	}
#else
	#include <time.h>
	void custom_sleep(unsigned int delay_ms){
		long delay_ns = ((long) delay_ms) * 1000000;
		long nanoseconds = delay_ns % 1000000000;
		time_t seconds = (delay_ns - nanoseconds) / 1000000000;
		nanosleep((const struct timespec[]){{seconds, nanoseconds}}, NULL);
	}
#endif
