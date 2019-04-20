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
	int k = 2;  // k tel que le buffer de hash est de longueur k * nombre de threads de calcul (TODO: a mettre en #define?)
	int hash_length = 32;  // Longueur d'un hash en bytes (TODO: a passer en #define, ou a hardcoder directement?)

	// Buffer des hashes
	uint8_t **hashes_buffer = (uint8_t **) malloc_retry(10, 10, k * user_options.n_threads * sizeof(uint8_t *));
	if(!hashes_buffer){
		fprintf(stderr, "Memory could not be accessed (malloc failure).\n");
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i < k * user_options.n_threads; i++){
		hashes_buffer[i] = (uint8_t *) malloc_retry(10, 10, hash_length * sizeof(uint8_t));
		if(!hashes_buffer[i]){
			fprintf(stderr, "Memory could not be accessed (malloc failure).\n");
			exit(EXIT_FAILURE);
		}
	}

	// Buffer des mots de passe en clair (apres inversion, donc)
	uint8_t **reversed_buffer = (uint8_t **) malloc_retry(10, 10, k * user_options.n_threads * sizeof(uint8_t *));
	if(!reversed_buffer){
		fprintf(stderr, "Memory could not be accessed (malloc failure).\n");
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i < k * user_options.n_threads; i++){
		reversed_buffer[i] = (uint8_t *) malloc_retry(10, 10, hash_length * sizeof(uint8_t));
		if(!reversed_buffer[i]){
			fprintf(stderr, "Memory could not be accessed (malloc failure).\n");
			exit(EXIT_FAILURE);
		}
	}

	// Creation des threads (rappel: sous windows ceci ne pourrait pas marcher)
	// Rappel: il faut un argument special dans la commande gcc pour tenir compte des threads
	pthread_t reader;  // On utilise un seul thread de lecture comme mentionne dans l'architecture
	pthread_t *reversers = (pthread_t *) malloc_retry(10, 10,
			user_options.n_threads * sizeof(pthread_t)); // Les threads qui feront l'inversion de hashes

	if(!reversers){
		fprintf(stderr, "Memory could not be accessed (malloc failure).\n");
		exit(EXIT_FAILURE);
	}
	pthread_t cand_manager;  // "candidate manager": thread qui s'arrange pour ne garder que les meilleurs pwd

	if(pthread_create(&reader, NULL, &read_thread_placeholder, NULL) != 0){
		fprintf(stderr, "Thread could not be created.\n");
		exit(EXIT_FAILURE);
	}

	for(int i = 0; i < user_options.n_threads; i++){
		if(pthread_create(&reversers[i], NULL, &reverse_thread_placeholder, NULL) != 0){
			fprintf(stderr, "Thread could not be created.\n");
			exit(EXIT_FAILURE);
		}
	}

	if(pthread_create(&cand_manager, NULL, &cand_thread_placeholder, NULL) != 0){
		fprintf(stderr, "Thread could not be created.\n");
		exit(EXIT_FAILURE);
	}


	// Terminaison des threads
	// TODO: Il faudra remplacer les "NULL"
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
