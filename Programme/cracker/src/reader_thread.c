// Fichier reprenant les fonctions relatives a la lecture des fichiers par le thread de lecture

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>

#include "utilities.h"
#include "init.h"
#include "reader_thread.h"

/*
 * Fonction qui lit les hashes des fichiers d'entree dans le buffer de hashes. Quand le buffer est rempli,
 * la fonction met le thread qui l'a lancee en attente et ce jusqu'a ce qu'une place dans le buffer se libere.
 * Quand tous les fichiers ont etes lus, le flag 'all_files_read' dans 'reader_params' est mis a true.
 * Toutes ces operations s'effectuent de maniere sure a l'aide de mutexes.
 *
 * @pre 'reader_params' est non-NULL et les valeurs qu'ils contient sont initialisees correctement (notamment
 *      les valeurs relatives aux parametres entres par l'utilisateur et celles relatives aux buffers).
 * @post Si aucune erreur ne se produit, 'reader_params->all_files_read' est mis a true, les fichiers sont
 *       fermes correctement et l'ensemble des hashes a ete lu et la fonction retourne ((void *) 0). Sinon,
 *       la fonction retourne ((void *) -1) et affiche un message d'erreur sur stderr.
 *
 */

// Attention: on retourne un int caste en void * (pas besoin de faire un int* btw)
void *read_files(void *reader_params) {
	// Recuperation des parametres sous un type adequat
	shared_data_t *shared = (shared_data_t *) reader_params;

	// Allocation de l'espace memoire qui servira de buffer pour le fichier quand il lira les hashes un a un
	uint8_t *read_hash = (uint8_t *) malloc_retry(10, 10,
			shared->hash_length * sizeof(uint8_t));
	if (!(read_hash)) {
		fprintf(stderr,
				"Failed to allocate memory for 'read_hash' in function 'reader_thread.c/read_files'.\n");
		return ((void *) -1);
	}

	int first_free_index; // Variable permettant de savoir quel indice du premier espace libre dans 'hashes_buffer'
	int file;
	char *filename;
	int errcode;  // Variable permettant de gerer les codes d'erreur ci-dessous
	unsigned int hash_count; // TODO: retirer ceci, c'est juste pour l'affichage des hashes (temporaire)

	// Pour chaque fichier de hashes, on lit les hashes un par un
	for (int i = 0; i < shared->user_options->n_files; i++) {
		filename = (shared->user_options->in_files_names)[i];
		if ((file = open(filename, O_RDONLY)) == -1) {
			free(read_hash);
			fprintf(stderr,
					"Failed to open file '%s' in function 'reader_thread.c/read_files'.\n",
					filename);
			return ((void *) -1);
		}

		hash_count = 0;  // TODO: Temporaire, juste pour l'affichage des hashes

		// "Tant que l'on a pas atteint la fin du fichier et que l'on ne rencontre aucune erreur"
		while ((errcode = read(file, (void *) read_hash,
				shared->hash_length * sizeof(uint8_t))) > 0) {
			// On ajoute le hash si le mutex est unlock et si il y a de la place dans le buffer

			// Attente d'un slot libre
			if (sem_wait(shared->hashes_empty) == -1) {
				free(read_hash);
				fprintf(stderr,
						"Failed to access semaphore 'shared->hashes_empty' in function 'reader_thread.c/read_files'.\n");
				return ((void *) -1);
			}

			// On s'approprie le buffer
			if ((errcode = pthread_mutex_lock(shared->hashes_buffer_mtx))) {
				free(read_hash);
				fprintf(stderr,
						"Failed to lock mutex 'shared->hashes_buffer_mtx' in function "
								"'reader_thread.c/read_files'.\n");
				errno = errcode;
				return ((void *) -1);
			}

			// On recupere l'indice du premier slot libre dans le buffer
			if (sem_getvalue(shared->hashes_full, &first_free_index) == -1) {
				free(read_hash);
				fprintf(stderr,
						"Failed to retrieve value from semaphore 'shared->hashes_full' in function "
								"'reader_thread.c/read_files'.\n");
				return ((void *) -1);
			}

			// Attention! TODO: verifier qu'on a bien free l'eventuel hash precedent au niveau du consommateur
			(shared->hashes_buffer)[first_free_index] = read_hash;

			printf("Added hash #%d to buffer: ", ++hash_count);
			print_hash(stdout, read_hash, shared->hash_length);
			printf("\n            Buffer view: ");
			print_hash(stdout, (shared->hashes_buffer)[first_free_index],
					shared->hash_length);
			printf("\n");

			// On libere 'hashes_buffer'
			if ((errcode = pthread_mutex_unlock(shared->hashes_buffer_mtx))) {
				free(read_hash);
				fprintf(stderr,
						"Failed to unlock mutex 'shared->hashes_buffer_mtx' in function "
								"'reader_thread.c/read_files'.\n");
				errno = errcode;
				return ((void *) -1);
			}

			// On prend note qu'un slot supplementaire du buffer est occupe
			if (sem_post(shared->hashes_full) == -1) {
				free(read_hash);
				fprintf(stderr,
						"Failed to access semaphore 'shared->hashes_buffer_mtx' in function "
								"'reader_thread.c/read_files'.\n");
				return ((void *) -1);
			}
		}
		if (errcode == -1) {
			free(read_hash);
			fprintf(stderr,
					"An error occurred while reading file '%s' in function 'reader_thread.c/read_files'.\n",
					filename);
			return ((void *) -1);
		}
		// Si errcode == 0, on a atteint l'EOF sans erreur

		if (close(file) == -1) {
			free(read_hash);
			fprintf(stderr,
					"Failed to close file '%s' in function 'reader_thread.c/read_files'.\n",
					filename);
			return ((void *) -1);
		}
	}
	free(read_hash);
	shared->all_files_read = true;  // Tous les fichiers ont ete lus en entier
	return ((void *) 0);
}
