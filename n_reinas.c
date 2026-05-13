#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Función para verificar si un par (a,b) está en la matriz
int contiene(int *x, int *y, int n, int a, int b) {
    for (int i = 0; i < n; i++) {
        if (x[i] == a && y[i] == b) return 1;
    }
    return 0;
}

// Función objetivo
int f(int *x, int *y, int n) {
    int cont = 0;
    int **vistos = malloc(n * n * sizeof(int*));
    int vistos_count = 0;

    for (int i = 0; i < n; i++) {
        int a = x[i], b = y[i];
        for (int j = 1; j < n; j++) {
            int candidatos[4][2] = {
                {a+j, b+j},
                {a-j, b+j},
                {a+j, b-j},
                {a-j, b-j}
            };
            for (int k = 0; k < 4; k++) {
                int cx = candidatos[k][0];
                int cy = candidatos[k][1];
                if (contiene(x, y, n, cx, cy)) {
                    // Verificar si ya lo contamos
                    int repetido = 0;
                    for (int m = 0; m < vistos_count; m++) {
                        if (vistos[m][0] == cx && vistos[m][1] == cy) {
                            repetido = 1;
                            break;
                        }
                    }
                    if (!repetido) {
                        vistos[vistos_count] = malloc(2 * sizeof(int));
                        vistos[vistos_count][0] = cx;
                        vistos[vistos_count][1] = cy;
                        vistos_count++;
                        cont++;
                    }
                }
            }
        }
    }

    for (int i = 0; i < vistos_count; i++) free(vistos[i]);
    free(vistos);
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

// Generar vecino
void get_neighbor(int *x, int *y, int *x_new, int *y_new, int n) {
    for (int i = 0; i < n; i++) {
        x_new[i] = x[i];
        y_new[i] = y[i];
    }
    if ((double)rand()/RAND_MAX < 0.5) {
        int i = rand() % n, j = rand() % n;
        while (j == i) j = rand() % n;
        int tmp = x_new[i]; x_new[i] = x_new[j]; x_new[j] = tmp;
    } else {
        int i = rand() % n, j = rand() % n;
        while (j == i) j = rand() % n;
        int tmp = y_new[i]; y_new[i] = y_new[j]; y_new[j] = tmp;
    }
}

// Simulated Annealing
void simulated_annealing(double t_0, double t_f, int n) {
    int *x = malloc(n * sizeof(int));
    int *y = malloc(n * sizeof(int));
    get_initial_solution(x, y, n);
    int fx = f(x, y, n);

    int *xbest = malloc(n * sizeof(int));
    int *ybest = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) { xbest[i] = x[i]; ybest[i] = y[i]; }
    int fbest = fx;

    double t = t_0;
    int cont = 0;

    int *x_new = malloc(n * sizeof(int));
    int *y_new = malloc(n * sizeof(int));

    while (t > t_f) {
        get_neighbor(x, y, x_new, y_new, n);
        int fy = f(x_new, y_new, n);

        if (fy <= fbest) {
            for (int i = 0; i < n; i++) { xbest[i] = x_new[i]; ybest[i] = y_new[i]; }
            fbest = fy;
            if (fbest == 0) break;
        }

        if (fy <= fx || ((double)rand()/RAND_MAX) < exp(-(fy - fx)/t)) {
            for (int i = 0; i < n; i++) { x[i] = x_new[i]; y[i] = y_new[i]; }
            fx = fy;
        }
        cont++;
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
    srand(time(NULL));
    if (argc < 4) {
        printf("Uso: %s n t_inicial t_final\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);
    double t_inicial = atof(argv[2]);
    double t_final = atof(argv[3]);

    simulated_annealing(t_inicial, t_final, n);
    return 0;
}

