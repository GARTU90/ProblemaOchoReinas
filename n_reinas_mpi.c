#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>

// Función que calcula conflictos de una reina contra todas
int conflictos_reina(int idx, int *x, int *y, int n) {
    int cont = 0;
    int a = x[idx], b = y[idx];
    for (int j = 0; j < n; j++) {
        if (j == idx) continue;
        int dx = abs(a - x[j]);
        int dy = abs(b - y[j]);
        if (dx == dy) cont++; // conflicto diagonal
    }
    return cont;
}

// Generar solución inicial
void get_initial_solution(int *x, int *y, int n) {
    for (int i = 0; i < n; i++) {
        x[i] = i;
        y[i] = i;
    }
    // Mezclar
    for (int i = n-1; i > 0; i--) {
        int j = rand() % (i+1);
        int tmp = x[i]; x[i] = x[j]; x[j] = tmp;
    }
    for (int i = n-1; i > 0; i--) {
        int j = rand() % (i+1);
        int tmp = y[i]; y[i] = y[j]; y[j] = tmp;
    }
}

// Función objetivo paralelizada con MPI
int f_parallel(int *x, int *y, int n, int miproc, int numproc) {
    MPI_Status status;
    int total_conflictos = 0;

    if (miproc == 0) { // maestro
        // enviar datos a cada esclavo
        for (int i = 1; i < numproc; i++) {
            int idx = i-1;
            MPI_Send(&idx, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(x, n, MPI_INT, i, 1, MPI_COMM_WORLD);
            MPI_Send(y, n, MPI_INT, i, 2, MPI_COMM_WORLD);
        }
        // recibir resultados
        for (int i = 1; i < numproc; i++) {
            int parcial;
            MPI_Recv(&parcial, 1, MPI_INT, i, 3, MPI_COMM_WORLD, &status);
            total_conflictos += parcial;
        }
    } else { // esclavos
        int idx;
        MPI_Recv(&idx, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(x, n, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(y, n, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
        int parcial = conflictos_reina(idx, x, y, n);
        MPI_Send(&parcial, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
    }

    // maestro devuelve el resultado
    if (miproc == 0) return total_conflictos;
    else return -1; // esclavos no usan este valor
}

// Simulated Annealing (solo maestro)
void simulated_annealing(double t_0, double t_f, int n, int miproc, int numproc) {
    if (miproc != 0) return; // solo maestro ejecuta SA

    int *x = malloc(n * sizeof(int));
    int *y = malloc(n * sizeof(int));
    get_initial_solution(x, y, n);
    int fx = f_parallel(x, y, n, miproc, numproc);

    int *xbest = malloc(n * sizeof(int));
    int *ybest = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) { xbest[i] = x[i]; ybest[i] = y[i]; }
    int fbest = fx;

    double t = t_0;
    int *x_new = malloc(n * sizeof(int));
    int *y_new = malloc(n * sizeof(int));

    while (t > t_f) {
        // generar vecino
        for (int i = 0; i < n; i++) { x_new[i] = x[i]; y_new[i] = y[i]; }
        if ((double)rand()/RAND_MAX < 0.5) {
            int i = rand() % n, j = rand() % n;
            while (j == i) j = rand() % n;
            int tmp = x_new[i]; x_new[i] = x_new[j]; x_new[j] = tmp;
        } else {
            int i = rand() % n, j = rand() % n;
            while (j == i) j = rand() % n;
            int tmp = y_new[i]; y_new[i] = y_new[j]; y_new[j] = tmp;
        }

        int fy = f_parallel(x_new, y_new, n, miproc, numproc);

        if (fy <= fbest) {
            for (int i = 0; i < n; i++) { xbest[i] = x_new[i]; ybest[i] = y_new[i]; }
            fbest = fy;
            if (fbest == 0) break;
        }

        if (fy <= fx || ((double)rand()/RAND_MAX) < exp(-(fy - fx)/t)) {
            for (int i = 0; i < n; i++) { x[i] = x_new[i]; y[i] = y_new[i]; }
            fx = fy;
        }
        t *= 0.99;
    }

    printf("Solucion final:\n");
    printf("x: ");
    for (int i = 0; i < n; i++) printf("%d ", xbest[i]);
    printf("\ny: ");
    for (int i = 0; i < n; i++) printf("%d ", ybest[i]);
    printf("\ncon f(x,y) = %d\n", fbest);

    free(x); free(y);
    free(xbest); free(ybest);
    free(x_new); free(y_new);
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int miproc, numproc;
    MPI_Comm_rank(MPI_COMM_WORLD, &miproc);
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);

    srand(time(NULL) + miproc); // semilla distinta por proceso

    if (argc < 4) {
        if (miproc == 0) printf("Uso: %s n t_inicial t_final\n", argv[0]);
        MPI_Finalize();
        return 1;
    }
    int n = atoi(argv[1]);
    double t_inicial = atof(argv[2]);
    double t_final = atof(argv[3]);

    simulated_annealing(t_inicial, t_final, n, miproc, numproc);

    MPI_Finalize();
    return 0;
}
