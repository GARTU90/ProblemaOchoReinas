# ProblemaOchoReinas

El problema de las ocho reinas consiste en poner ocho reinas en un tablero de ajedrez que es de $8x8$.

Consiste en generar tablero con 8 reinas aleatorio y se usa el algoritmo de simulated annealing para generar un vecino y de decidir si explorar esa nueva solucion.

La parte a paralelizar es la funcion objetivo que consiste en revisar las diagonales de cada reina en el tablero y revisar si se encuentra en el tablero.

Ejemplo 8x8:

<img width="408" height="413" alt="figura 1" src="https://github.com/user-attachments/assets/d213a8be-a9d8-4626-9466-191d3cd9748c" />


Para poder probar la eficiencia de el cluster se deicio tomar un problema con $50$ reinas en un tablero de $50 x 50$
en el cluster se utilizaron 8 nodos, un maestro y 7 esclavos.
Se corrio el algoritmo 10 veces por grupo de nodos, es decir, 10 veces con 8 nodo, 10 con 7 nodos, y asi sucesivamente.
Tambien se agregaron las barras de error usando la desviacione satndar como criterio. el resultado fue el siguiente:

<img width="889" height="490" alt="Grafico_nodos_vs_tiempo" src="https://github.com/user-attachments/assets/77f58f73-3ebf-4131-b70a-a2e65603a76f" />

hay un claro descenco en los tiempos de procesamiento entre mas nodos se utilzian como es lo esperado.


