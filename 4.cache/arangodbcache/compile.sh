g++ dbcache.cpp -pipe -O2 -Wall -m64 -Ofast -flto -march=native -funroll-loops -std=c++1z -pthread -I ./fuerte/include -L ./fuerte/.build-dev -lfuerte -lcurl -lboost_system -lboost_thread -lssl -lcrypto -lvelocypack -o dbcache