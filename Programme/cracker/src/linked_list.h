// Fichier contenant du code pour la gestion d'une liste chainee

/*
 *
 * Author: Eduardo Vannini (NOMA: 10301700)
 *          Date: 09-05-2019
 *
 */

#ifndef LINKED_LIST_HEADER
#define LINKED_LIST_HEADER

typedef struct node {
		struct node *next;
		char *contents;
} node_t;

typedef struct linked_list {
		node_t *head;
		node_t *tail;
		unsigned int length;
} linked_list_t;

int add_node(linked_list_t *list, char *contents);

void remove_all(linked_list_t *list);

#endif
