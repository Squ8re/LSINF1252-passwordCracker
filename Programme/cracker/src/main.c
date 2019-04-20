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
#include "parsing.h"
#include "utilities.h"
#include "init.h"
#include "linked_list.h"

#define HASH_LENGTH 32  // Il y a 32 bytes par hash
#define BUFFER_RATIO 2  // Ratio: (longueur des buffers)/(nombre de threads de calcul)


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
	// Parsing des options venant de la ligne de commande
	options_t user_options = {.n_threads = 1, .out_file_name = NULL, .in_files_names = NULL,
			                  .n_files = 0, .c_flag = false, .o_flag = false, .t_flag = false};
	parse_options(argc, argv, &user_options);

	// Affichage des options obenues (temporaire)
	printf("n_threads = %d\nout_file_name = %s\nc_flag = %d\no_flag = %d\nt_flag = %d\nn_files = %d\n", user_options.n_threads,
			user_options.out_file_name, (int) user_options.c_flag, (int) user_options.o_flag, (int) user_options.t_flag,
			user_options.n_files);
	for(int i = 0; i < user_options.n_files; i++){
		printf("input file #%d: %s\n", i + 1, user_options.in_files_names[i]);
	}

	// Creation des buffers
	uint8_t **hashes_buffer;
	char **reversed_buffer;
	linked_list_t best_candidates;  // Liste chainee qui contiendra les meilleurs candidats a tout instant
	create_buffers(&hashes_buffer, &reversed_buffer, &user_options, BUFFER_RATIO, HASH_LENGTH);


	// Creation et demarrage des threads (rappel: sous windows ceci pourrait ne pas marcher)
	threads_t program_threads = {.read_func = &read_thread_placeholder,
			                     .reverse_func = &reverse_thread_placeholder,
	                             .cand_func = &cand_thread_placeholder};
	launch_threads(&program_threads, &user_options);

	// temporaire
	pthread_t reader = program_threads.reader;
	pthread_t *reversers = program_threads.reversers;
	pthread_t cand_manager = program_threads.cand_manager;
	// </temporaire>

	// Terminaison des threads
	// TODO: Il faudra remplacer les "NULL"
	// TODO: utiliser la structure 'threads'
	if(pthread_join(reader, NULL) != 0){
		fprintf(stderr, "Error with pthread_join.\n");
	}

	for(int i = 0; i < user_options.n_threads; i++){
		if(pthread_join(reversers[i], NULL) != 0){
			fprintf(stderr, "Error with pthread_join.\n");
		}
	}

	if(pthread_join(cand_manager, NULL) != 0){
		fprintf(stderr, "Error with pthread_join.\n");
	}

	return EXIT_SUCCESS;
}
