g++ main.cpp shared_mutex.cpp -pipe -O2 -Wall -m64 -Ofast -flto -march=native -funroll-loops -std=c++11 -pthread -o recsharedmutex
