// Fichier qui regroupe diverses fonctions utilitaires

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "utilities.h"
#include "custom_sleep.h"


/*
 * @pre filename != NULL
 * @post retourner 'true' si le fichier existe, 'false' sinon
 */
bool file_exists(char *filename){
	int file = open(filename, O_RDONLY);
	if(file == -1){
		return false;
	}
	close(file);
	return true;
 }


// Fonction qui permet d'afficher sur la sortie 'out' un message indiquant la bonne utilisation du programme
void display_usage(const FILE *out){
	fprintf(stderr, "Usage: ./cracker [-t NTHREADS] [-c] [-o FICHIEROUT] FICHIER1 [FICHIER2 ... FICHIERN]\n");
}


// Fonction qui tente d'effectuer un malloc un maximum de 'n_tries' fois et en espacant ses essais de 'delay_ms' millisecondes
// La fonction s'utilise comme un malloc classique, a l'exception des deux arguments supplementaires
// TODO: Remplacer le "int" par un "unsigned int"? Et ajouter des tests?
void *malloc_retry(int n_tries, int delay_ms, size_t size){
	void *ptr = NULL;
	for(int i = 0; i < n_tries && !ptr; i++){
		ptr = malloc(size);
		custom_sleep(delay_ms);
	}
	return ptr;
}
