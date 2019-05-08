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

#include "utilities.h"
#include "init.h" 														// Utilisation de shared_data
#include "linked_list.h" 												// Utilisation des listes chaines
#include "parsing.h"													// Utilisation des options utilisateurs
/*
 * Ce fichier contient toutes les fonctions relatives aux threads de tri.
 *
 *  Created on: 14 avr. 2019
 *      Author: amade
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
void * GetPassword(shared_data_t *sharedData, void *returnString){
	int firstFullIndex;				// Premier indice rempli.
	int errcode;					// Gestion des codes erreurs.

	// Attente d'un slot rempli
	if(sem_wait(sharedData->reversed_full) == -1){
		fprintf(stderr,
				"Failed to access semaphore 'sharedData->reversed_full' in function 'sort_thread.c/GetPassword.\n");
		return ((void*) -1);
	}

	// Blocage du buffer
	if((errcode=pthread_mutex_lock(sharedData->reversed_buffer_mtx)) != 0){
		fprintf(stderr,
				"Failed to lock mutex 'sharedData->reversed_buffer_mtx' in function 'sort_thread.c/GetPassword.\n");
		errno = errcode;
		return ((void*) -1);
	}

	// Recuperation de l'indice du premier slot rempli dans le buffer
	if(sem_getvalue(sharedData->reversed_full, &firstFullIndex) == -1){
		fprintf(stderr,
				"Failed to retrieve value from semaphore 'sharedData->reversed_full' in function"
						" 'sort_thread.c/GetPassword'.\n");
		return((void*) -1);
	}

	// Copie de la valeur a recuperer et free du slot buffer
	memcpy(returnString,
			(sharedData->reversed_buffer)[firstFullIndex],
			sharedData);
	free((sharedData->hashes_buffer)[firstFullIndex]);

	// Liberation du thread
	if((errcode = pthread_mutex_unlock(sharedData->reversed_buffer_mtx))){
		fprintf(stderr,
				"Failed to unlock mutex 'sharedData->reversed_buffer_mtx' in function "
					"'sort_thread.c/GetPassword'.\n");
		errno = errcode;
		return ((void*) -1);
	}

	// Indication du slot libre
	if(sem_post(sharedData->reversed_empty) == -1){
		fprintf(stderr,
				"Failed to access semaphore 'sharedData->reverse_empty' in function "
					"'sort_thread.c/GetPassword'.\n");
		return ((void*) -1);
	}
	// Fin de la fonction.
	return((void*) 0);
}

/*
 * Cette fonction compte le nombre de voyelles de l'argument.
 * @pre		L'argument contient un tableau de charactere.
 * @post	L'argument n'est pas modifie.
 * @return	La fonction retourne un pointeur vers un 'void*'.
 * 			La fonction retourne le nombre de voyelles apparaissant dans le mot.
 */
void * CountVowels(char Password[]){
	//TODO: Verifier la condition de la boucle while
	int i = 0;
	int count = 0;
	while(Password[i] != '\0'){
		if(Password[i] == 'a' || Password[i] ==  'A' ||
				Password[i] == 'e' || Password[i] == 'E' ||
				Password[i] == 'i' || Password[i] == 'I' ||
				Password[i] == 'o' || Password[i] == 'O' ||
				Password[i] == 'u' || Password[i] == 'U'){
			count++;
		}
		i++;
	}
	return ((void *) count);
}

/*
 * Cette fonction compte le nombre de consonnes de l'argument.
 * @pre		L'argument contient un tableau de charactere.
 * @post	L'argument n'est pas modifie.
 * @return	La fonction retourne un pointeur vers un 'void*'.
 * 			La fonction retourne le nombre de consonnes apparaissant dans le mot.
 */
void * CountConsonants(char Password[]){
	//TODO: Verifier la condition de la boucle while et du if
		int i = 0;
		int count = 0;
		while(Password[i] != '\0'){
			if(!(Password[i] == 'a' || Password[i] ==  'A' ||
					Password[i] == 'e' || Password[i] == 'E' ||
					Password[i] == 'i' || Password[i] == 'I' ||
					Password[i] == 'o' || Password[i] == 'O' ||
					Password[i] == 'u' || Password[i] == 'U')){
				count++;
			}
			i++;
		}
		return ((void *) count);
}
/**
 * Cette fonction recupere les mots de passes dechiffres et gere la liste des meilleurs candidats.
 * Une fois imprime les candidats sur la sortie choisie par 'sharedData->user_options_o_flag'.
 * @pre		La structure sharedData est correctement initialisee.
 * @post	Les elements tries sont retires des buffers correspondants
 * @return	La fonction retourne un pointeur vers un 'void *'.
 * 			La fonction retourne 0 si elle termine correctement, -1 sinon.
 */
void * SortPasswords(shared_data_t * sharedData){
	linked_list_t candidates = (linked_list_t)(malloc(sizeof(linked_list_t)));  // liste des meilleurs candidats.
	int maxNumber = 0; 								// nombre max de voyelle ou consonne deja trouve.
	int quality; 									// nombre de voyelle ou consonne de l'element analyse.
	char toCompare[]; 								// element analyse.
	// TODO: /!\ TROUVER LA CONDITION DE FIN DE BOUCLE
	while(true){
		// Recuperation d'un mot de passe
		if(getPassword(sharedData,&toCompare) == -1){
			fprintf(stderr,
					"Failed to retrieve password in function 'sort_thread.c/SortPasswords.\n");
			return ((void*) -1);
		}
		//TODO: c_flag introuvable (mis dans parsing, est-ce normal?)
		// recuperation du nombre de lettre a optimiser
		if(sharedData->user_options->c_flag){	// Si ce sont des consonnes...
			quality = CountConsonants(toCompare);
		}else{									// Si ce sont des voyelles...
			quality = CountVowels(sharedData, &toCompare);
		}

		// Tri des mots de passe
		if(quality> maxNumber){					// S'il est de meilleur qualite que les autres...
			remove_all(candidates);
			add_node(candidates, &toCompare);
			maxNumber = quality;
		}else if(quality == maxNumber){			// S'il est de meme qualite que les autres...
			add_node(candidates, &toCompare);
		}
	}

	//Une fois le tri termine
	if(sharedData->user_options->o_flag){		// Si ecriture dans un fichier...
		node_t *traveller = candidates->head;
		int i;
		//TODO: Quelles permissions pour les users?
		//TODO: out_file_name introuvable (mis dans parsing, est-ce normal?)
		// Ouverture du fichier
		int fr = open(sharedData->user_options->out_file_name, O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU);
		if(fr == -1){
			fprintf(stderr,
					"Failed to open outfile in function 'sort_thread.c/SortPasswords'.\n");
			return ((void*) -1);
		}
		// TODO: voir pour le \n en fin de password
		// Ecriture du fichier
		while(i<=candidates->length){
			if(write(fr,traveller->contents,(strlen(traveller->contents)+1)*sizeof(char)) != (strlen(traveller->contents)+1)*sizeof(char)){
				fprintf(stderr,
						"Failed to write in outfile in function 'sort_thread.c/SortPasswords'.\n");
				close(fr);
				return ((void*) -1);
			}
		}

		// Fermeture du fichier
		if(close(fr) != 0){
			fprintf(stderr,
					"Failed to close outfile in function 'sort_thread.c/SortPasswords'.\n");
			return ((void*) -1);
		}
	}else{										// Si ecriture sur la sortie standart...
		node_t *traveller = candidates->head;
		int i;
		printf("Here are the best candidates of the password list.\n");
		printf("There is %d best candidates.\n",candidates->length);
		while(i<=candidates->length){
			printf("Candidate %04d : %s", traveller->contents); //TODO: Test le printf
			traveller = traveller->next;
		}
	}
}
