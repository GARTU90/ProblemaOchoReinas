# Problema de las Ocho Reinas
El **problema de las ocho reinas** consiste en colocar ocho reinas en un tablero de ajedrez de $8 \times 8$ de tal manera que ninguna de ellas pueda atacar a otra.

## Descripción del Algoritmo

Para resolverlo, el sistema genera un tablero inicial con 8 reinas posicionadas de forma aleatoria. Posteriormente, se utiliza el algoritmo de **Recocido Simulado (*Simulated Annealing*)** para generar soluciones vecinas y decidir si se explora o no cada nueva configuración en función de la probabilidad de aceptación.

### Paralelización
La sección optimizada mediante paralelización es la **función objetivo**. Esta se encarga de revisar exhaustivamente las diagonales y filas de cada reina en el tablero para contabilizar los conflictos existentes.

> **Ejemplo de un tablero $8 \times 8$:**
>
> <img width="408" height="413" alt="Figura 1: Tablero de 8x8" src="https://github.com/user-attachments/assets/d213a8be-a9d8-4626-9466-191d3cd9748c" />

---

## Escalabilidad y Pruebas en Clúster

Este problema es fácilmente escalable en dimensión bidimensional: si el número de reinas ($N$) aumenta, el tamaño del tablero crece proporcionalmente a $N \times N$. 

Para evaluar la eficiencia y el rendimiento del clúster, se escaló el problema a **$50$ reinas en un tablero de $50 \times 50$**.

### Configuración del Experimento
* **Infraestructura:** Se utilizó un clúster de 8 nodos (1 maestro y 7 esclavos).
* **Metodología:** El algoritmo se ejecutó 10 veces por cada configuración de nodos (es decir, 10 iteraciones con 8 nodos, 10 iteraciones con 7 nodos, y así sucesivamente).
* **Visualización:** Los gráficos incluyen barras de error calculadas a partir de la desviación estándar de los tiempos de ejecución.

### Resultados

<img width="889" height="490" alt="Gráfico de Nodos vs Tiempo" src="https://github.com/user-attachments/assets/77f58f73-3ebf-4131-b70a-a2e65603a76f" />

Se observa un claro descenso en los tiempos de procesamiento a medida que se incorporan más nodos al clúster. Sin embargo, al alcanzar los **6 nodos**, la mejora en el rendimiento deja de ser significativa para este tamaño de problema ($50$ reinas), estabilizando la curva de tiempo de ejecución.

