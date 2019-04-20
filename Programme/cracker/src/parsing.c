// Fichier qui regroupe le code utile a la lecture des options donnees en ligne de commande par l'utilisateur

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include "parsing.h"
#include "utilities.h"


// Fonction qui ecrit dans l'instance '*user_options' afin que les valeurs s'y trouvant correspondent aux valeurs que l'on
// peut retrouver dans 'argv'.
// Ne doit s'utiliser que sur un 'user_options' dont les arrays sont NULL, autrement il y aura des fuites de memoire car
// on malloc des espaces pour les nouveaux arrays sans free les arrays potentiellement deja presents (bien qu'on pourrait
// le faire sans aucun probleme)
void parse_options(int argc, char **argv, options_t *user_options){
	opterr = 0;  // Ne pas autoriser getopt a ecrire sur stderr (on va faire nos propres messages d'erreur)
	int opt;

	while((opt = getopt(argc, argv, "co:t:")) != -1){
		switch(opt){
		case 'c':
			if(!user_options->c_flag){
				user_options->c_flag = true;
			}
			else{
				// L'option "-c" a ete donnee plusieurs fois
				fprintf(stderr, "Option '-c' used more than once.\n");
				display_usage(stderr);
				exit(EXIT_FAILURE);
			}
			break;
		case 'o':
			if(!user_options->o_flag){
				user_options->o_flag = true;
				user_options->out_file_name = (char *) malloc_retry(10, 10, sizeof(char) * (1 + strlen(optarg)));  // Ne pas oublier le '\0'!
				if(!user_options->out_file_name){
					fprintf(stderr, "Memory could not be accessed (malloc failure).\n");
					exit(EXIT_FAILURE);
				}

				// TODO: On ne verifie pas que le fichier existe car si il n'existe pas, on pourrait vouloir le creer

				strcpy(user_options->out_file_name, optarg);
			}
			else{
				// L'option "-o" a ete donnee plusieurs fois
				fprintf(stderr, "Option '-o' used more than once.\n");
				display_usage(stderr);
				exit(EXIT_FAILURE);
			}
			break;
		case 't':
			if(!user_options->t_flag){
				user_options->t_flag = true;
				int arg_value = atoi(optarg);
				if(arg_value <= 0){
					// On aura 0 quand l'utilisateur a demande 0 threads de calcul ou quand atoi a echoue son execution
					// (ce qui arrive par exemple si la valeur entree ne correspond pas a un chiffre)
					fprintf(stderr, "'%s' is not a valid value for option '-t'. The value must be a positive integer.\n", optarg);
					display_usage(stderr);
					exit(EXIT_FAILURE);
				}
				user_options->n_threads = (unsigned int) arg_value;
			}
			else{
				// L'option "-t" a ete utilisee plusieurs fois
				fprintf(stderr, "Option '-t' used more than once.\n");
				display_usage(stderr);
				exit(EXIT_FAILURE);
			}
			break;
		case '?':
			if(optopt == 't' || optopt == 'o'){
				fprintf(stderr, "Option '-%c' requires a value.\n", optopt);
			}
			else if(isprint(optopt)){
				fprintf(stderr, "Unknown option '-%c'.\n", optopt);
			}
			else{
				fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
			}
			display_usage(stderr);
			exit(EXIT_FAILURE);
			break;
		default:
			fprintf(stderr, "Got unexpected error while parsing options.\n");
			exit(EXIT_FAILURE);
			break;
		}
	}

	// Recuperation des noms des fichiers de hashes
	user_options->n_files = argc - optind;
	if(user_options->n_files == 0){
		fprintf(stderr, "At least one input file must be provided.\n");
		display_usage(stderr);
		exit(EXIT_FAILURE);
	}

	user_options->in_files_names = (char **) malloc_retry(10, 10, sizeof(char *) * (user_options->n_files));
	if(!user_options){
		fprintf(stderr, "Memory could not be accessed (malloc failure).\n");
		exit(EXIT_FAILURE);
	}
	for(int i = optind;  i < argc; i++){
		char *filename = argv[i];
		// Verifier que le fichier existe
		if(!file_exists(filename)){
			fprintf(stderr, "File '%s' could not be found.\n", filename);
			exit(EXIT_FAILURE);
		}
		user_options->in_files_names[i - optind] = (char *) malloc_retry(10, 10, sizeof(char *) * (1 + strlen(filename)));
		if(!user_options->in_files_names[i - optind]){
			fprintf(stderr, "Memory could not be accessed (malloc failure).\n");
			exit(EXIT_FAILURE);
		}
		strcpy(user_options->in_files_names[i - optind], filename);
	}
}
