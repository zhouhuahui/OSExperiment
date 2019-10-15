如何编译：
gcc -c HashFile.c jtRecord.c
gcc -o main HashFile.o jtRecord.o

gcc -c HashFile.c main2.c `pkg-config --libs --cflags gtk+-2.0`
gcc -o main2 HashFile.c main2.c `pkg-config --libs --cflags gtk+-2.0`
