// Fichier qui permet d'avoir une meme fonction de "delay" pour plusieurs OS

#include "custom_sleep.h"


#ifdef _WIN32
	#include <windows.h>
	void custom_sleep(unsigned int delay_ms){
		Sleep(delay_ms);
	}
#else
	#include <unistd.h>
	void custom_sleep(unsigned int delay_ms){
		delay(delay_ms);
	}
#endif
