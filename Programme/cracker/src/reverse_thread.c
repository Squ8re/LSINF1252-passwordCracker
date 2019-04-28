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
/*
 * Ce fichier contient toutes les fonctions relatives aux threads de lecture.
 *
 *  Created on: 14 avr. 2019
 *      Author: amade
 */

/*
 * Cette fonction renvoie un pointeur de type void* pointant vers un hash.
 * @pre 	sharedData est de type shared_data_t et est correctement initialise,
 * 			returnHash doit contenir un espace en memoire de la taille d'un tableau de 32 bytes.
 * @post 	returnHash contient un hash crypte,
		 	sharedData est modifie afin de ne plus contenir le hash qui a ete lu.
 * @return  renvoie un code erreur sous forme de void*.
 * 			retourne 0 en cas de reussite
 * 			retourne un autre chiffre (int) en cas d'erreur.
 */
void *getHash(shared_data_t *sharedData, void *returnHash){
	int firstFullIndex; 			// premier indice rempli
	int errcode;					// gestion des codes erreurs
	// Attente d'un slot rempli
	if(sem_wait(sharedData->hashes_full) == -1){
		fprintf(stderr,
				"Failed to access semaphore 'sharedData->hashes_full' in function 'reverse_thread.c/getHash'.\n");
		return ((void*) -1);}

	// Blocage du buffer
	if((errcode=pthread_mutex_lock(sharedData->hashes_buffer_mtx)) != 0){
		fprintf(stderr,
				"Failed to lock mutex 'sharedData->hashes_buffer_mtx in function 'reverse_thread.c/getHash'.\n");
		errno = errcode;
		return ((void*) -1);}

	// On recupere l'indice du premier slot rempli dans le buffer
	if (sem_getvalue(sharedData->hashes_full, &firstFullIndex) == -1) {
		fprintf(stderr,
				"Failed to retrieve value from semaphore 'sharedData->hashes_full' in function "
						"'reverse_thread.c/getHash'.\n");
		return ((void *) -1);}

	// On copie la valeur a recuperer et on la supprime du buffer
	memcpy(returnHash,
			(sharedData->hashes_buffer)[firstFullIndex],
			sharedData->hash_length * sizeof(uint8_t));
	free((sharedData->hashes_buffer)[firstFullIndex]);

	// On libere le thread
	if ((errcode = pthread_mutex_unlock(sharedData->hashes_buffer_mtx))) {
		fprintf(stderr,
				"Failed to unlock mutex 'sharedData->hashes_buffer_mtx' in function "
						"'reverse_thread.c/getHash'.\n");
		errno = errcode;
		return ((void *) -1);}

	// On indique qu'il y a un slot libre
	if(sem_post(sharedData->hashes_empty) == -1){
		fprintf(stderr,
				"Failed to access semaphore 'sharedData->hashes_buffer_mtx' in function "
						"'reader_thread.c/read_files'.\n");
		return((void*)-1);
	}

	// On termine la fonction
	return ((void*) 0);
}
