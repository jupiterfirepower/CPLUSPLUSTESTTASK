recursivefree_shared_mutex : main.o
	g++ -lm -pthread -O3 -o recursivefree_shared_mutex main.o
		 

gcc = g++ -std=c++11 -pthread -O3 -c

main.o : main.cpp
	$(gcc) main.cpp

clean : 
	rm main.o recursivefree_shared_mutex
