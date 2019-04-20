// Fichier contenant du code utile a l'initialisation du programme faite dans main

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "utilities.h"
#include "parsing.h"
#include "init.h"

void create_buffers(uint8_t ***hashes_buffer_ptr,
		            char ***reversed_buffer_ptr,
					options_t *user_options,
				    unsigned int buffer_ratio,
					unsigned int hash_length){
	// buffer_ratio > 0: entier tel que la longueur des buffers est de k * nombre_de_threads_de_calcul
	// hash_length > 0: entier donnant le nombre de bytes composant un hash
	// Triples pointeurs car: les buffers sont des pointeurs de pointeurs (**) et nous on veut pointer vers les buffer (***)

	unsigned int buffer_length = buffer_ratio * user_options->n_threads;

	// Buffer des hashes
	*hashes_buffer_ptr = (uint8_t **) malloc_retry(10, 10, buffer_length * sizeof(uint8_t *));
	if(!(*hashes_buffer_ptr)){
		fprintf(stderr, "Memory could not be accessed (malloc failure).\n");
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i < buffer_length; i++){
		(*hashes_buffer_ptr)[i] = (uint8_t *) malloc_retry(10, 10, hash_length * sizeof(uint8_t));
		if(!(*hashes_buffer_ptr)[i]){
			fprintf(stderr, "Memory could not be accessed (malloc failure).\n");
			exit(EXIT_FAILURE);
		}
	}

	// Buffer des mots de passe en clair (apres inversion, donc)
	*reversed_buffer_ptr = (char **) malloc_retry(10, 10, buffer_length * sizeof(char *));
	if(!(*reversed_buffer_ptr)){
		fprintf(stderr, "Memory could not be accessed (malloc failure).\n");
		exit(EXIT_FAILURE);
	}

	// Contrairement a hashes_buffer, ou l'on connait deja la taille des hashes et ou on peut donc initialiser les espaces
	// qui contiendront les hashes, on ne peut pas faire de meme avec reversed_buffer, car on ne sait pas encore la taille
	// qu'auront les char * representant les mots de passe en clair.
	// TODO: Il faudra faire attention quand on initialise ces espaces a tenir compte du '\0' dans le malloc.
}

void launch_threads(threads_t *threads_ptr, const options_t *user_options){
	// @pre threads doit etre une structure vide (sinon on aura une fuite de memoire a cause du fait qu'on utilise pas de free ici)
	// Allocation du tableau des threads d'inversion de hashes
	threads_ptr->reversers = (pthread_t *) malloc_retry(10, 10, user_options->n_threads * sizeof(pthread_t));

	if(!(threads_ptr->reversers)){
		fprintf(stderr, "Memory could not be accessed (malloc failure).\n");
		exit(EXIT_FAILURE);
	}

	// Lancement des threads
	if(pthread_create(&(threads_ptr->reader), NULL, threads_ptr->read_func, NULL) != 0){
		fprintf(stderr, "Thread could not be created.\n");
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i < user_options->n_threads; i++){
		if(pthread_create(&(threads_ptr->reversers)[i], NULL, threads_ptr->reverse_func, NULL) != 0){
			fprintf(stderr, "Thread could not be created.\n");
			exit(EXIT_FAILURE);
		}
	}

	if(pthread_create(&(threads_ptr->cand_manager), NULL, threads_ptr->cand_func, NULL) != 0){
		fprintf(stderr, "Thread could not be created.\n");
		exit(EXIT_FAILURE);
	}
}
