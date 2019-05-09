all: cracker

cracker: custom_sleep.o main.o init.o linked_list.o parsing.o reader_thread.o reverse_thread.o reverse.o sha256.o sort_thread.o utilities.o
	gcc -o cracker -Wall -Werror -pthread custom_sleep.o
	gcc -o cracker -Wall -Werror -pthread main.o
	gcc -o cracker -Wall -Werror -pthread init.o
	gcc -o cracker -Wall -Werror -pthread linked_list.o
	gcc -o cracker -Wall -Werror -pthread parsing.o
	gcc -o cracker -Wall -Werror -pthread reader_thread.o
	gcc -o cracker -Wall -Werror -pthread reverse_thread.o
	gcc -o cracker -Wall -Werror -pthread reverse.o
	gcc -o cracker -Wall -Werror -pthread sha256.o
	gcc -o cracker -Wall -Werror -pthread sort_thread.o
	gcc -o cracker -Wall -Werror -pthread utilities.o

custom_sleep.o: ./src/custom_sleep.c ./src/custom_sleep.h ./src/utilities.h
	gcc -c -Wall -Werror ./src/custom_sleep.c
main.o: ./src/main.c ./src/parsing.h ./src/utilities.h ./src/init.h ./src/reader_thread.h ./src/reverse_thread.h ./src/sort_thread.h
	gcc -c -Wall -Werror -pthread ./src/main.c
init.o: ./src/init.c ./src/init.h ./src/parsing.h ./src/linked_list.h ./src/utilities.h ./src/parsing.h ./src/init.h
	gcc -c -Wall -Werror -pthread ./src/init.c
linked_list.o: ./src/linked_list.c ./src/linked_list.h ./src/utilities.h
	gcc -c -Wall -Werror ./src/linked_list.c
parsing.o: ./src/parsing.c ./src/parsing.h ./src/utilities.h
	gcc -c -Wall -Werror -pthread ./src/parsing.c
reader_thread.o: ./src/reader_thread.c ./src/reader_thread.h ./src/utilities.h ./src/init.h
	gcc -c -Wall -Werror -pthread ./src/reader_thread.c
reverse_thread.o: ./src/reverse_thread.c ./src/reverse_thread.h ./src/utilities.h ./src/init.h
	gcc -c -Wall -Werror -pthread ./src/reverse_thread.c
reverse.o: ./src/reverse.c ./src/reverse.h ./src/sha256.h
	gcc -c -Wall -Werror ./src/reverse.c
sha256.o: ./src/sha256.c ./src/sha256.h
	gcc -c -Wall -Werror ./src/sha256.c
sort_thread.o: ./src/sort_thread.c ./src/sort_thread.h ./src/utilities.h ./src/init.h ./src/linked_list.h ./src/parsing.h
	gcc -c -Wall -Werror -pthread ./src/sort_thread.c
utilities.o: ./src/utilities.c ./src/utilities.h ./src/custom_sleep.h ./src/parsing.h ./src/linked_list.h
	gcc -c -Wall -Werror -pthread ./src/utilities.c

clean:
	rm cracker main.o custom_sleep.o init.o linked_list.o parsing.o reader_thread.o reverse_thread.o reverse.o sha256.o sort_thread.o utilities.o
