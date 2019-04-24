// Fichier contenant du code utile a l'initialisation du programme faite dans main

#ifndef INIT_HEADER
#define INIT_HEADER

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

#include "parsing.h"
#include "linked_list.h"

typedef struct threads {
		pthread_t *reader;    // Pointeur vers le thread de lecture des fichiers
		pthread_t *reversers;       // Tableau des threads d'inversion de hashes
		pthread_t *cand_manager; // Pointeur vers le thread de gestion des meilleurs candidats
		void *(*read_func)(void *); // Pointeur vers la fonction qu'utilise le thread de lecture
		void *(*reverse_func)(void *); // Pointeur vers la fonction que font tourner les threads d'inversion de hash
		void *(*cand_func)(void *); // Pointeur vers la fonction que fait tourner le thread de gestion des meilleurs candidats
} threads_t;

typedef struct shared_data {
		// Les options de l'utilisateur
		options_t *user_options; // Pointeur vers une structure reprenant les options choisies par l'utilisateur

		// Les threads et leurs donnees
		threads_t *threads_data; // Pointeur vers une structure reprenant les threads et des donnees associees a ceux-ci

		// Constantes utiles
		unsigned int buffer_ratio; // Valeur telle que les buffers contiennent buffer_ratio * n_threads_calcul elements
		unsigned int hash_length;         // Nombre de bytes constituant un hash
		unsigned int buffer_length; // Nombre d'elements dans un buffer (identique pour tous les buffers)

		// Les buffers
		uint8_t **hashes_buffer;               // Buffer contenant les hashes
		char **reversed_buffer;             // Buffer contenant les pwd en clair
		linked_list_t best_candidates; // Liste chainee contenant les meilleurs pwd en clair a tout instant

		// Outils de coordination des threads
		bool all_files_read; // pointe vers un false tant que tous les fichiers n'ont pas ete lus
		pthread_mutex_t *hashes_buffer_mtx; // pointe vers le mutex de protection du buffer contenant les hashes
		pthread_mutex_t *reversed_buffer_mtx; // pointe vers le mutex de protection du buffer contenant les pwd en clair
		sem_t *hashes_empty; // Premier semaphore de gestion du buffer contenant les hashes
		sem_t *hashes_full; // Second semaphore de gestion du buffer contenant les hashes
		sem_t *reversed_empty; // Premier semaphore de gestion du buffer contenant les pwd en clair
		sem_t *reversed_full; // Second semaphore de gestion du buffer contenant les pwd en clair
} shared_data_t;

int create_buffers(shared_data_t *shared);

int launch_threads(shared_data_t *shared);

int create_coordinators(shared_data_t *shared);

#endif
