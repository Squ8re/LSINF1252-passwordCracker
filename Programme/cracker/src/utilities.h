// Fichier regroupant divers utilitaires

#ifndef UTILITIES_HEADER
#define UTILITIES_HEADER

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "parsing.h"
#include "init.h"

int file_exists(const char *filename);

void display_usage(FILE *restrict out);

void *malloc_retry(unsigned int n_tries, unsigned int delay_ms, size_t size);

void print_hash(FILE *restrict out, uint8_t *hash, unsigned int hash_length);

void display_options(options_t const *user_options);

void free_all(shared_data_t *shared);

#endif
