#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "utilities.h"
#include "init.h" 														// Utilisation de shared_data
#include "linked_list.h" 												// Utilisation des listes chaines
#include "parsing.h"													// Utilisation des options utilisateurs
/*
 * Ce fichier contient toutes les fonctions relatives aux threads de tri.
 *
 *  Created on: 14 avr. 2019
 *      Author: Amadeo DAVID
 *      Noma  : 4476 1700
 */

/*
 * Cette fonction prends un element a trier.
 * @pre		'sharedData' est correctement initialise
 * 			'return' contient l'espace memoire necessaire pour un String.
*  @post	return contient un element a trier.
*  			L'element a trier est retire du buffer.
*  @return	La fonction retourne un pointeur vers un 'void*'.
*  			La fonction retourne 0 (sous forme de 'void*') si tout s'est bien deroule, -1 sinon.
 */
int get_password(shared_data_t *shared, void *return_string){
	int first_full_index;				// Premier indice rempli.
	int errcode;					// Gestion des codes erreurs.

	// Attente d'un slot rempli
	if(sem_wait(shared->reversed_full) == -1){
		fprintf(stderr,
				"Failed to access semaphore 'shared->reversed_full' in function 'sort_thread.c/get_password.\n");
		return -1;
	}

	// Blocage du buffer
	if((errcode=pthread_mutex_lock(shared->reversed_buffer_mtx)) != 0){
		fprintf(stderr,
				"Failed to lock mutex 'shared->reversed_buffer_mtx' in function 'sort_thread.c/get_password.\n");
		errno = errcode;
		return -1;
	}

	// Recuperation de l'indice du premier slot rempli dans le buffer
	if(sem_getvalue(shared->reversed_full, &first_full_index) == -1){
		fprintf(stderr,
				"Failed to retrieve value from semaphore 'shared->reversed_full' in function"
						" 'sort_thread.c/get_password'.\n");
		return -1;
	}
	first_full_index--;

	// Copie de la valeur a recuperer et free du slot buffer
	strcpy(return_string,
			(shared->reversed_buffer)[first_full_index]);
	free((shared->hashes_buffer)[first_full_index]);

	// Liberation du thread
	if((errcode = pthread_mutex_unlock(shared->reversed_buffer_mtx))){
		fprintf(stderr,
				"Failed to unlock mutex 'shared->reversed_buffer_mtx' in function "
					"'sort_thread.c/get_password'.\n");
		errno = errcode;
		return -1;
	}

	// Indication du slot libre
	if(sem_post(shared->reversed_empty) == -1){
		fprintf(stderr,
				"Failed to access semaphore 'shared->reverse_empty' in function "
					"'sort_thread.c/get_password'.\n");
		return -1;
	}
	// Fin de la fonction.
	return 0;
}

/*
 * Cette fonction compte le nombre de voyelles de l'argument.
 * @pre		L'argument contient un tableau de charactere.
 * @post	L'argument n'est pas modifie.
 * @return	La fonction retourne un pointeur vers un 'void*'.
 * 			La fonction retourne le nombre de voyelles apparaissant dans le mot.
 */
int count_vowels(char *password){
	int i;
	int count = 0;
	for(i=1; i<strlen(password); i++){
		if(password[i] == 'a' || password[i] ==  'A' ||
				password[i] == 'e' || password[i] == 'E' ||
				password[i] == 'i' || password[i] == 'I' ||
				password[i] == 'o' || password[i] == 'O' ||
				password[i] == 'u' || password[i] == 'U'){
			count++;
		}
	}
	return count;
}

/*
 * Cette fonction compte le nombre de consonnes de l'argument.
 * @pre		L'argument contient un tableau de charactere.
 * @post	L'argument n'est pas modifie.
 * @return	La fonction retourne un pointeur vers un 'void*'.
 * 			La fonction retourne le nombre de consonnes apparaissant dans le mot.
 */
int count_consonants(char *password){
	int i;
	int count = 0;
	for(i=1; i<strlen(password); i++){
		if(!(password[i] == 'a' || password[i] ==  'A' ||
				password[i] == 'e' || password[i] == 'E' ||
				password[i] == 'i' || password[i] == 'I' ||
				password[i] == 'o' || password[i] == 'O' ||
				password[i] == 'u' || password[i] == 'U')){
			count++;
		}
	}
	return count;
}
/**
 * Cette fonction recupere les mots de passes dechiffres et gere la liste des meilleurs candidats.
 * @pre		La structure sharedData est correctement initialisee.
 * @post	Les elements tries sont retires des buffers correspondants
 * @return	La fonction retourne un pointeur vers un 'void *'.
 * 			La fonction retourne 0 si elle termine correctement, -1 sinon.
 */
void *sort_passwords(void* sort_params){
	shared_data_t *shared = (shared_data_t *) (sort_params);
	linked_list_t *candidates = (linked_list_t *) (malloc(sizeof(linked_list_t)));  // liste des meilleurs candidats.
	init_linked_list(candidates);
	int max_number = 0; 							// nombre max de voyelle ou consonne deja trouve.
	int quality; 									// nombre de voyelle ou consonne de l'element analyse.
	char *to_compare; 								// element analyse.

	// Tant que tous les fichiers n'ont pas ete reverse, on trie les mots de passe.
	while(shared->all_files_reversed){
		// Recuperation d'un mot de passe
		if(get_password(shared, &to_compare) == -1){
			fprintf(stderr,
					"Failed to retrieve password in function 'sort_thread.c/sort_passwords.\n");
			return ((void*) -1);
		}

		// recuperation du nombre de lettre a optimiser
		if(shared->user_options->c_flag){	// Si ce sont des consonnes...
			quality = count_consonants(to_compare);
		}else{									// Si ce sont des voyelles...
			quality = count_vowels(to_compare);
		}

		// Tri des mots de passe
		if(quality> max_number){					// S'il est de meilleur qualite que les autres...
			remove_all(candidates);
			add_node(candidates, to_compare);
			max_number = quality;
		}else if(quality == max_number){			// S'il est de meme qualite que les autres...
			add_node(candidates, to_compare);
		}
	}

	//Une fois le tri termine
	node_t *traveller = candidates->head;
	if(traveller == NULL){
		// On n'a obtenu aucun candidat
		printf("No available password (is there anything in your input files?).\n");
		return ((void *) 0);
	}

	if(shared->user_options->o_flag){		// Si ecriture dans un fichier...

		// Ouverture du fichier
		int fr = open(shared->user_options->out_file_name, O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU);
		if(fr == -1){
			fprintf(stderr,
					"Failed to open outfile in function 'sort_thread.c/sort_passwords'.\n");
			return ((void*) -1);
		}

		// Ecriture du fichier
		for(int i = 0; i <= (candidates->length); i++){
			if(write(fr, traveller->contents, (strlen(traveller->contents) + 1) * sizeof(char))
					!= (strlen(traveller->contents) + 1) * sizeof(char)){
				fprintf(stderr,
						"Failed to write in outfile in function 'sort_thread.c/sort_passwords'.\n");
				close(fr);
				return ((void*) -1);
			}
			if(write(fr,"\n",sizeof(char)) != sizeof(char)){
				fprintf(stderr,
						"Failed to write in outfile in function 'sort_thread.c/sort_passowrds'.\n");
				close(fr);
				return ((void*) -1);
			}
		}

		// Fermeture du fichier
		if(close(fr) != 0){
			fprintf(stderr,
					"Failed to close outfile in function 'sort_thread.c/sort_passwords'.\n");
			return ((void*) -1);
		}
	}else{										// Si ecriture sur la sortie standart...
		node_t *traveller = candidates->head;
		printf("Here are the best candidates of the password list.\n");
		printf("There is %d best candidates.\n",candidates->length);
		for(int i = 0; i<=candidates->length; i++){
			printf("Candidate %04d : %s", i, traveller->contents);
			traveller = traveller->next;
		}
	}

	return ((void *) 0);
}
