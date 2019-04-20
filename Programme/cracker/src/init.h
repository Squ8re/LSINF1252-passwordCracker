// Fichier contenant du code utile a l'initialisation du programme faite dans main

#ifndef INIT_HEADER
#define INIT_HEADER

#include <stdint.h>
#include "parsing.h"


typedef struct threads{
	pthread_t reader;               // Thread de lecture des fichiers
	pthread_t *reversers;           // Tableau des threads d'inversion de hashes
	pthread_t cand_manager;         // Thread de gestion des meilleurs candidats
	void *(*read_func)(void *);     // Pointeur vers la fonction qu'utilise le thread de lecture
	void *(*reverse_func)(void *);  // Pointeur vers la fonction que font tourner les threads d'inversion de hash
	void *(*cand_func)(void *);     // Pointeur vers la fonction que fait tourner le thread de gestion des meilleurs candidats
}threads_t;

void create_buffers(uint8_t ***hashes_buffer_ptr,
		            char ***reversed_buffer_ptr,
					options_t *user_options,
				    unsigned int buffer_ratio,
					unsigned int hash_length);

void launch_threads(threads_t *threads_ptr, const options_t *user_options);


#endif
