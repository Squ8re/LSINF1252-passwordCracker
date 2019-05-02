#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#include "utilities.h"
#include "init.h" 														// Utilisation de shared_data
/*
 * Ce fichier contient toutes les fonctions relatives aux threads de tri.
 *
 *  Created on: 14 avr. 2019
 *      Author: amade
 */

/*
 * Cette fonction prends un element de reverse.
 */
