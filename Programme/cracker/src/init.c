// Fichier contenant du code utile a l'initialisation du programme faite dans main

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#include "utilities.h"
#include "parsing.h"
#include "init.h"

/*
 * Fonction qui permet d'initialiser les buffers.
 *
 * @pre 'shared' est non-NULL et 'shared->user_options' a ete initialise correctement.
 * @post Si aucune erreur n'a ete rencontree, les buffers de 'shared' sont initialises correctement
 *       et 0 et retourne. En cas d'erreur, un message d'erreur est affiche sur stderr, -1 est retourne
 *       et la valeur de 'errno' est ajustee.
 *
 */
int create_buffers(shared_data_t *shared) {
	// Allocation de la memoire pour le buffer 'hashes_buffer' qui contiendra les hashes non inverses
	shared->hashes_buffer = (uint8_t **) malloc_retry(10, 10,
			shared->buffer_length * sizeof(uint8_t *));
	if (!(shared->hashes_buffer)) {
		fprintf(stderr,
				"Failed to allocate memory for 'shared->hashes_buffer'.\n");
		return -1;
	}

	// Allocation de la memoire pour les hashes qui seront contenus dans le buffer 'hashes_buffer'
	for (int i = 0; i < shared->buffer_length; i++) {
		(shared->hashes_buffer)[i] = (uint8_t *) malloc_retry(10, 10,
				shared->hash_length * sizeof(uint8_t));
		if (!(shared->hashes_buffer)[i]) {
			for (int j = 0; j < i; j++) {
				free((shared->hashes_buffer)[j]);
			}
			free(shared->hashes_buffer);
			fprintf(stderr,
					"Failed to allocate memory for '(shared->hashes_buffer)[%d]' "
							"in function 'init.c/create_buffers'.\n", i);
			return -1;
		}
	}

	// Allocation de la memoire pour le buffer 'reversed_buffer' qui contiendra les mots de passe en clair
	shared->reversed_buffer = (char **) malloc_retry(10, 10,
			shared->buffer_length * sizeof(char *));
	if (!(shared->reversed_buffer)) {
		for (int i = 0; i < shared->buffer_length; i++) {
			free((shared->hashes_buffer)[i]);
		}
		free(shared->hashes_buffer);
		fprintf(stderr,
				"Failed to allocate memory for 'shared->reversed_buffer' in function"
						" 'init.c/create_buffers'.\n");
		return -1;
	}

	// NOTE: Contrairement a hashes_buffer, ou l'on connait deja la taille des hashes et ou on peut donc
	//       initialiser les espaces qui contiendront les hashes, on ne peut pas faire de meme avec
	//       reversed_buffer, car on ne sait pas encore la taille qu'auront les char * representant les
	//       mots de passe en clair.
	// TODO: Il faudra faire attention, quand on initialise ces espaces dans 'reversed_buffer', a tenir
	//       compte du '\0' dans le malloc.

	return 0;
}

/*
 * Fonction qui permet de creer et de lancer les threads du programme.
 *
 * @pre 'shared' est non-NULL et 'shared->user_options' est initialise correctement.
 * @post Si aucune erreur n'a ete rencontree, les threads sont crees et lances et la fonction retourne 0.
 * 		 Sinon, la fonction affiche un message d'erreur sur 'stderr', ajuste la valeur de 'errno' et retourne -1;
 *
 */
int launch_threads(shared_data_t *shared) {
	// Allocation de la memoire pour le thread de lecture 'reader'
	shared->threads_data->reader = (pthread_t *) malloc_retry(10, 10,
			sizeof(pthread_t));
	if (!(shared->threads_data->reader)) {
		fprintf(stderr,
				"Failed to allocate memory for 'shared->threads_data->reader' in"
						" function 'init.c/launch_threads'.\n");
		return -1;
	}

	// Allocation de la memoire pour le tableau 'reversers' contenant les threads d'inversion
	shared->threads_data->reversers = (pthread_t *) malloc_retry(10, 10,
			shared->user_options->n_threads * sizeof(pthread_t *));
	if (!(shared->threads_data->reversers)) {
		free(shared->threads_data->reader);
		fprintf(stderr,
				"Failed to allocate memory for 'shared->threads_data->reversers'"
						" in function 'init.c/launch_threads'.\n");
		return -1;
	}

	// Allocation de la memoire pour le thread 'cand_manager' qui gerera les "meilleurs candidats"
	shared->threads_data->cand_manager = (pthread_t *) malloc_retry(10, 10,
			sizeof(pthread_t));
	if (!(shared->threads_data->cand_manager)) {
		free(shared->threads_data->reader);
		free(shared->threads_data->reversers);
		fprintf(stderr, "Failed to allocate memory for 'shared->threads_data"
				"->cand_manager' in function 'init.c/launch_threads'.\n");
		return -1;
	}

	int errcode;  // Variable permettant de gerer les codes d'erreur ci-dessous

	// Lancement du thread de lecture 'reader'
	if ((errcode = pthread_create(shared->threads_data->reader, NULL,
			shared->threads_data->read_func, (void *) shared)) != 0) {
		free(shared->threads_data->reader);
		free(shared->threads_data->reversers);
		free(shared->threads_data->cand_manager);
		fprintf(stderr,
				"Failed to create thread 'shared->threads_data->reader' in function 'init.c/launch_threads'.\n");
		errno = errcode;
		return -1;
	}

	// Lancement des threads d'inversion des hashes
	for (int i = 0; i < shared->user_options->n_threads; i++) {
		// TODO: Remplacer le dernier "NULL" par l'argument des threads consommateurs
		if ((errcode = pthread_create(&(shared->threads_data->reversers)[i],
				NULL, shared->threads_data->reverse_func, NULL)) != 0) {
			free(shared->threads_data->reader);
			free(shared->threads_data->reversers);
			free(shared->threads_data->cand_manager);
			fprintf(stderr,
					"Failed to create thread 'shared->threads_data->reversers[%d]' in "
							"function 'init.c/launch_threads'", i);
			errno = errcode;
			return -1;
		}
	}

	// Lancement du thread 'cand_manager', gerant les "meilleurs candidats"
	// TODO: Remplacer le dernier "NULL" par l'argument du thread cand_manager
	if ((errcode = pthread_create(shared->threads_data->cand_manager, NULL,
			shared->threads_data->cand_func, NULL)) != 0) {
		free(shared->threads_data->reader);
		free(shared->threads_data->reversers);
		free(shared->threads_data->cand_manager);
		fprintf(stderr,
				"Failed to create thread 'shared->threads_data->cand_manager' in "
						"function 'init.c/launch_threads'.\n");
		errno = errcode;
		return -1;
	}

	return 0;
}

/*
 * Fonction qui permet de creer les differents outils de coordination des differents threads du programme
 * (mutexes, semaphores, etc).
 *
 * @pre 'shared' est non-NULL et 'shared->buffer_length' a ete initialise.
 * @post Si aucune erreur n'est rencontree, les outils de coordination contenus dans 'shared' sont initialises
 *       et la fonction retourne 0, sinon, la fonction affiche un message d'erreur dans stderr puis retourne -1
 *       et ajuste la valeur de 'errno'.
 *
 */
int create_coordinators(shared_data_t *shared) {
	shared->all_files_read = false;
	// Pas besoin de mutex pour all_files_read car seul le thread de lecture va le modifier
	// Il faudra mettre un mutex si on utilise plusieurs threads de lecture (et encore, pas sur...)

	// Allocation de la memoire pour le mutex du buffer 'hashes_buffer'
	shared->hashes_buffer_mtx = (pthread_mutex_t *) malloc_retry(10, 10,
			sizeof(pthread_mutex_t));
	if (!(shared->hashes_buffer_mtx)) {
		fprintf(stderr,
				"Failed to allocate memory for 'shared->hashes_buffer_mtx'"
						" in function 'init.c/create_coordinators'.\n");
		return -1;
	}

	// Allocation de la memoire pour le mutex du buffer 'reversed_buffer'
	shared->reversed_buffer_mtx = (pthread_mutex_t *) malloc_retry(10, 10,
			sizeof(pthread_mutex_t));
	if (!(shared->reversed_buffer_mtx)) {
		free(shared->hashes_buffer_mtx);
		fprintf(stderr,
				"Failed to allocate memory for 'shared->reversed_buffer_mtx'"
						" in function 'init.c/create_coordinators'.\n");
		return -1;
	}

	// Allocation de la memoire pour le premier semaphore du buffer 'hashes_buffer'
	shared->hashes_empty = (sem_t *) malloc_retry(10, 10, sizeof(sem_t));
	if (!(shared->hashes_empty)) {
		free(shared->hashes_buffer_mtx);
		free(shared->reversed_buffer_mtx);
		fprintf(stderr, "Failed to allocate memory for 'shared->hashes_empty'"
				" in function 'init.c/create_coordinators'.\n");
		return -1;
	}

	// Allocation de la memoire pour le second semaphore du buffer 'hashes_buffer'
	shared->hashes_full = (sem_t *) malloc_retry(10, 10, sizeof(sem_t));
	if (!(shared->hashes_full)) {
		free(shared->hashes_buffer_mtx);
		free(shared->reversed_buffer_mtx);
		free(shared->hashes_empty);
		fprintf(stderr, "Failed to allocate memory for 'shared->hashes_full'"
				" in function 'init.c/create_coordinators'.\n");
		return -1;
	}

	// Allocation de la memoire pour le premier semaphore du buffer 'reversed_buffer'
	shared->reversed_empty = (sem_t *) malloc_retry(10, 10, sizeof(sem_t));
	if (!(shared->reversed_empty)) {
		free(shared->hashes_buffer_mtx);
		free(shared->reversed_buffer_mtx);
		free(shared->hashes_empty);
		free(shared->hashes_full);
		fprintf(stderr, "Failed to allocate memory for 'shared->reversed_empty'"
				" in function 'init.c/create_coordinators'.\n");
		return -1;
	}

	// Allocation de la memoire pour le second semaphore du buffer 'reversed_buffer'
	shared->reversed_full = (sem_t *) malloc_retry(10, 10, sizeof(sem_t));
	if (!(shared->reversed_full)) {
		free(shared->hashes_buffer_mtx);
		free(shared->reversed_buffer_mtx);
		free(shared->hashes_empty);
		free(shared->hashes_full);
		free(shared->reversed_empty);
		fprintf(stderr, "Failed to allocate memory for 'shared->reversed_full'"
				" in function 'init.c/create_coordinators'.\n");
		return -1;
	}

	int errcode;  // Variable pour gerer les codes d'erreur ci-dessous

	// Initialisation du mutex du buffer 'hashes_buffer'
	if ((errcode = pthread_mutex_init(shared->hashes_buffer_mtx, NULL))) {
		free(shared->hashes_buffer_mtx);
		free(shared->reversed_buffer_mtx);
		free(shared->hashes_empty);
		free(shared->hashes_full);
		free(shared->reversed_empty);
		free(shared->reversed_full);
		fprintf(stderr,
				"Failed to initialize mutex 'shared->hashes_buffer_mtx' in"
						" function 'init.c/create_coordinators'.\n");
		errno = errcode;
		return -1;
	}

	// Initialisation du mutex du buffer 'reversed_buffer'
	if ((errcode = pthread_mutex_init(shared->reversed_buffer_mtx, NULL))) {
		free(shared->hashes_buffer_mtx);
		free(shared->reversed_buffer_mtx);
		free(shared->hashes_empty);
		free(shared->hashes_full);
		free(shared->reversed_empty);
		free(shared->reversed_full);
		fprintf(stderr,
				"Failed to initialize mutex 'shared->reversed_buffer_mtx' in"
						" function 'init.c/create_coordinators'.\n");
		errno = errcode;
		return -1;
	}

	// Initialisation du premier semaphore du buffer 'hashes_buffer'
	if (sem_init(shared->hashes_empty, 0, shared->buffer_length)) {
		free(shared->hashes_buffer_mtx);
		free(shared->reversed_buffer_mtx);
		free(shared->hashes_empty);
		free(shared->hashes_full);
		free(shared->reversed_empty);
		free(shared->reversed_full);
		fprintf(stderr,
				"Failed to initialize semaphore 'shared->hashes_empty' in function 'init.c/create_coordinators'.\n");
		return -1;
	}

	// Initialisation du second semaphore du buffer 'hashes_buffer'
	if (sem_init(shared->hashes_full, 0, 0)) {
		free(shared->hashes_buffer_mtx);
		free(shared->reversed_buffer_mtx);
		free(shared->hashes_empty);
		free(shared->hashes_full);
		free(shared->reversed_empty);
		free(shared->reversed_full);
		fprintf(stderr,
				"Failed to initialize semaphore 'shared->hashes_full' in function 'init.c/create_coordinators'.\n");
		return -1;
	}

	// Initialisation du premier semaphore du buffer 'reversed_buffer'
	if (sem_init(shared->reversed_empty, 0, shared->buffer_length)) {
		free(shared->hashes_buffer_mtx);
		free(shared->reversed_buffer_mtx);
		free(shared->hashes_empty);
		free(shared->hashes_full);
		free(shared->reversed_empty);
		free(shared->reversed_full);
		fprintf(stderr,
				"Failed to initialize semaphore 'shared->reversed_empty' in function 'init.c/create_coordinators'.\n");
		return -1;
	}

	// Initialisation du second semaphore du buffer 'reversed_buffer'
	if (sem_init(shared->reversed_full, 0, 0)) {
		free(shared->hashes_buffer_mtx);
		free(shared->reversed_buffer_mtx);
		free(shared->hashes_empty);
		free(shared->hashes_full);
		free(shared->reversed_empty);
		free(shared->reversed_full);
		fprintf(stderr,
				"Failed to initialize semaphore 'shared->reversed_full' in function 'init.c/create_coordinators'.\n");
		return -1;
	}

	return 0;
}
