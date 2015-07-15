/**
* Programa que grafica un hilo periodico por medio de Gnuplot.
* @Author Marco Antonio Cruz
* Licencia GPLv2.
*/

#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "gnuplot_i.h"

#define SLEEP_LGTH  1

/*Amplitud del pulso generado*/
#define AMPLITUD 10
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

#define timespec_diff(t1,t2)\
	(double)(t1.tv_sec + (double)t1.tv_usec/1000000) -\
	(double)(t2.tv_sec + (double)t2.tv_usec/1000000);

#define timespec_freq(t1,fq)do{\
	double time = (double)(t1.tv_sec + (double)t1.tv_nsec/1e9);\
	fq = 1 / time;\
}while(0)

enum estado_hilo
{
    SALIR    = 0,
    EJECUTAR = 1
};

struct timespec periodo_pulso;
struct timespec periodo_grafica;
int pulso = 0;
unsigned char estado = EJECUTAR;

void catch(int sig);
void *hilo_Generador_Pulso(void *variable);
void *hilo_Grafica(void *variable);

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
* @brief Hilo generador de un tren de pulsos.
* @param variable Ejemplo de paso de una variable.
*/
void *hilo_Generador_Pulso(void *variable)
{
    int num_hilo = (intptr_t)variable;
    struct timespec interrupcion_pulso;

    printf("Hilo %d: Generador pulso fue creado\n",num_hilo);
    /*Se obtiene un tiempo base*/
    clock_gettime(CLOCK_REALTIME, &interrupcion_pulso);

    while(estado == EJECUTAR)
    {
	/*Se suma el tiempo interrupcion más el tiempo del periodo
	* tiempo_Int = tiempo_Int + tiempo_periodo	
	*/
        timespec_add(&interrupcion_pulso,&periodo_pulso);
	/*El hilo duerme*/
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME,&interrupcion_pulso,NULL);
	if(pulso != AMPLITUD){
		pulso = AMPLITUD;
	}else{
		pulso = 0;
	}			
    }

    puts("Terminando el hilo generador de pulso");
    pthread_exit(NULL);
}

/**
* @brief Hilo que grafica el tren de pulsos generado.
* @param variable Ejemplo de paso de una variable.
*/
void *hilo_Grafica(void *variable)
{
    int num_hilo = (intptr_t)variable;
    struct timespec interrupcion_grafica;
    struct timeval t_ini, t_fin;
    double secs;

    printf("Hilo %d: Grafica fue creado\n",num_hilo);
    /*Se obtiene un tiempo base*/
    clock_gettime(CLOCK_REALTIME, &interrupcion_grafica);
    /*Configurando gnuplot*/
    gnuplot_ctrl *h1 = gnuplot_init() ;
    gnuplot_cmd(h1,"set title \"Grafica del Hilo\"\n");
    gnuplot_cmd(h1,"set xlabel \"Tiempo\"\n");
    gnuplot_cmd(h1,"set ylabel \"Amplitud\"\n");
    gnuplot_cmd(h1,"set yrange [0:%g]\n", AMPLITUD*1.5);
    /*Archivo temporal en el cual estan los puntos a graficar*/
    FILE * temp = fopen("data.temp", "w");
    gettimeofday(&t_ini, NULL);
    fprintf(temp, "%.16g %d\n", 0.0 , pulso);
    fflush(temp);

    while(estado == EJECUTAR)
    {
	/*Se suma el tiempo interrupcion más el tiempo del periodo
	* tiempo_Int = tiempo_Int + tiempo_periodo	
	*/
        timespec_add(&interrupcion_grafica,&periodo_grafica);
	/*El hilo duerme*/
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME,&interrupcion_grafica,NULL); 
	/*Puntos (Tiempo) para graficar en el eje X*/
	gettimeofday(&t_fin, NULL);
  	secs = timespec_diff(t_fin, t_ini);
	/*Escribiendo los puntos en el archivo temporal*/
        fprintf(temp, "%.16g %d\n", secs * 1000.0 , pulso); //Write the data to a temporary file
	fflush(temp);
        //gnuplot_resetplot(h1) ;
	/*Ploteo*/
        gnuplot_cmd(h1, "plot 'data.temp' t \"Hilo\" with linespoints linetype -1 linewidth 4 linecolor \"blue\"") ;
    }

    fclose(temp);
    gnuplot_close(h1) ;
    puts("Terminando el hilo grafica");
    pthread_exit(NULL);
}

/**
* @brief Funcion principal
*/
int main(int argc, char **argv)
{
    pthread_t threads[3];
    int rc,t=0;
    double freq;

    /*Periodo de hilo_Generador_Pulso*/
    periodo_pulso.tv_sec=2;
    /*Periodo de hilo_Grafica*/
    periodo_grafica.tv_sec=1;

    timespec_freq(periodo_pulso,freq);
    printf("Frecuencia pulso: %4gHz\n",freq);

    /*Configuración de la señal*/
    signal(SIGINT, &catch);

    /*Creando el hilo*/
    printf("Creando Hilos\n");
    rc=pthread_create(&threads[0],NULL,hilo_Generador_Pulso,(void *)(intptr_t)t);
    if(rc)
    {
        printf("Error %d\n",rc);
        return -1;
    }

    rc=pthread_create(&threads[1],NULL,hilo_Grafica,(void *)(intptr_t)++t);
    if(rc)
    {
        printf("Error %d\n",rc);
        return -1;
    }

    pthread_exit(NULL);
    return 0;
}
