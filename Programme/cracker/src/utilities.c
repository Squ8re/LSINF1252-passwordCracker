// Fichier qui regroupe diverses fonctions utilitaires

/*
 *
 * Author: Eduardo Vannini (NOMA: 10301700)
 *          Date: 09-05-2019
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "utilities.h"
#include "custom_sleep.h"
#include "parsing.h"
#include "linked_list.h"

/*
 * Fonction qui permet de determiner si un fichier existe ou non.
 *
 * @pre filename != NULL
 * @post Si aucune erreur n'a ete rencontree, la fonction retourne 0 si le fichier existe et 1 sinon.
 *       En cas d'erreur, la fonction affiche un message d'erreur sur stderr et retourne -1 et la
 *       valeur de 'errno' est ajustee.
 */
int file_exists(const char *filename) {
	int file;
	if ((file = open(filename, O_RDONLY)) == -1) {
		return 1;
	}
	if (close(file) == -1) {
		fprintf(stderr,
				"Failed to close file '%s' in function 'utilities.c/file_exists'.\n",
				filename);
		return -1;
	}
	return 0;
}

/*
 * Fonction qui tente d'effectuer un 'malloc' un maximum de 'n_tries' fous et en espacant les tentatives
 * de 'delay_ms' millisecondes.
 *
 * @pre n_tries > 0 et delay_ms > 0
 * @post En cas de reussite de l'un des mallocs, le pointeur (void *) non-NULL retourne par ce malloc est retourne
 *       par la fonction. Si aucun des mallocs n'a reussi, alors un pointeur (void *) egal a NULL est retourne.
 *
 */
void *malloc_retry(unsigned int n_tries, unsigned int delay_ms, size_t size) {
	void *ptr = NULL;
	for (int i = 0; i < n_tries && !ptr; i++) {
		ptr = malloc(size);
		custom_sleep(delay_ms);
	}
	return ptr;
}

/*
 * Fonction qui permet d'afficher les valeurs contenues dans une structure de type 'options_t' et decrivant
 * les differents parametres donnes par l'utilisateur lors du lancement du programme.
 *
 * @pre 'user_options' est non-NULL.
 * @post Le contenu de 'user_options' est affiche proprement sur la sortie standard (stdout).
 *
 */
void display_options(options_t const *user_options) {
	printf(
			"n_threads = %d\nout_file_name = %s\nc_flag = %d\no_flag = %d\nt_flag = %d\nn_files = %d\n",
			user_options->n_threads, user_options->out_file_name,
			(int) user_options->c_flag, (int) user_options->o_flag,
			(int) user_options->t_flag, user_options->n_files);
	for (int i = 0; i < user_options->n_files; i++) {
		printf("input file #%d: %s\n", i + 1,
				(user_options->in_files_names)[i]);
	}
}

/*
 * Fonction permettant de liberer l'ensemble de la memoire occupee par une structure de type 'shared_data_t'.
 * Comme cette structure reprend l'ensemble des valeurs allouees sur le heap pour ce programme, cette fonction
 * permet de liberer tout le heap.
 * NB: Si certains elements dans 'shared' n'ont pas encore ete alloues, ce n'est pas un souci.
 *
 *
 * @pre 'shared' est non-NULL.
 * @post L'ensemble de la memoire occupee par 'shared' est liberee a l'aide de 'free'.
 *
 */
void free_all(shared_data_t *shared) {
	// NOTE: Puisque 'free(NULL)' ne constitue pas une erreur, on peut faire un free sur tout le contenu
	//       de 'shared' sans trop se soucier de ce qui a ete malloc ou pas. 'shared' peut donc n'etre que
	//       partiellement initialise quand il est passe dans cette fonction.

	if (shared) {
		// Free les user_options
		if (shared->user_options) {
			for (int i = 0; i < shared->user_options->n_files; i++) {
				free((shared->user_options->in_files_names)[i]);
			}

			free(shared->user_options->in_files_names);
			free(shared->user_options->out_file_name);
			free(shared->user_options);
		}

		// Free les threads_data
		if (shared->threads_data) {
			free(shared->threads_data->reader);
			free(shared->threads_data->reversers);
			free(shared->threads_data->cand_manager);
			free(shared->threads_data);
		}

		// Free les buffers
		if (shared->hashes_buffer) {
			for (int i = 0; i < shared->buffer_length; i++) {
				free((shared->hashes_buffer)[i]);
			}
			free(shared->hashes_buffer);
		}
		if (shared->hashes_buffer) {
			for (int i = 0; i < shared->buffer_length; i++) {
				free((shared->reversed_buffer)[i]);
			}
			free(shared->reversed_buffer);
		}

		// Free la linked list
		remove_all(&(shared->best_candidates));

		// Free les outils de coordination des threads
		// Pas besoin de tester car free(NULL) ne pose aucun souci
		free(shared->hashes_buffer_mtx);
		free(shared->reversed_buffer_mtx);
		free(shared->hashes_empty);
		free(shared->hashes_full);
		free(shared->reversed_empty);
		free(shared->reversed_full);

		// Free l'espace de shared
		free(shared);
	}
}
