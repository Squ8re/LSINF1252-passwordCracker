/*
 * sort.h
 *
 *  Created on: 2 mai 2019
 *      Author: amade
 */

#ifndef SRC_SORT_THREAD_H_
#define SRC_SORT_THREAD_H_

void * GetPassword(shared_data_t *sharedData, void *returnString);
void * CountVowels(char Password[]);
void * CountConsonants(char Password[]);
void * SortPasswords(shared_data_t * sharedData);

#endif /* SRC_SORT_THREAD_H_ */
