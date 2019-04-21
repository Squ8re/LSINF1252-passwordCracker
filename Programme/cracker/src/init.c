// Fichier contenant du code utile a l'initialisation du programme faite dans main

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

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
	check_malloc((void *) *hashes_buffer_ptr, "Failed to allocate memory for 'hashes_buffer_ptr'"
			                                  " in function 'init.c/create_buffers'");

	for(int i = 0; i < buffer_length; i++){
		(*hashes_buffer_ptr)[i] = (uint8_t *) malloc_retry(10, 10, hash_length * sizeof(uint8_t));
		check_malloc((void *) (*hashes_buffer_ptr)[i], "Failed to allocate memory for 'hashes_buffer_ptr[i]'"
				                                       " in function 'init.c/create_buffers'");
	}

	// Buffer des mots de passe en clair (apres inversion, donc)
	*reversed_buffer_ptr = (char **) malloc_retry(10, 10, buffer_length * sizeof(char *));
	check_malloc((void *) *reversed_buffer_ptr, "Failed to allocate memory for 'reversed_buffer_ptr' in function"
			                                    " 'init.c/create_buffers'");

	// Contrairement a hashes_buffer, ou l'on connait deja la taille des hashes et ou on peut donc initialiser les espaces
	// qui contiendront les hashes, on ne peut pas faire de meme avec reversed_buffer, car on ne sait pas encore la taille
	// qu'auront les char * representant les mots de passe en clair.
	// TODO: Il faudra faire attention quand on initialise ces espaces a tenir compte du '\0' dans le malloc.
}

void launch_threads(threads_t *threads_ptr, const options_t *user_options){
	// @pre threads doit etre une structure vide (sinon on aura une fuite de memoire a cause du fait qu'on utilise pas de free ici)
	// Allocation du tableau des threads d'inversion de hashes
	threads_ptr->reversers = (pthread_t *) malloc_retry(10, 10, user_options->n_threads * sizeof(pthread_t));
	check_malloc((void *) threads_ptr->reversers, "Failed to allocate memory for 'threads_ptr->reversers'"
			                                      " in function 'init.c/launch_threads'");

	// Lancement des threads
	int tmp = pthread_create(&(threads_ptr->reader), NULL, threads_ptr->read_func, (void *) (threads_ptr->reader_params));
	if(tmp != 0){
		handle_error("Failed to create thread 'reader' in function 'init.c/launch_threads'");
	}

	for(int i = 0; i < user_options->n_threads; i++){
		if(pthread_create(&(threads_ptr->reversers)[i], NULL, threads_ptr->reverse_func, NULL) != 0){
			handle_error("Failed to create thread 'reversers[i]' in function 'init.c/launch_threads'");
		}
	}

	if(pthread_create(&(threads_ptr->cand_manager), NULL, threads_ptr->cand_func, NULL) != 0){
		handle_error("Failed to create thread 'cand_manager' in function 'init.c/launch_threads'");
	}
}

coord_t *create_coordinators(unsigned int buffer_length){
	// Creation des mutex, semaphores et autres variables "de controle"
	// Ce qui est partage entre les threads doit aller dans le heap
	bool *all_files_read = (bool *) malloc_retry(10, 10, sizeof(bool));
	*all_files_read = false;
	// Pas besoin de mutex pour all_files_read car seul le thread de lecture va le modifier
	// Il faudra mettre un mutex si on utilise plusieurs threads de lecture (et encore, pas sur...)
	// Mutex sur le hashes_buffer
	pthread_mutex_t *hashes_buffer_mtx = (pthread_mutex_t *) malloc_retry(10, 10, sizeof(pthread_mutex_t));
	check_malloc((void *) hashes_buffer_mtx, "Failed to allocate memory for 'hashes_buffer_mtx'"
                                             " in function 'init.c/create_coordinators'");

	// Mutex sur le reversed_buffer
	pthread_mutex_t *reversed_buffer_mtx = (pthread_mutex_t *) malloc_retry(10, 10, sizeof(pthread_mutex_t));
	check_malloc((void *) reversed_buffer_mtx, "Failed to allocate memory for 'reversed_buffer_mtx'"
                                               " in function 'init.c/create_coordinators'");

	sem_t *hashes_empty = (sem_t *) malloc_retry(10, 10, sizeof(sem_t));
	check_malloc((void *) hashes_empty, "Failed to allocate memory for 'hashes_empty'"
                                        " in function 'init.c/create_coordinators'");

	sem_t *hashes_full = (sem_t *) malloc_retry(10, 10, sizeof(sem_t));
	check_malloc((void *) hashes_full, "Failed to allocate memory for 'hashes_full'"
                                       " in function 'init.c/create_coordinators'");

	sem_t *reversed_empty = (sem_t *) malloc_retry(10, 10, sizeof(sem_t));
	check_malloc((void *) reversed_empty, "Failed to allocate memory for 'reversed_empty'"
                                          " in function 'init.c/create_coordinators'");

	sem_t *reversed_full = (sem_t *) malloc_retry(10, 10, sizeof(sem_t));
	check_malloc((void *) reversed_full, "Failed to allocate memory for 'reversed_full'"
                                         " in function 'init.c/create_coordinators'");

	if(pthread_mutex_init(hashes_buffer_mtx, NULL) ||
	   pthread_mutex_init(reversed_buffer_mtx, NULL)){
		handle_error("Failed to initialize mutexes in function 'init.c/create_coordinators'");
	}

	if(sem_init(hashes_empty, 0, buffer_length) ||
	   sem_init(hashes_full, 0, 0) ||
	   sem_init(reversed_empty, 0, buffer_length) ||
	   sem_init(reversed_full, 0, 0)){
		handle_error("Failed to initialize semaphores in function 'init.c/create_coordinators'");
	}

	coord_t *coord_ptr = malloc_retry(10, 10, sizeof(coord_t));
	check_malloc((void *) coord_ptr, "Failed to allocate memory for 'coord_ptr'"
                                     " in function 'init.c/create_coordinators'");

	coord_ptr->all_files_read = all_files_read;
	coord_ptr->hashes_buffer_mtx = hashes_buffer_mtx;
	coord_ptr->reversed_buffer_mtx = reversed_buffer_mtx;
	coord_ptr->hashes_empty = hashes_empty;
	coord_ptr->hashes_full = hashes_full;
	coord_ptr->reversed_empty = reversed_empty;
	coord_ptr->reversed_full = reversed_full;

	return coord_ptr;
}
