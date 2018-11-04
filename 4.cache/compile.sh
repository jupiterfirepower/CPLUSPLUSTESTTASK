g++ dbcache.cpp -pipe -O2 -Wall -m64 -Ofast -flto -march=native -funroll-loops -std=c++1z -lcassandra -pthread -o dbcache
