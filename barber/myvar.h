// myvar.h - Estructura de datos para comunicacion MPI (reinas)
// Adaptado del barbero de juan.daniel.rangel.avila@gmail.com

#ifndef MYVAR_H
#define MYVAR_H

#define MAX_N 512

typedef struct {
    int x[MAX_N];   // coordenadas x de todas las reinas
    int y[MAX_N];   // coordenadas y de todas las reinas
    int n;          // numero total de reinas
    int queen_idx;  // indice de la reina que este esclavo debe evaluar
    int F;          // resultado parcial: colisiones diagonales de queen_idx
} MPI_myvar;

#endif
