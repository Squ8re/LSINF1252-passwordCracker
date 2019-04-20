// Fichier regroupant divers utilitaires

#ifndef UTILITIES_HEADER
#define UTILITIES_HEADER

#include <stdbool.h>
#include <stdio.h>


bool file_exists(char *filename);

void display_usage(const FILE *out);

void *malloc_retry(int n_tries, int delay_ms, size_t size);


#endif
