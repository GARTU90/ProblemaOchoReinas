// Reinas con Simulated Annealing distribuido via MPI
// Adaptado del barbero de juan.daniel.rangel.avila@gmail.com
// GNU/GPL License
// 20260513

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include "myvar.h"
#include "Timming.h"

// -------------------------------------------------------
// Funcion objetivo PARCIAL (ejecutada en cada esclavo)
// Cuenta cuantas otras reinas estan en diagonal con la
// reina queen_idx, sin doble conteo (solo j > queen_idx)
// -------------------------------------------------------
int f_parcial(MPI_myvar *range) {
    int i   = range->queen_idx;
    int n   = range->n;
    int ax  = range->x[i];
    int ay  = range->y[i];
    int cont = 0;

    for (int j = i + 1; j < n; j++) {
        int bx = range->x[j];
        int by = range->y[j];
        int dx = abs(ax - bx);
        int dy = abs(ay - by);
        if (dx == dy && dx != 0)   // diagonal
            cont++;
    }
    return cont;
}

// -------------------------------------------------------
// Utilidades del SA (solo en master)
// -------------------------------------------------------
void get_initial_solution(int *x, int *y, int n) {
    for (int i = 0; i < n; i++) { x[i] = i; y[i] = i; }
    for (int i = n-1; i > 0; i--) {
        int j = rand() % (i+1);
        int tmp = x[i]; x[i] = x[j]; x[j] = tmp;
    }
    for (int i = n-1; i > 0; i--) {
        int j = rand() % (i+1);
        int tmp = y[i]; y[i] = y[j]; y[j] = tmp;
    }
}

void get_neighbor(int *x, int *y, int *xn, int *yn, int n) {
    for (int i = 0; i < n; i++) { xn[i] = x[i]; yn[i] = y[i]; }
    if ((double)rand()/RAND_MAX < 0.5) {
        int i = rand() % n, j = rand() % n;
        while (j == i) j = rand() % n;
        int tmp = xn[i]; xn[i] = xn[j]; xn[j] = tmp;
    } else {
        int i = rand() % n, j = rand() % n;
        while (j == i) j = rand() % n;
        int tmp = yn[i]; yn[i] = yn[j]; yn[j] = tmp;
    }
}

// -------------------------------------------------------
// MAIN
// -------------------------------------------------------
int main(int argn, char **argc) {
    int miproc, numproc;
    MPI_Status status;
    MPI_myvar range;

    double utime0, stime0, wtime0,
           utime2, stime2, wtime2;

    int    n;
    double t_inicial, t_final;

    MPI_Init(&argn, &argc);
    MPI_Comm_rank(MPI_COMM_WORLD, &miproc);
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    MPI_Barrier(MPI_COMM_WORLD);

    // --------------------------------------------------
    // MASTER: lectura de parametros
    // --------------------------------------------------
    if (miproc == 0) {
        if (argn < 4) {
            printf("Faltan parametros [n] [t_inicial] [t_final]\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        if (sscanf(argc[1], "%d",  &n)         != 1) { printf("Error al convertir n.\n");         MPI_Abort(MPI_COMM_WORLD,1); return 1; }
        if (sscanf(argc[2], "%lf", &t_inicial) != 1) { printf("Error al convertir t_inicial.\n"); MPI_Abort(MPI_COMM_WORLD,1); return 1; }
        if (sscanf(argc[3], "%lf", &t_final)   != 1) { printf("Error al convertir t_final.\n");   MPI_Abort(MPI_COMM_WORLD,1); return 1; }

        if (n > MAX_N) {
            printf("n=%d supera MAX_N=%d. Ajusta myvar.h\n", n, MAX_N);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }

        printf("[n=%d] t_inicial=%lf t_final=%lf\n", n, t_inicial, t_final);
        uswtime(&utime2, &stime2, &wtime2);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // --------------------------------------------------
    // ESCLAVOS: bucle de servicio (igual que el barbero)
    // --------------------------------------------------
    if (miproc != 0) {
        range.F = 0;
        while (1) {
            // aviso de disponibilidad
            MPI_Send(&range, sizeof(range), MPI_CHARACTER, 0, 0, MPI_COMM_WORLD);
            // recibir trabajo
            MPI_Recv(&range, sizeof(range), MPI_CHARACTER, 0, 0, MPI_COMM_WORLD, &status);
            // evaluar contribucion parcial de la reina queen_idx
            range.F = f_parcial(&range);
        }

    } else {
        // --------------------------------------------------
        // MASTER ORQUESTADOR: SA + distribucion de f()
        // --------------------------------------------------
        srand(time(NULL));

        int *x     = malloc(n * sizeof(int));
        int *y     = malloc(n * sizeof(int));
        int *xbest = malloc(n * sizeof(int));
        int *ybest = malloc(n * sizeof(int));
        int *xn    = malloc(n * sizeof(int));
        int *yn    = malloc(n * sizeof(int));

        get_initial_solution(x, y, n);

        // --- funcion para evaluar f(x,y,n) usando los esclavos ---
        // Envia reina i al esclavo i (round-robin si n > numproc-1)
        // y recoge las contribuciones parciales
        // Se hace bloqueante para garantizar coherencia dentro del SA

        // Evaluacion inicial
        // (usamos una version local rapida para arrancar;
        //  podria distribuirse igual, pero n puede ser mayor que esclavos)
        // Aqui distribuimos: cada esclavo recibe una reina a la vez
        // Usamos el mismo patron no-bloqueante del barbero para f():

        // Macro de evaluacion distribuida de f
        // Retorna la suma total de colisiones diagonales
        #define EVAL_F(sol_x, sol_y, result_f) do {                         \
            int _total = 0;                                                   \
            MPI_Request _req;                                                 \
            MPI_myvar   _r, _res;                                             \
            int _flag, _sent = 0, _recv = 0;                                 \
            _flag = -1;                                                       \
            while (_recv < n) {                                               \
                if (_flag != 0) {                                             \
                    MPI_Irecv(&_res, sizeof(_res), MPI_CHARACTER,            \
                              MPI_ANY_SOURCE, MPI_ANY_TAG,                   \
                              MPI_COMM_WORLD, &_req);                        \
                    _flag = 0;                                                \
                }                                                             \
                MPI_Test(&_req, &_flag, &status);                            \
                if (_flag != 0 && status.MPI_SOURCE != -1) {                 \
                    _total += _res.F;                                         \
                    if (_sent < n) {                                          \
                        for (int _i = 0; _i < n; _i++) {                    \
                            _r.x[_i] = (sol_x)[_i];                         \
                            _r.y[_i] = (sol_y)[_i];                         \
                        }                                                     \
                        _r.n = n;                                             \
                        _r.queen_idx = _sent;                                \
                        _r.F = 0;                                             \
                        MPI_Send(&_r, sizeof(_r), MPI_CHARACTER,             \
                                 status.MPI_SOURCE, 0, MPI_COMM_WORLD);     \
                        _sent++;                                              \
                    }                                                         \
                    _recv++;                                                  \
                    _flag = -1;                                               \
                }                                                             \
            }                                                                 \
            (result_f) = _total;                                              \
        } while(0)

        int fx, fy, fbest;
        EVAL_F(x, y, fx);
        for (int i = 0; i < n; i++) { xbest[i] = x[i]; ybest[i] = y[i]; }
        fbest = fx;

        double t = t_inicial;
        int iter = 0;

        while (t > t_final) {
            get_neighbor(x, y, xn, yn, n);
            EVAL_F(xn, yn, fy);

            if (fy <= fbest) {
                for (int i = 0; i < n; i++) { xbest[i] = xn[i]; ybest[i] = yn[i]; }
                fbest = fy;
                if (fbest == 0) break;
            }

            if (fy <= fx || ((double)rand()/RAND_MAX) < exp(-(double)(fy - fx)/t)) {
                for (int i = 0; i < n; i++) { x[i] = xn[i]; y[i] = yn[i]; }
                fx = fy;
            }

            iter++;
            t *= 0.99;
        }

        printf("Solucion final:\n");
        //printf("x: "); for (int i = 0; i < n; i++) printf("%d ", xbest[i]); printf("\n");
        //printf("y: "); for (int i = 0; i < n; i++) printf("%d ", ybest[i]); printf("\n");
        printf("con f(x,y) = %d\n", fbest);
        printf("iteraciones SA = %d\n", iter);

        uswtime(&utime0, &stime0, &wtime0);

        printf("\nBenchmarks (sec):\n");
        printf("real %.3f\n", wtime0 - wtime2);
        printf("user %.3f\n", utime0 - utime2);
        printf("sys  %.3f\n", stime0 - stime2);
        printf("\n");
        printf("CPU/Wall %.3f %%\n",
               100.0 * (utime0 - utime2 + stime0 - stime2) / (wtime0 - wtime2));
        printf("\n");

        free(x); free(y);
        free(xbest); free(ybest);
        free(xn); free(yn);
    }

    MPI_Abort(MPI_COMM_WORLD, MPI_SUCCESS);
    return 0;
}
