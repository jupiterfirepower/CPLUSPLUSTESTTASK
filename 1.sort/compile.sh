g++ randomgen.cpp -pipe -O2 -Wall -m64 -Ofast -flto -march=native -funroll-loops -std=c++1z -I . -o randomgen
g++ bigfilesort.cpp -pipe -O2 -Wall -m64 -Ofast -flto -march=native -funroll-loops -std=c++1z -I . -o bigfilesort

