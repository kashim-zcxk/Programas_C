/**
* Programación concurrente.
* El primer hilo (Hilo_Secuencia1) deberá imprimir de forma indefinida la secuencia de números fibonacci. El
* hilo al ser interrumpido debe imprimir la secuencia de números primos.
* El segundo hilo (Hilo_Secuencia2) deberá imprimir de forma indefinida la secuencia de números aleatorios
* entre 1 - 100. El hilo al ser interrumpido debe imprimir la secuencia de números pares.
* El tercer hilo (Hilo_Interrupciones) deberá interrumpir a los hilos 1 y 2 cada cierto tiempo.
* Debe monitorear la actividad de los hilos 1,2,3 y guardar la siguiente información en un archivo log:
*	ID del Hilo
*	Nombre del Hilo
*	Nombre de la Secuencia
*	Numero
*
* Para compilar en Linux: gcc timer_t.c -o timer_t -lrt -lpthread -lm
*
* @Author Marco Antonio Cruz
* Licencia GPLv2.
*/

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define LOG_NAME "log.zz"
#define NUM_INT	4
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
    OPCION1  = 0,
    OPCION2  = 1,
    EJECUTAR = 1 << 1,
    SALIR    = ~0
};

struct timespec period1;
struct timespec period2;
struct timespec period3;

unsigned char opcion_hilo1 = OPCION1;
unsigned char opcion_hilo2 = OPCION1;

unsigned char estado_hilo1 = EJECUTAR;
unsigned char estado_hilo2 = EJECUTAR;

FILE *fd_log;

/**
* @brief Verifica si un numero es primo.
* @param primo El numero a verificar si es primo.
* @return Regresa 1 si es primo y 0 en caso contrario. 
*/
int primo_check(unsigned long primo)
{
    int i;
    for(i=2; i<=sqrt(primo); i++)
        if(primo%i == 0) return 0;
    return 1;
}

/**
* @brief Imprime la hora en stdout y en log.
*/
void tiempo()
{
    time_t tiempo = time(0);
    struct tm *tlocal = localtime(&tiempo);
    char output[128];
    strftime(output,128,"%H:%M:%S",tlocal);
    printf("%s ",output);
    fprintf(fd_log,"%s",output);
}

/**
* @brief Imprime de forma indefinida la secuencia de números fibonacci. El
* hilo al ser interrumpido debe imprimir la secuencia de números primos.
* @param t Numero de hilo.
*/
void *hilo_Secuencia1(void *t)
{
    int taskno = (intptr_t)t;
    struct timespec next;
    unsigned long res[]= {0, 1, 1};
    unsigned long primo = 1;

    printf("Hilo %d Secuencia1 Creado\n",taskno);
    clock_gettime(CLOCK_REALTIME, &next);

    while(estado_hilo1 == EJECUTAR)
    {
        timespec_add(&next,&period1);
	/*El hilo duerme el tiempo especificado.*/
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME,&next,NULL);		
        tiempo();
	/*Serie de Fibonacci.*/
        if(opcion_hilo1 == OPCION1)
        {
            res[2] = res[1] + res[0];
            printf("{ Hilo : %s%d%s, Fibonacci : %s%lu%s }\n", "\033[92m", taskno, "\033[0m", "\033[92m", res[2], "\033[0m");
            fprintf(fd_log,"{\n\tHilo : %d, \n\tNombre : %s, \n\tSecuencia : %s, \n\tNumero : %lu \n}\n", taskno, "hilo_Secuencia1", "Fibonacci", res[2] );
            res[0] = res[1];
            res[1] = res[2];
        }
	/*Numeros Primos*/
        else
        {
            do
            {
                primo++;
            }
            while(primo_check(primo) == 0);
            printf("{ Hilo : %s%d%s, Primos : %s%lu%s }\n", "\033[92m", taskno, "\033[0m", "\033[93m", primo, "\033[0m");
            fprintf(fd_log,"{\n\tHilo : %d, \n\tNombre : %s, \n\tSecuencia : %s, \n\tNumero : %lu \n}\n", taskno, "hilo_Secuencia1", "Primos", primo);
        }
    }

    pthread_exit(NULL);
}

void *hilo_Secuencia2(void *t)
{
    int taskno = (intptr_t)t;
    struct timespec next;
    int num = 0, par = 0;

    printf("Hilo %d Secuencia2 Creado\n",taskno);
    clock_gettime(CLOCK_REALTIME, &next);

    while(estado_hilo2 == EJECUTAR)
    {
        timespec_add(&next,&period2);
	/*El hilo duerme el tiempo especificado.*/
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME,&next,NULL); 	
        tiempo();
	/*Numeros Aleatorios*/
        if(opcion_hilo2 == OPCION1)
        {
            num = 1+rand()%(101-1);
            printf("{ Hilo : %s%d%s, Aleatorio : %s%d%s }\n", "\033[91m", taskno, "\033[0m", "\033[94m", num, "\033[0m");
            fprintf(fd_log,"{\n\tHilo : %d, \n\tNombre : %s, \n\tSecuencia : %s, \n\tNumero : %d \n}\n", taskno, "hilo_Secuencia2", "Aleatorio", num);
        }
	/*Numeros Pares*/
        else
        {
            printf("{ Hilo : %s%d%s, Pares : %s%d%s }\n", "\033[91m", taskno, "\033[0m", "\033[91m", par, "\033[0m");
            fprintf(fd_log,"{\n\tHilo : %d, \n\tNombre : %s, \n\tSecuencia : %s \n\tNumero : %d \n}\n", taskno, "hilo_Secuencia2", "Pares", par);
            par += 2;
        }
    }

    pthread_exit(NULL);
}

void *hilo_Interrupciones(void *t)
{
    int taskno = (intptr_t)t;
    int contador_Int = 0;
    struct timespec next;

    printf("Hilo %d Interrupciones Creado\n\n",taskno);
    clock_gettime(CLOCK_REALTIME, &next);

    do
    {
        timespec_add(&next,&period3);
	/*El hilo duerme el tiempo especificado.*/
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME,&next,NULL);	
        tiempo();
        printf("{ Hilo : %s%d%s, Toggle : %s%d%s }\n", "\033[95m", taskno, "\033[0m", "\033[95m", contador_Int + 1, "\033[0m");
        fprintf(fd_log,"{\n\tHilo : %d, \n\tNombre : %s, \n\tToggle : %d \n}\n", taskno, "hilo_Interrupciones", contador_Int + 1);
	/*Se realizan las "intrrupciones"*/
        if(opcion_hilo1 == OPCION1 && opcion_hilo2 == OPCION1)
        {
            opcion_hilo1 = OPCION2;
            opcion_hilo2 = OPCION2;
        }
        else
        {
            opcion_hilo1 = OPCION1;
            opcion_hilo2 = OPCION1;
        }
        contador_Int ++;
    }
    while(contador_Int < NUM_INT);	/*Despues de realizar <NUM_INT> interrupciones el programa termina*/ 

    estado_hilo1 = SALIR;
    estado_hilo2 = SALIR;

    pthread_exit(NULL);
}

/**
* @brief Funcion principal
*/
int main(int argc, char **argv)
{
    srand(time(NULL));
    /*Se abre el archivo de log*/
    if((fd_log = fopen(LOG_NAME, "a"))==NULL)
    {
        perror("log.zz");
        return -1;
    }

    pthread_t threads[3];
    int rc,t=0;

    /*Periodo de hilo_Secuencia1*/
    period1.tv_nsec=400000;
    period1.tv_sec=1;
    /*Periodo de hilo_Secuencia2*/
    period2.tv_nsec=500000;
    period2.tv_sec=2;
    /*Periodo de hilo_Interrupciones*/
    period3.tv_nsec=900000;
    period3.tv_sec=5;

    printf("Creando Hilos\n");
    rc=pthread_create(&threads[0],NULL,hilo_Secuencia1,(void *)(intptr_t)t);
    if(rc)
    {
        printf("Error %d\n",rc);
        return -1;
    }

    rc=pthread_create(&threads[1],NULL,hilo_Secuencia2,(void *)(intptr_t)++t);
    if(rc)
    {
        printf("Error %d\n",rc);
        return -1;
    }

    rc=pthread_create(&threads[1],NULL,hilo_Interrupciones,(void *)(intptr_t)++t);
    if(rc)
    {
        printf("Error %d\n",rc);
        return -1;
    }

    pthread_exit(NULL);
    fclose(fd_log);
    return 0;
}

