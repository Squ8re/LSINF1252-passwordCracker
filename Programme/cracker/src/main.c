#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <semaphore.h>
#include <errno.h>

#include "parsing.h"
#include "utilities.h"
#include "init.h"
#include "linked_list.h"

#define HASH_LENGTH 32  // Il y a 32 bytes par hash
#define BUFFER_RATIO 2  // Ratio: (longueur des buffers)/(nombre de threads de calcul)


// Fonction qui lit les hashes des fichiers d'entree dans le buffer de hashes
void *read_files(void *reader_data_param){
	reader_data_t *reader_data = (reader_data_t *) reader_data_param;
	uint8_t *read_hash = (uint8_t *) malloc_retry(10, 10, HASH_LENGTH * sizeof(uint8_t));
	check_malloc((void *) read_hash, "Failed to allocate memory for 'read_hash' in function 'read_files'");
	for(int i = 0; i < reader_data->n_files; i++){
		char *filename = (reader_data->in_files_names)[i];
		int file = open(filename, O_RDONLY);
		if(file == -1){
			fprintf(stderr, "Failed to open file '%s' in function 'read_files': %s\n", filename, strerror(errno));
			exit(EXIT_FAILURE);
		}

		int errcode = read(file, (void *) read_hash, HASH_LENGTH * sizeof(uint8_t));
		int j = 0;
		while(errcode > 0){
			// On ajoute le hash si le mutex est unlock et si il y a de la place dans le buffer
			sem_wait(reader_data->hashes_empty);  // Attente d'un slot libre
			pthread_mutex_lock(reader_data->hashes_buffer_mtx);  // On s'approprie le buffer

			pthread_mutex_unlock(reader_data->hashes_buffer_mtx);  // On libere le buffer
			sem_post(reader_data->hashes_full);  // On notifie qu'un slot supplementaire du buffer est occupe

			printf("Added hash #%d to buffer: ", ++j);
			print_hash(stdout, read_hash, HASH_LENGTH);
			printf("\n");

			errcode = read(file, (void *) read_hash, HASH_LENGTH * sizeof(uint8_t));
		}
		if(errcode == -1){
			fprintf(stderr, "An error occurred while reading file '%s' in function 'read_files': %s\n", filename, strerror(errno));
			exit(EXIT_FAILURE);
		}
		// Si errcode == 0, on a atteint l'EOF.

		if(close(file) == -1){
			fprintf(stderr, "Failed to close file '%s' in function 'read_files': %s\n", filename, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	free(read_hash);
	*(reader_data->all_files_read) = true;
	return NULL;
}

// Fonction temporaire qui devra etre remplacee par la fonction de lecture de fichiers
void *read_thread_placeholder(void *param){
	printf("'read_thread_placeholder' was called.\n");
	fflush(stdout);
	return NULL;
}


// Fonction temporaire qui devra etre remplacee par la fonction d'inversion de hashes
void *reverse_thread_placeholder(void *param){
	printf("'reverse_thread_placeholder' was called.\n");
	fflush(stdout);
	return NULL;
}


// Fonction temporaire qui devra etre remplacee par la fonction de gestion des candidats
void *cand_thread_placeholder(void *param){
	printf("'cand_thread_placeholder' was called.\n");
	fflush(stdout);
	return NULL;
}


int main(int argc, char **argv){
	// Mettre errno a 0 (pas d'erreur pour l'instant)
	errno = 0;

	// Parsing des options venant de la ligne de commande
	options_t *user_options = (options_t *) malloc_retry(10, 10, sizeof(options_t));
	check_malloc((void *) user_options, "Failed to allocate memory for 'user_options' in function 'main.c/main'");
	user_options->n_threads = 1;
	user_options->out_file_name = NULL;
	user_options->in_files_names = NULL;
	user_options->n_files = 0;
	user_options->c_flag = false;
	user_options->o_flag = false;
	user_options->t_flag = false;

	parse_options(argc, argv, user_options);

	// Affichage des options obenues (temporaire)
	printf("n_threads = %d\nout_file_name = %s\nc_flag = %d\no_flag = %d\nt_flag = %d\nn_files = %d\n", user_options->n_threads,
			user_options->out_file_name, (int) user_options->c_flag, (int) user_options->o_flag, (int) user_options->t_flag,
			user_options->n_files);
	for(int i = 0; i < user_options->n_files; i++){
		printf("input file #%d: %s\n", i + 1, (user_options->in_files_names)[i]);
	}

	// Creation des buffers
	uint8_t **hashes_buffer;
	char **reversed_buffer;
	linked_list_t best_candidates;  // Liste chainee qui contiendra les meilleurs candidats a tout instant
	create_buffers(&hashes_buffer, &reversed_buffer, user_options, BUFFER_RATIO, HASH_LENGTH);

	// Creation des outils de coordination entre les threads
	// Voir "init.h" pour le contenu de coord_t.
	coord_t *coord_ptr = create_coordinators(BUFFER_RATIO * user_options->n_threads);

	// Creation et demarrage des threads (rappel: sous windows ceci pourrait ne pas marcher)
	reader_data_t *reader_params = (reader_data_t *) malloc_retry(10, 10, sizeof(reader_data_t));
	check_malloc((void *) reader_params, "Failed to allocate memory for 'reader_params' in function 'main.c/main'");

	reader_params->n_files = user_options->n_files;
	reader_params->in_files_names = user_options->in_files_names;
	reader_params->all_files_read = coord_ptr->all_files_read;
	reader_params->hashes_buffer = &hashes_buffer;
	reader_params->hashes_buffer_mtx = coord_ptr->hashes_buffer_mtx;
	reader_params->hashes_empty = coord_ptr->hashes_empty;
	reader_params->hashes_full = coord_ptr->hashes_full;

	threads_t program_threads = {.read_func = &read_files,
			                     .reverse_func = &reverse_thread_placeholder,
	                             .cand_func = &cand_thread_placeholder,
	                             .reader_params = reader_params};
	launch_threads(&program_threads, user_options);


	// temporaire
	pthread_t reader = program_threads.reader;
	pthread_t *reversers = program_threads.reversers;
	pthread_t cand_manager = program_threads.cand_manager;
	// </temporaire>

	// Terminaison des threads
	// TODO: Il faudra remplacer les "NULL" (pas sur en fait, ca depend comment on veut gerer les erreurs)
	// TODO: utiliser la structure 'threads'
	if(pthread_join(reader, NULL) != 0){
		handle_error("Failed to join thread 'reader' in function 'main.c/main'");
	}

	for(int i = 0; i < user_options->n_threads; i++){
		if(pthread_join(reversers[i], NULL) != 0){
			handle_error("Failed to join thread 'reversers[i]' in function 'main.c/main'");
		}
	}

	if(pthread_join(cand_manager, NULL) != 0){
		handle_error("Failed to join thread 'cand_manager' in function 'main.c/main'");
	}

	return EXIT_SUCCESS;
}
