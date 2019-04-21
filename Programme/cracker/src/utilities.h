// Fichier regroupant divers utilitaires

#ifndef UTILITIES_HEADER
#define UTILITIES_HEADER

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>


bool file_exists(const char *filename);

void display_usage(FILE *restrict out);

void *malloc_retry(int n_tries, int delay_ms, size_t size);

void check_malloc(const void *ptr, char *msg);

void handle_error(char *msg);

void print_hash(FILE *restrict out, uint8_t *hash, unsigned int hash_length);


#endif
