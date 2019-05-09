cracker : main.o custom_sleep.o init.o linked_list.o parsing.o reader_thread.o\
		 reverse_thread.o reverse.o sha256.o sort_thread.o utilities.o
		gcc -o cracker -Wall -Werror -pthread .\src\*.o

main.o : main.c main.h									#parsing.h utilities.h init.h reader_thread.h reverse_thread.h sort_thread.h
		gcc -c -Wall -Werror -pthread .\src\main.c
custom_sleep.o : custom_sleep.c custom_sleep.h 			#utilities.h
		gcc -c -Wall -Werror .\src\custom_sleep.c
init.o : init.c init.h parsing.h linked_list.h u 		#utilities.h parsing.h init.h
		gcc -c -Wall -Werror -pthread .\src\init.c
linked_list.o : linked_list.c linked_list.h				#utilities.h
		gcc -c -Wall -Werror .\src\linked_list.c
parsing.o : parsing.c parsing.h							#utilities.h
		gcc -c -Wall -Werror -pthread .\src\parsing.c
reader_thread.o : reader_thread.c reader_thread.h		#utilities.h init.h
		gcc -c -Wall -Werror -pthread .\src\reader_thread.c
reverse_thread.o : reverse_thread.c reverse_thread.h	#utilities.h init.h
		gcc -c -Wall -Werror -pthread .\src\reverse_thread.c
reverse.o : reverse.c reverse.h							#sha256.h
		gcc -c -Wall -Werror .\src\reverse.c
sha256.o : sha256.c sha256.h
		gcc -c -Wall -Werror .\src\sha256.c
sort_thread.o : sort_thread.c sort_thread.h				#utilities.h init.h linked_list.h parsing.h
		gcc -c -Wall -Werror -pthread .\src\sort_thread.c
utilities.o : utilities.c utilities.h					#custom_sleep.h parsing.h linked_list.h
		gcc -c -Wall -Werror -pthread .\src\utilities.c

clean :
		rm cracker main.o custom_sleep.o init.o linked_list.o parsing.o reader_thread.o \
			reverse_thread.o reverse.o sha256.o sort_thread.o utilities.o