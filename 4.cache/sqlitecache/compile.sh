g++ dbcache.cpp -pipe -O2 -Wall -m64 -Ofast -flto -march=native -funroll-loops -std=c++1z -pthread -I ./driver/SQLiteCpp/include -L ./driver/Lib -L/usr/lib/x86_64-linux-gnu -lsqlite3 -lSQLiteCpp -ldl -o dbcache