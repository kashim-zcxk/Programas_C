/**
* Ejemplo de un hilo periodico.
*
* @Author Marco Antonio Cruz
* Licencia GPLv2.
*/

#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*Numero de ticks por segundo*/
#define NSEC_PER_SEC 1000000000L
#define timespec_normalize(t){\
	if((t)->tv_nsec >= NSEC_PER_SEC){\
		(t)->tv_nsec -= NSEC_PER_SEC;\
		(t)->tv_sec++;\
	}else if((t)->tv_nsec < 0){\
		(t)->tv_nsec += NSEC_PER_SEC;\
		(t)->tv_sec--;\
	}\
}
#define timespec_add(t1,t2) do{\
	(t1)->tv_nsec += (t2)->tv_nsec;\
	(t1)->tv_sec += (t2)->tv_sec;\
	timespec_normalize(t1);\
}while(0)

enum estado_hilo
{
    SALIR    = 0,
    EJECUTAR = 1
};

unsigned char estado = EJECUTAR;

void *callback_Hilo_Periodico(void *t);
void catch(int sig);

/**
* @brief Función que se ejecuta al recibir una señal de interrupción (Ctrl + c).
* Termina el ciclo del hilo periodico.
* @param sig Tipo de interrupción capturada.
*/
void catch(int sig)
{
    puts("¡Saliendo del programa xD!");
    estado = SALIR;

}

/**
* @brief Imprime de forma indefinida una frase.
* @param variable Ejemplo de paso de una variable.
*/
void *callback_Hilo_Periodico(void *variable)
{
    int num_hilo = (intptr_t)variable;
    struct timespec siguiente;
    struct timespec periodo_hilo;

    /*Periodo del hilo*/
    periodo_hilo.tv_nsec = 0;
    periodo_hilo.tv_sec = 2;

    printf("Hilo %d: fue creado.\n",num_hilo);
    /*Se obtiene el tiempo de la primera interrupción*/
    clock_gettime(CLOCK_MONOTONIC, &siguiente);

    while(estado == EJECUTAR)
    {
	/*Se suma el tiempo siguiente más el tiempo del periodo
	* tiempo_siguiente = tiempo_siguiente + tiempo_periodo	
	* Esto se debe porque usamos un timer absoluto
	*/
        timespec_add(&siguiente,&periodo_hilo);
	/*El hilo duerme*/
        clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&siguiente,NULL);
	printf("¡Hola! Soy un hilo periodico. %ld\n",(long)siguiente.tv_sec);			
    }

    puts("Terminando el hilo.");
    pthread_exit(NULL);
}

/**
* @brief Funcion principal
*/
int main(int argc, char **argv)
{
    pthread_t hilo_periodico;
    int rc, num_hilo=0;

    /*Configuración de la señal*/
    signal(SIGINT, &catch);

    printf("Creando Hilos\n");
    /*Creando el hilo*/
    rc=pthread_create(&hilo_periodico,NULL,callback_Hilo_Periodico,(void *)(intptr_t)num_hilo);

    if(rc)
    {
        printf("Error al crear el hilo: %d\n",rc);
        return -1;
    }

    pthread_exit(NULL);
    return 0;
}
