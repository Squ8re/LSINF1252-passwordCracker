// Fichier contenant du code pour la gestion d'une liste chainee

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utilities.h"
#include "linked_list.h"


void add_node(linked_list_t *list, char *contents){
	// Creer et remplir la nouvelle node
	node_t *new_node = (node_t *) malloc_retry(10, 10, sizeof(node_t));
	check_malloc((void *) new_node, "Failed to allocate memory for 'new_node' in function 'linked_list.c/add_node'");

	new_node->next = NULL;
	new_node->contents = (char *) malloc_retry(10, 10, sizeof(char) * (1 + strlen(contents)));
	check_malloc((void *) new_node->contents, "Failed to allocate memory for 'new_node->contents' in function"
			                                  " 'linked_list.c/add_node'");
	strcpy(new_node->contents, contents);

	// Ajouter la node a la fin de la liste
	if(!(list->head)){
		list->head = new_node;
		list->tail = new_node;
	}
	else{
		list->tail->next = new_node;
		list->tail = new_node;
	}
}

void remove_all(linked_list_t *list){
	node_t *current_node = list->head;
	while(current_node){
		current_node = current_node->next;
		free(list->head);
		list->head = current_node;
	}
	list->tail = NULL;
}
