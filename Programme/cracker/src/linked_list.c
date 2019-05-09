// Fichier contenant du code pour la gestion d'une liste chainee

/*
 *
 * Author: Eduardo Vannini (NOMA: 10301700)
 *          Date: 09-05-2019
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utilities.h"
#include "linked_list.h"

/*
 * Fonction permettant d'ajouter une node contenant la valeur 'contents' a la fin d'une liste chainee.
 *
 * @pre 'list' est non-NULL.
 * @post Si la liste chainee etait vide au depart, elle contiendra desormais un seul element qui est la node
 *       que l'on a ajoutee. Si la liste chainee n'etait pas vide, alors la nouvelle node sera proprement
 *       ajoutee a la fin de cette liste chainee. En cas d'erreur, la fonction affiche un message d'erreur sur
 *       stderr et retourne -1. Sinon, elle retourne 0.
 *
 */
int add_node(linked_list_t *list, char *contents) {
	// Allocation de la memoire pour la nouvelle node
	node_t *new_node = (node_t *) malloc_retry(10, 10, sizeof(node_t));
	if (!new_node) {
		fprintf(stderr,
				"Failed to allocate memory for 'new_node' in function 'linked_list.c/add_node'.\n");
		return -1;
	}

	// Allocation de la memoire pour le contenu de la nouvelle node
	new_node->next = NULL;
	new_node->contents = (char *) malloc_retry(10, 10,
			sizeof(char) * (1 + strlen(contents)));
	if (!(new_node->contents)) {
		free(new_node);
		fprintf(stderr,
				"Failed to allocate memory for 'new_node->contents' in function 'linked_list.c/add_node'.\n");
		return -1;
	}
	strcpy(new_node->contents, contents);

	// Ajout de la nouvelle node a la fin de la liste chainee
	if (!(list->head)) {
		list->head = new_node;
		list->tail = new_node;
		list->length = 1;
	} else {
		list->tail->next = new_node;
		list->tail = new_node;
		list->length++;
	}

	return 0;
}

/*
 * Fonction permettant de retirer toutes les nodes d'une liste chainee.
 *
 * @pre 'list' est non-NULL.
 * @post 'list' pointe vers une liste chainee vide.
 *
 */
void remove_all(linked_list_t *list) {
	node_t *current_node = list->head;
	while (current_node) {
		current_node = current_node->next;
		free(list->head);
		list->head = current_node;
	}
	list->tail = NULL;
	list->length=0;
}

/*
 * Fonction permettant d'initialiser une linked list vide.
 *
 * @pre 'list' est non-NULL et est non-initialisee.
 * @post 'list' est initialisee (sa longueur est mise a 0 et ses pointeurs 'head' et 'tail' sont NULL)
 *
 */
void init_linked_list(linked_list_t *list){
	list->length = 0;
	list->head = NULL;
	list->tail = NULL;
}
