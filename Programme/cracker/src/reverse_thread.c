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
 * Ce fichier contient toutes les fonctions relatives aux threads de reversing.
 * La fonction a appeler pour lancer l'operation est 'reverse'.
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
				"Failed to access semaphore 'sharedData->hashes_empty' in function "
						"'reverse_thread.c/getHash'.\n");
		return((void*)-1);
	}

	// On termine la fonction
	return ((void*) 0);
}
/**
 * Cette fonction transforme un hash et le stock dans le buffer des fichiers qui ne sont pas tries.
 * @pre 	sharedData est de type shared_data_t et est correctement initialise.
 * @post	sharedData est modifie afin de contenir le mot de passe dechiffre dans le bon buffer.
 * 			sharedData ne contient plus le hash qui a decrypte.
 * @return	renvoie un code erreur sous forme de void*.
 * 			retourne 0 en cas de reussite
 * 			retour un autre chiffre (int) en cas d'erreur.
 */
void *reverse(shared_data_t *sharedData){
	int firstFreeIndex; 			// premier indice rempli
	int errcode;					// gestion des codes erreurs
	// Recuperation d'un hash.
	int* hash[] = (int *)(malloc(32));
	if(hash==-1){
		fprintf(stderr, "Failed to allocate memory for 'hash' in function 'reverse_thread.c/reverse'.\n");
		return ((void*) -1);
	}
	if(getHash(sharedData, (void*)(hash)) != 0){
		fprintf(stderr,
				"Failed to retrieve hash from 'sharedData->hashes_buffer' in function 'reverse_thread.c/reverse'.\n");
	}

	// Attente d'un slot vide du buffer 'reversed_buffer'.
	if(sem_wait(sharedData->reversed_empty) == -1){
		fprintf(stderr,
				"Failed to access semaphore 'sharedData->reversed_empty' in function 'reverse_thread.c/reverse'.\n");
		return ((void*) -1);}

	// Blocage du buffer
	if((errcode=pthread_mutex_lock(sharedData->reversed_buffer_mtx)) != 0){
		fprintf(stderr,
				"Failed to lock mutex 'sharedData->reversed_buffer_mtx in function 'reverse_thread.c/reverse'.\n");
		errno = errcode;
		return ((void*) -1);}

	// On recupere l'indice du premier slot rempli dans le buffer
		if (sem_getvalue(sharedData->hashes_full, &firstFreeIndex) == -1) {
		fprintf(stderr,
				"Failed to retrieve value from semaphore 'sharedData->reversed_empty' in function "
						"'reverse_thread.c/reverse'.\n");
		return ((void *) -1);}

	// Application de la fonction.
	if(!((int) reversehash(hash,(sharedData->reversed_buffer)[firstFreeIndex],sharedData->hash_length))){
		fprintf(stderr, "Failed to reverse hash in function 'reverse_thread.c/reverse'.\n");
		return ((void *) -1);
	}
	free(hash);

	// On libere le thread
	if ((errcode = pthread_mutex_unlock(sharedData->reversed_buffer_mtx))) {
		fprintf(stderr,
				"Failed to unlock mutex 'sharedData->reversed_buffer_mtx' in function "
						"'reverse_thread.c/reverse'.\n");
		errno = errcode;
		return ((void *) -1);}

	// On indique qu'il y a un slot rempli en plus
	if(sem_post(sharedData->reversed_full) == -1){
		fprintf(stderr,
				"Failed to access semaphore 'sharedData->reversed_full' in function "
						"'reverse_thread.c/reverse'.\n");
		return((void*)-1);
	}

	// On termine la fonction.
	return((void*) 0);
}
