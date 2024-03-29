#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "reverse.h"
#include "utilities.h"
#include "init.h" 														// Utilisation de shared_data
/*
 * Ce fichier contient toutes les fonctions relatives aux threads de reversing.
 * La fonction a appeler pour lancer l'operation est 'reverse'.
 *
 *  Created on: 14 avr. 2019
 *      Author: Amadeo DAVID
 *      NOMA  : 4476 1700
 */

/*
 * Cette fonction renvoie un pointeur de type void* pointant vers un hash.
 * @pre 	shared est de type shared_data_t et est correctement initialise,
 * 			return_hash doit contenir un espace en memoire de la taille d'un tableau de shared->hash_length bytes.
 * @post 	return_hash contient un hash crypte,
		 	shared est modifie afin de ne plus contenir le hash qui a ete lu.
 * @return  renvoie un code erreur sous forme de void*.
 * 			retourne 0 en cas de reussite
 * 			retourne un autre chiffre (int) en cas d'erreur.
 */
int get_hash(shared_data_t *shared, uint8_t *return_hash){
	int first_full_index; 			// premier indice rempli
	int errcode;					// gestion des codes erreurs

	// Attente d'un slot rempli
	if(sem_wait(shared->hashes_full) == -1){
		fprintf(stderr,
				"Failed to access semaphore 'shared->hashes_full' in function 'reverse_thread.c/get_hash'.\n");
		return -1;
	}

	// Blocage du buffer
	if((errcode=pthread_mutex_lock(shared->hashes_buffer_mtx)) != 0){
		fprintf(stderr,
				"Failed to lock mutex 'shared->hashes_buffer_mtx in function 'reverse_thread.c/get_hash'.\n");
		errno = errcode;
		return -1;
	}

	// On recupere l'indice du premier slot rempli dans le buffer
	if (sem_getvalue(shared->hashes_full, &first_full_index) == -1) {
		fprintf(stderr,
				"Failed to retrieve value from semaphore 'shared->hashes_full' in function "
						"'reverse_thread.c/get_hash'.\n");
		return -1;
	}

	// On copie la valeur a recuperer et on la supprime du buffer
	memcpy(return_hash,
			((shared->hashes_buffer)[first_full_index]),
			sizeof(uint8_t) * shared->hash_length);
	free((shared->hashes_buffer)[first_full_index]);

	// On libere le thread
	if ((errcode = pthread_mutex_unlock(shared->hashes_buffer_mtx))) {
		fprintf(stderr,
				"Failed to unlock mutex 'shared->hashes_buffer_mtx' in function "
						"'reverse_thread.c/get_hash'.\n");
		errno = errcode;
		return -1;
	}

	// On indique qu'il y a un slot libre
	if(sem_post(shared->hashes_empty) == -1){
		fprintf(stderr,
				"Failed to access semaphore 'shared->hashes_empty' in function "
						"'reverse_thread.c/get_hash'.\n");
		return -1;
	}

	// On verifie si tout les fichiers ont ete recuperes
	if(shared->all_files_read && first_full_index == 0){
		shared->all_files_reversed = true;
	}

	// On termine la fonction
	return 0;
}
/**
 * Cette fonction transforme un hash et le stock dans le buffer des fichiers qui ne sont pas tries.
 * @pre 	shared est de type shared_data_t et est correctement initialise.
 * @post	shared est modifie afin de contenir le mot de passe dechiffre dans le bon buffer.
 * 			shared ne contient plus le hash qui a decrypte.
 * @return	renvoie un code erreur sous forme de void*.
 * 			retourne 0 en cas de reussite
 * 			retour un autre chiffre (int) en cas d'erreur.
 */
void *reverse(void *reverse_params){
	shared_data_t *shared = (shared_data_t *) (reverse_params);
	int first_free_index = -1; 		// premier indice rempli
	int errcode;					// gestion des codes erreurs
	uint8_t *hash;
	char *reversed;

	while(!(shared->all_files_read && first_free_index == 0)){
		// Recuperation d'un hash.
		hash = (uint8_t *) (malloc(shared->hash_length * sizeof(uint8_t)));
		if(!hash){
			fprintf(stderr, "Failed to allocate memory for 'hash' in function 'reverse_thread.c/reverse'.\n");
			return ((void*) -1);
		}
		if(get_hash(shared, hash) != 0){
			fprintf(stderr,
					"Failed to retrieve hash from 'shared->hashes_buffer' in function 'reverse_thread.c/reverse'.\n");
		}

		// alloue reverse
		reversed = (char *) malloc(shared->hash_length * sizeof(char));
		if(!reversed){
			fprintf(stderr, "Failed to allocate memory for 'reversed' in function 'reverse_thread.c/reverse'.\n");
		}

		// Application de la fonction reverse.
		if(!(reversehash(hash, reversed, shared->hash_length))){
			fprintf(stderr, "Failed to reverse hash in function 'reverse_thread.c/reverse'.\n");
			return ((void *) -1);
		}
		free(hash);

		// Attente d'un slot vide du buffer 'reversed_buffer'.
		if(sem_wait(shared->reversed_empty) == -1){
			fprintf(stderr,
					"Failed to access semaphore 'shared->reversed_empty' in function 'reverse_thread.c/reverse'.\n");
			return ((void*) -1);}

		// Blocage du buffer
		if((errcode=pthread_mutex_lock(shared->reversed_buffer_mtx)) != 0){
			fprintf(stderr,
					"Failed to lock mutex 'shared->reversed_buffer_mtx in function 'reverse_thread.c/reverse'.\n");
			errno = errcode;
			return ((void*) -1);}

		// On recupere l'indice du premier slot vide dans le buffer reverse
		if (sem_getvalue(shared->reversed_full, &first_free_index) == -1) {
		fprintf(stderr,
				"Failed to retrieve value from semaphore 'shared->reversed_empty' in function "
						"'reverse_thread.c/reverse'.\n");
		return ((void *) -1);}

		memcpy((shared->reversed_buffer)[first_free_index], reversed, (strlen(reversed) + 1)*sizeof(char));
		free(reversed);
		printf("MDP stocke dans reversed_buffer : %s", (shared->reversed_buffer)[first_free_index]);

		// On libere le buffer
		if ((errcode = pthread_mutex_unlock(shared->reversed_buffer_mtx))) {
			fprintf(stderr,
					"Failed to unlock mutex 'shared->reversed_buffer_mtx' in function "
							"'reverse_thread.c/revers_files'.\n");
				errno = errcode;
				return ((void *) -1);
			}

		// On indique qu'il y a un slot rempli en plus
		if(sem_post(shared->reversed_full) == -1){
			fprintf(stderr,
					"Failed to access semaphore 'shared->reversed_full' in function "
							"'reverse_thread.c/reverse'.\n");
			return((void*)-1);
		}
	}
	// On termine la fonction.
	return((void*) 0);
}
