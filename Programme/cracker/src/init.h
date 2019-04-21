// Fichier contenant du code utile a l'initialisation du programme faite dans main

#ifndef INIT_HEADER
#define INIT_HEADER

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#include "parsing.h"


typedef struct reader_data{
	unsigned int n_files;
	char **in_files_names;
	bool *all_files_read;
	uint8_t ***hashes_buffer;
	pthread_mutex_t *hashes_buffer_mtx;
	sem_t *hashes_empty;
	sem_t *hashes_full;
}reader_data_t;

typedef struct threads{
	pthread_t reader;               // Thread de lecture des fichiers
	pthread_t *reversers;           // Tableau des threads d'inversion de hashes
	pthread_t cand_manager;         // Thread de gestion des meilleurs candidats
	void *(*read_func)(void *);     // Pointeur vers la fonction qu'utilise le thread de lecture
	void *(*reverse_func)(void *);  // Pointeur vers la fonction que font tourner les threads d'inversion de hash
	void *(*cand_func)(void *);     // Pointeur vers la fonction que fait tourner le thread de gestion des meilleurs candidats
	reader_data_t *reader_params;   // Pointeur vers la structure contenant les parametres de la fonction de lecture
}threads_t;

// Structure qui regroupe les differents outils de coordination des threads
typedef struct coordinators{
	bool *all_files_read;                  // *all_files_read est false tant que tous les fichiers n'ont pas ete lus
	pthread_mutex_t *hashes_buffer_mtx;    // pointe vers le mutex de protection du buffer contenant les hashes
	pthread_mutex_t *reversed_buffer_mtx;  // pointe vers le mutex de protection du buffer contenant les pwd en clair
	sem_t *hashes_empty;                   // Premier semaphore de gestion du buffer contenant les hashes
	sem_t *hashes_full;                    // Second semaphore de gestion du buffer contenant les hashes
	sem_t *reversed_empty;                 // Premier semaphore de gestion du buffer contenant les pwd en clair
	sem_t *reversed_full;                  // Second semaphore de gestion du buffer contenant les pwd en clair
}coord_t;

void create_buffers(uint8_t ***hashes_buffer_ptr,
		            char ***reversed_buffer_ptr,
					options_t *user_options,
				    unsigned int buffer_ratio,
					unsigned int hash_length);

void launch_threads(threads_t *threads_ptr, const options_t *user_options);

coord_t *create_coordinators(unsigned int buffer_length);


#endif
