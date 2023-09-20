#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void printMatrix(int arr[], int height, int width);
void fillMatrix(int arr[], int height, int width);

int main(void) {
  int velocidades_empleados[256];
  int velocidad_dron = 0;

  // ---largo----
  //            |
  //            |
  //            ancho
  //            |
  //            |
  int largo_parcela = 0;
  int ancho_parcela = 0;
  int ticks_por_segundo = 1;
  printf("Ingresa el largo y ancho de la parcela:\n");
  fscanf(stdin, "%d %d", &largo_parcela, &ancho_parcela);
  fgetc(stdin); // Quita el \n del final

  int index = 0;
  printf("Ingresa las velocidades de los empleados:\n");
  while ((fscanf(stdin, "%d", &velocidades_empleados[index])) == 1) {
    fgetc(stdin); // Quitar el \n del final.
    index++;
  }
  fgetc(stdin); // Quitar \n después del '-'
  printf("Ingresa la velocidad del dron:\n");
  fscanf(stdin, "%d", &velocidad_dron);
  printf("Ingresa los ticks por segundo:\n");
  fscanf(stdin, "%d", &ticks_por_segundo);

  for (int i = 0; i < index; i++) {
    printf("La velocidad del empleado %d es %d\n", i, velocidades_empleados[i]);
  }

  int empleados_terminaron = 0;
  int dron_termino = 0;

  int dron_tick_count = 0;
  int empleados_tick_count = 0;

  int parcela_empleados[largo_parcela * ancho_parcela];
  int parcela_dron[largo_parcela * ancho_parcela];
  uint wait_seconds = 1000000 / ticks_por_segundo;
  int total_a_fumigar = largo_parcela * ancho_parcela;

  fillMatrix(parcela_dron, largo_parcela, ancho_parcela);
  fillMatrix(parcela_empleados, largo_parcela, ancho_parcela);

  int velocidad_conjunta_empleados = 0;
#pragma omp parallel for reduction(+ : velocidad_conjunta_empleados)
  for (int empleadoI = 0; empleadoI < index; empleadoI++) {
    velocidad_conjunta_empleados += velocidades_empleados[empleadoI];
  }

  printf("Los empleados en conjunto fumigan %d secciones por tick\n",
         velocidad_conjunta_empleados);

#pragma omp parallel sections shared(empleados_terminaron, dron_termino,       \
                                     parcela_dron, parcela_empleados)
  {
#pragma omp section
    {
      // Mostrar matrices...
      int frame_count = 0;
      while (!empleados_terminaron || !dron_termino) {
        printf("\033[H\033[2J");
        printf("\033[92mParcela Empleados:\n");
        printMatrix(parcela_empleados, ancho_parcela, largo_parcela);

        printf("\033[92mParcela Dron:\n");
        printMatrix(parcela_dron, ancho_parcela, largo_parcela);
        printf("Frame %d...\n", ++frame_count);
        usleep(1000000 / 60); // 60 frames per second.
      }
	  // Mostrando estado final...
      printf("\033[H\033[2J");
      printf("\033[92mParcela Empleados:\n");
      printMatrix(parcela_empleados, ancho_parcela, largo_parcela);

      printf("\033[92mParcela Dron:\n");
      printMatrix(parcela_dron, ancho_parcela, largo_parcela);
      printf("Frame %d...\n", ++frame_count);

      printf("Se termino de fumigar, calculando datos...\n");
    }

#pragma omp section
    {
      int seccion_sin_fumigar = -1;
      // Drone fumiga parcela...
      while (!dron_termino) {
        dron_tick_count += 1;

        for (int i = 0; i <= velocidad_dron; i++) {
          seccion_sin_fumigar += 1;
          parcela_dron[seccion_sin_fumigar] = 1;
          dron_termino = seccion_sin_fumigar == (total_a_fumigar - 1);

          if (dron_termino) {
            break;
          }
        }

        usleep(wait_seconds);
      }
    }

#pragma omp section
    {
      int seccion_sin_fumigar = -1;

      while (!empleados_terminaron) {
        empleados_tick_count += 1;

        for (int i = 0; i <= velocidad_conjunta_empleados; i++) {
          seccion_sin_fumigar += 1;
          parcela_empleados[seccion_sin_fumigar] = 1;
          empleados_terminaron = seccion_sin_fumigar == (total_a_fumigar - 1);

          if (empleados_terminaron) {
            break;
          }
        }

        usleep(wait_seconds);
      }
    }
  }

  return 0;
}

void printMatrix(int arr[], int height, int width) {
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      int index = i * width + j;
      int should_color = arr[index];

      if (should_color) {
        printf("\033[34m 1 ");
      } else {
        printf("\033[91m 0 ");
      }
    }
    printf("\n"); // Moverse a la siguiente fila
  }
}

void fillMatrix(int arr[], int height, int width) {
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      arr[i * width + j] = 0;
    }
  }
}
