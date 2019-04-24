#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "parsing.h"
#include "utilities.h"
#include "init.h"
#include "reader_thread.h"

#define HASH_LENGTH 32  // Il y a 32 bytes par hash
#define BUFFER_RATIO 2  // Ratio: (longueur des buffers)/(nombre de threads de calcul)

// Fonction temporaire qui devra etre remplacee par la fonction d'inversion de hashes
void *reverse_thread_placeholder(void *param) {
	printf("'reverse_thread_placeholder' was called.\n");
	fflush(stdout);
	return ((void *) 0);
}

// Fonction temporaire qui devra etre remplacee par la fonction de gestion des candidats
void *cand_thread_placeholder(void *param) {
	printf("'cand_thread_placeholder' was called.\n");
	fflush(stdout);
	return ((void *) 0);
}

int main(int argc, char **argv) {
	// Mettre errno a 0 (pas d'erreur pour l'instant)
	errno = 0;

	// Structure contenant les information partagees entre les threads
	shared_data_t *shared = (shared_data_t *) malloc_retry(10, 10,
			sizeof(shared_data_t));
	if (!shared) {
		fprintf(stderr,
				"Failed to allocate memory for 'shared' in function 'main.c/main'.\n");
		return EXIT_FAILURE;
	}

	shared->buffer_ratio = BUFFER_RATIO;
	shared->hash_length = HASH_LENGTH;

	// Parsing des options venant de la ligne de commande
	options_t *user_options = (options_t *) malloc_retry(10, 10,
			sizeof(options_t));
	if (!user_options) {
		fprintf(stderr,
				"Failed to allocate memory for 'user_options' in function 'main.c/main'.\n");
		free_all(shared);
		return EXIT_FAILURE;
	}

	if (parse_options(argc, argv, user_options) == -1) {
		fprintf(stderr, "Failed to parse options in 'main.c/main'.\n"); // parse_options ne touche pas a 'errno'
		free_all(shared);
		return EXIT_FAILURE;
	}
	shared->user_options = user_options; // Ajout dans les informations partagees
	shared->buffer_length = BUFFER_RATIO * user_options->n_threads;

	// Affichage des options obtenues (temporaire)
	display_options(user_options);

	// Creation des buffers
	if (create_buffers(shared) == -1) {
		fprintf(stderr,
				"Failed to create buffers in 'main.c/main' (errno=%d : %s).\n",
				errno, strerror(errno));
		free_all(shared);
		return EXIT_FAILURE;
	}

	// Creation des outils de coordination entre les threads
	// Voir "init.h" pour le contenu de coord_t.
	if (create_coordinators(shared) == -1) {
		fprintf(stderr,
				"Failed to create coordinators in 'main.c/main' (errno=%d : %s).\n",
				errno, strerror(errno));
		free_all(shared);
		return EXIT_FAILURE;
	}

	// Creation et demarrage des threads (rappel: sous windows ceci pourrait ne pas marcher)
	shared->threads_data = (threads_t *) malloc_retry(10, 10,
			sizeof(threads_t));
	if (!(shared->threads_data)) {
		fprintf(stderr,
				"Failed to allocate memory for 'shared->threads_data' in function 'main/main.c'.\n");
		free_all(shared);
		return EXIT_FAILURE;
	}

	shared->threads_data->read_func = &read_files;
	shared->threads_data->reverse_func = &reverse_thread_placeholder; // TODO: A modifier
	shared->threads_data->cand_func = &cand_thread_placeholder; // TODO: A modifier

	if (launch_threads(shared) == -1) {
		fprintf(stderr,
				"Failed to launch threads in 'main.c/main' (errno=%d : %s).\n",
				errno, strerror(errno));
		free_all(shared);
		return EXIT_FAILURE;
	}

	// Terminaison des threads
	// TODO: Il faudra remplacer les "NULL" pour gerer les erreurs
	int errcode;
	if ((errcode = pthread_join(*(shared->threads_data->reader), NULL)) != 0) {
		fprintf(stderr,
				"Failed to join thread 'reader' in function 'main.c/main' (errno=%d : %s).\n",
				errcode, strerror(errcode));
		free_all(shared);
		return EXIT_FAILURE;
	}

	for (int i = 0; i < user_options->n_threads; i++) {
		if ((errcode = pthread_join((shared->threads_data->reversers)[i], NULL))
				!= 0) {
			fprintf(stderr,
					"Failed to join thread 'reversers[%d]' in function 'main.c/main' "
							"(errno=%d : %s)", i, errcode, strerror(errcode));
		}
	}

	if ((errcode = pthread_join(*(shared->threads_data->cand_manager), NULL))
			!= 0) {
		fprintf(stderr,
				"Failed to join thread 'cand_manager' in function 'main.c/main' (errno=%d : %s).\n",
				errcode, strerror(errcode));
		free_all(shared);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
