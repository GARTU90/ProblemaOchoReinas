#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

//./a.out -n -m -p
int main(int argn, char **args){
   int rank, size;
   char hostname[MPI_MAX_PROCESSOR_NAME];
   int len;

   // Inicializa el entorno MPI
   MPI_Init(&argn, &args);

   MPI_Barrier(MPI_COMM_WORLD);

   // Obtiene el número de proceso (rank)
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   // Obtiene el número total de procesos
   MPI_Comm_size(MPI_COMM_WORLD, &size);
   MPI_Get_processor_name(hostname, &len);
   
   printf("Hola mundo soy el proceso %i de %i en %s\n",rank,size,hostname);

   // Finaliza MPI
   
   MPI_Finalize();
   return 0;
}
