/*
 * reverse_thread.h
 *
 *  Created on: 24 avr. 2019
 *      Author: Amadeo DAVID
 *      NOMA  : 4476 1700
 */

#ifndef SRC_REVERSE_THREAD_H_
#define SRC_REVERSE_THREAD_H_

#include "init.h"


int get_hash(shared_data_t *shared, uint8_t *return_hash);

void *reverse(void *shared);

#endif /* SRC_REVERSE_THREAD_H_ */
