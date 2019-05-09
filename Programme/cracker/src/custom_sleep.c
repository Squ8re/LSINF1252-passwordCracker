// Fichier qui permet d'avoir une meme fonction de "delay" pour plusieurs OS

/*
 *
 * Author: Eduardo Vannini (NOMA: 10301700)
 *          Date: 09-05-2019
 *
 */

#include "utilities.h"
#include "custom_sleep.h"

#ifdef _WIN32
#include <windows.h>
/*
 * Fonction permettant d'introduire un delai de longueur donnee dans l'execution du programme.
 *
 * @pre 'delay_ms' > 0.
 * @post Le programme sera mis en pause durant 'delay_ms' millisecondes puis retourne 0.
 *
 */
int custom_sleep(unsigned int delay_ms) {
	Sleep(delay_ms);
	return 0; // Pour avoir le meme returntype dans la version win et la version linux
}
#else
#include <time.h>
/*
 * Fonction permettant d'introduire un delai de longueur donnee dans l'execution du programme.
 *
 * @pre 'delay_ms' > 0.
 * @post Si aucune erreur n'est rencontree, le programme sera mis en pause durant 'delay_ms'
 *       millisecondes. Sinon, la fonction affiche un message d'erreur sur stderr et retourne -1.
 *
 */
int custom_sleep(unsigned int delay_ms) {
	long delay_ns = ((long) delay_ms) * 1000000;
	long nanoseconds = delay_ns % 1000000000;
	time_t seconds = (delay_ns - nanoseconds) / 1000000000;
	if (nanosleep((const struct timespec[] ) { { seconds, nanoseconds } }, NULL)
			== -1) {
		fprintf(stderr,
				"Failed to call function 'nanosleep' in function 'custom_sleep.c/custom_sleep'.\n");
		return -1;
	}
	return 0;
}
#endif
