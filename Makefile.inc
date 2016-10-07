CC = gcc
CFLAGS = -O
LD = gfortran
LDFLAGS = -O -llapack -lblas -Wl,-zmuldefs -static

AR = ar ruv
MAKE = make
