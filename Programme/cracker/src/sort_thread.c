#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "utilities.h"
#include "init.h" 														// Utilisation de shared_data
#include "linked_list.h" 												// Utilisation des listes chaines
/*
 * Ce fichier contient toutes les fonctions relatives aux threads de tri.
 *
 *  Created on: 14 avr. 2019
 *      Author: amade
 */

/*
 * Cette fonction prends un element a trier.
 * @pre		'sharedData' est correctement initialise
 * 			'return' contient l'espace memoire necessaire pour un String.
*  @post	return contient un element a trier.
*  			L'element a trier est retire du buffer.
*  @return	La fonction retourne un pointeur vers un 'void*'.
*  			La fonction retourne 0 (sous forme de 'void*') si tout s'est bien deroule, -1 sinon.
 */
void * getPassword(shared_data_t *sharedData, void *returnString){
	int firstFullIndex;				// Premier indice rempli.
	int errcode;					// Gestion des codes erreurs.

	// Attente d'un slot rempli
	if(sem_wait(sharedData->reversed_full) == -1){
		fprintf(stderr,
				"Failed to access semaphore 'sharedData->reversed_full' in function 'sort_thread.c/getPassword.\n");
		return ((void*) -1);
	}

	// Blocage du buffer
	if((errcode=pthread_mutex_lock(sharedData->reversed_buffer_mtx)) != 0){
		fprintf(stderr,
				"Failed to lock mutex 'sharedData->reversed_buffer_mtx' in function 'sort_thread.c/getPassword.\n");
		errno = errcode;
		return ((void*) -1);
	}

	// Recuperation de l'indice du premier slot rempli dans le buffer
	if(sem_getvalue(sharedData->reversed_full, &firstFullIndex) == -1){
		fprintf(stderr,
				"Failed to retrieve value from semaphore 'sharedData->reversed_full' in function"
						" 'sort_thread.c/getPassword'.\n");
		return((void*) -1);
	}

	// Copie de la valeur a recuperer et suppression du buffer
	memcpy(returnString,
			(sharedData->reversed_buffer)[firstFullIndex],
			sharedData)
	// Fin de la fonction.
	return((void*) 0);
}
