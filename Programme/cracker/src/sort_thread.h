/*
 * sort.h
 *
 *  Created on: 2 mai 2019
 *      Author: Amadeo DAVID
 *      NOMA  : 4476 1700
 */

#ifndef SRC_SORT_THREAD_H_
#define SRC_SORT_THREAD_H_


void **get_password(shared_data_t *shared, void *return_string);

void * count_vowels(char password[]);

void * count_consonants(char password[]);

void * sort_passwords(void *shared);


#endif /* SRC_SORT_THREAD_H_ */
