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
 * @pre 	shared est de type shared_data_t et est correctement initialise,
 * 			return_hash doit contenir un espace en memoire de la taille d'un tableau de shared->hash_length bytes.
 * @post 	return_hash contient un hash crypte,
		 	shared est modifie afin de ne plus contenir le hash qui a ete lu.
 * @return  renvoie un code erreur sous forme de void*.
 * 			retourne 0 en cas de reussite
 * 			retourne un autre chiffre (int) en cas d'erreur.
 */
void *get_hash(shared_data_t *shared, uint8_t **return_hash){
	// TODO: (Ed): J'ai change le void * en uint8_t ** pour return_hash: check si ca implique pas qqes changements ci-dessous...
	int first_full_index; 			// premier indice rempli
	int errcode;					// gestion des codes erreurs
	// Attente d'un slot rempli
	if(sem_wait(shared->hashes_full) == -1){
		fprintf(stderr,
				"Failed to access semaphore 'shared->hashes_full' in function 'reverse_thread.c/get_hash'.\n");
		return ((void*) -1);}

	// Blocage du buffer
	if((errcode=pthread_mutex_lock(shared->hashes_buffer_mtx)) != 0){
		fprintf(stderr,
				"Failed to lock mutex 'shared->hashes_buffer_mtx in function 'reverse_thread.c/get_hash'.\n");
		errno = errcode;
		return ((void*) -1);}

	// TODO: (Ed): J'ai rajouter un "--" parce que tu as pris l'indice du premier slot libre (copy-paste de reader_thread) et pas du premier
	// slot rempli (il me semble, je peux me tromper...)

	// On recupere l'indice du premier slot rempli dans le buffer
	if (sem_getvalue(shared->hashes_full, &first_full_index) == -1) {
		fprintf(stderr,
				"Failed to retrieve value from semaphore 'shared->hashes_full' in function "
						"'reverse_thread.c/get_hash'.\n");
		return ((void *) -1);}

	first_full_index--;  // TODO: la modif que j'ai fait. (pas sur que ce soit correct though)
	// TODO: Comment on fait si first_full_index est < 0? (aka le buffer est vide)

	// On copie la valeur a recuperer et on la supprime du buffer
	memcpy(return_hash,
			(shared->hashes_buffer)[first_full_index],
			shared->hash_length * sizeof(uint8_t));
	free((shared->hashes_buffer)[first_full_index]);

	// On libere le thread
	if ((errcode = pthread_mutex_unlock(shared->hashes_buffer_mtx))) {
		fprintf(stderr,
				"Failed to unlock mutex 'shared->hashes_buffer_mtx' in function "
						"'reverse_thread.c/get_hash'.\n");
		errno = errcode;
		return ((void *) -1);}

	// On indique qu'il y a un slot libre
	if(sem_post(shared->hashes_empty) == -1){
		fprintf(stderr,
				"Failed to access semaphore 'shared->hashes_empty' in function "
						"'reverse_thread.c/get_hash'.\n");
		return((void*)-1);
	}

	// On termine la fonction
	return ((void*) 0);
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
void *reverse(shared_data_t *shared){
	int first_free_index; 			// premier indice rempli
	int errcode;					// gestion des codes erreurs
	// Recuperation d'un hash.

	uint8_t *hash = (uint8_t *)(malloc(shared->hash_length * sizeof(uint8_t)));
	if(!hash){
		fprintf(stderr, "Failed to allocate memory for 'hash' in function 'reverse_thread.c/reverse'.\n");
		return ((void*) -1);
	}

	// TODO: Ici, comment on s'assure qu'il y a un hash actuellement disponible?
	// Sinon on peut aussi gerer ca dans get_hash
	// TODO: comme on recupere le 'first_free_index' plusieurs fois, go fair eune fonction? (pas oblige hn)
	if(get_hash(shared, &hash) != 0){
		fprintf(stderr,
				"Failed to retrieve hash from 'shared->hashes_buffer' in function 'reverse_thread.c/reverse'.\n");
	}

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

	// On recupere l'indice du premier slot rempli dans le buffer
	// TODO: Meme reflexion que dans get_hash: ici je pens eque tu recuperes le premier slot vide.
	if (sem_getvalue(shared->hashes_full, &first_free_index) == -1) {
	fprintf(stderr,
			"Failed to retrieve value from semaphore 'shared->reversed_empty' in function "
					"'reverse_thread.c/reverse'.\n");
	return ((void *) -1);}

	char *reversed; // On stocke en local le temps de lire la taille du String
	// Application de la fonction.

	// TODO: pourquoi le cast (int)? (ca marche hn mais bon je pense pas que ce soit utile)
	if(!((int) reversehash(hash, &reversed, shared->hash_length))){
		fprintf(stderr, "Failed to reverse hash in function 'reverse_thread.c/reverse'.\n");
		return ((void *) -1);
	}
	free(hash);

	// On cree l'espace pour le reversed_hash
	//TODO : verifier si le +1 a du sens -> (Ed) Je pense que oui car strlen ne compte pas le \0
	(shared->reversed_buffer)[first_free_index] = (char[]) (malloc((strlen(reversed)+1)*sizeof(char)));
	if((shared->reversed_buffer)[first_free_index] == -1){
		fprintf(stderr,
				"Failed to allocate memory for '(shared->reversed_buffer)[first_free_index]' "
					"in function 'reverse_thread.c/reverse'.\n");
		return((void *) -1);
	}
	//TODO : regarder la gestion des erreurs de memcpy
	// On copie le reversed hash dans la structure partagee
	memcpy(reversed,
			(shared->reversed_buffer)[first_free_index],
			(strlen(reversed)+1)*sizeof(char));

	// On libere le thread
	if ((errcode = pthread_mutex_unlock(shared->reversed_buffer_mtx))) {
		fprintf(stderr,
				"Failed to unlock mutex 'shared->reversed_buffer_mtx' in function "
						"'reverse_thread.c/reverse'.\n");
		errno = errcode;
		return ((void *) -1);}

	// On indique qu'il y a un slot rempli en plus
	if(sem_post(shared->reversed_full) == -1){
		fprintf(stderr,
				"Failed to access semaphore 'shared->reversed_full' in function "
						"'reverse_thread.c/reverse'.\n");
		return((void*)-1);
	}

	// On termine la fonction.
	return((void*) 0);
}
