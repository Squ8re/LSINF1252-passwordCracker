// Fichier qui regroupe le code utile a la lecture des options donnees en ligne de commande par l'utilisateur

/*
 *
 * Author: Eduardo Vannini (NOMA: 10301700)
 *          Date: 09-05-2019
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#include "parsing.h"
#include "utilities.h"

/*
 * Fonction qui remplit 'user_options' avec les valeurs issues de 'argv', de sorte a obtenir une structure
 * contenant les parametres donnes par l'utilisateur lors du lancement du programme.
 *
 * @pre 'argc' doit etre egal au nombre d'elements dans 'argv'. 'user_options' est non-NULL et les differents
 *      attributs de 'user_options' ne doivent pas etre initialises (sinon il y aura une fuite de memoire car
 *      on ne les aura pas free quand cela aurait ete necessaire).
 *
 * @post Si aucune erreur n'est rencontree, la structure 'user_options' est remplie pour refleter le contenu
 *       de argv, puis la fonction retourne 0. Sinon, la fonction retourne -1 *sans* avoir ajuste la valeur de
 *       'errno' mais en ayant affiche un message d'erreur sur stderr.
 */
int parse_options(int argc, char **argv, options_t *user_options) {
	// Valeurs par defaut, mises de maniere preventive
	user_options->n_threads = 1;
	user_options->out_file_name = NULL;
	user_options->in_files_names = NULL;
	user_options->n_files = 0;
	user_options->c_flag = false;
	user_options->o_flag = false;
	user_options->t_flag = false;

	opterr = 0; // Ne pas autoriser getopt a ecrire sur stderr (on va faire nos propres messages d'erreur)
	int opt;

	// Utilisation de 'getopt' pour recuperer les options passees en ligne de commande par l'utilisateur
	while ((opt = getopt(argc, argv, "co:t:")) != -1) {
		switch (opt) {
			case 'c':
				if (!user_options->c_flag) {
					user_options->c_flag = true;
				} else {
					// L'option "-c" a ete donnee plusieurs fois
					fprintf(stderr, "Option '-c' used more than once.\n");
					display_usage(stderr);
					free(user_options->out_file_name);
					return -1;
				}
				break;
			case 'o':
				if (!user_options->o_flag) {
					user_options->o_flag = true;
					user_options->out_file_name = (char *) malloc_retry(10, 10,
							sizeof(char) * (1 + strlen(optarg))); // Ne pas oublier le '\0'!
					if (!(user_options->out_file_name)) {
						fprintf(stderr,
								"Failed to allocate memory for 'user_options->filename'"
										" in function 'parsing.c/parse_options'.\n");
						return -1;
					}

					strcpy(user_options->out_file_name, optarg);
				} else {
					// L'option "-o" a ete donnee plusieurs fois
					fprintf(stderr, "Option '-o' used more than once.\n");
					display_usage(stderr);
					free(user_options->out_file_name);
					return -1;
				}
				break;
			case 't':
				if (!user_options->t_flag) {
					user_options->t_flag = true;
					int arg_value = atoi(optarg);
					if (arg_value <= 0) {
						// On aura 0 quand l'utilisateur a demande 0 threads de calcul ou quand 'atoi' a echoue son execution
						// (ce qui arrive par exemple si la valeur entree ne correspond pas a un chiffre)
						fprintf(stderr,
								"'%s' is not a valid value for option '-t'. The value must be a positive integer.\n",
								optarg);
						display_usage(stderr);
						free(user_options->out_file_name);
						return -1;
					}
					user_options->n_threads = (unsigned int) arg_value;
				} else {
					// L'option "-t" a ete utilisee plusieurs fois
					fprintf(stderr, "Option '-t' used more than once.\n");
					display_usage(stderr);
					free(user_options->out_file_name);
					return -1;
				}
				break;
			case '?':
				if (optopt == 't' || optopt == 'o') {
					fprintf(stderr, "Option '-%c' requires a value.\n", optopt);
				} else if (isprint(optopt)) {
					fprintf(stderr, "Unknown option '-%c'.\n", optopt);
				} else {
					fprintf(stderr, "Unknown option character '\\x%x'.\n",
							optopt);
				}
				display_usage(stderr);
				free(user_options->out_file_name);
				return -1;
				break;
			default:
				fprintf(stderr,
						"Unknown error while parsing options in function 'parsing.c/parse_options'.\n");
				return -1;
				break;
		}
	}

	user_options->n_files = argc - optind;
	if (user_options->n_files == 0) {
		fprintf(stderr, "At least one input file must be provided.\n");
		display_usage(stderr);
		free(user_options->out_file_name);
		return -1;
	}

	// Allocation de la memoire pour les chemins vers les fichiers contenant les hashes
	user_options->in_files_names = (char **) malloc_retry(10, 10,
			sizeof(char *) * (user_options->n_files));
	if (!(user_options->in_files_names)) {
		free(user_options->out_file_name);
		fprintf(stderr,
				"Failed to allocate memory for 'user_options->in_files_names'"
						" in function 'parsing.c/parse_options'.\n");
		return -1;
	}

	int errcode;  // Variable utilisee pour gerer les codes d'erreur ci-dessous
	char *filename;

	// Recuperation des noms des fichiers contenant les hashes
	for (int i = optind; i < argc; i++) {
		filename = argv[i];
		// Verifier que le fichier existe
		errcode = file_exists(filename);
		if (errcode == -1) {
			fprintf(stderr,
					"Failed to check for file '%s' in function 'parsing.c/parse_options' (errno=%d : %s).\n",
					filename, errno, strerror(errno));
			free(user_options->out_file_name);
			free(user_options->in_files_names);
			return -1;
		} else if (errcode == 1) {
			free(user_options->out_file_name);
			free(user_options->in_files_names);
			fprintf(stderr, "File '%s' could not be found.\n", filename);
			return -1;
		}
		user_options->in_files_names[i - optind] = (char *) malloc_retry(10, 10,
				sizeof(char *) * (1 + strlen(filename)));
		if (!((user_options->in_files_names)[i - optind])) {
			free(user_options->out_file_name);
			free(user_options->in_files_names);
			fprintf(stderr,
					"Failed to allocate memory for '(user_options-> in_files_names)[%d] in "
							"function 'parsing.c/parse_options'.\n",
					i - optind);
			return -1;
		}
		strcpy(user_options->in_files_names[i - optind], filename);
	}

	return 0;
}
