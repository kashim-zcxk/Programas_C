
CC 		= gcc
CFLAGS 		= -O3 -lrt -lpthread -I.
RM		= rm -f

default:
	$(CC) thread_period.c gnuplot_i.c -o thread_period $(CFLAGS) 
	@echo '¡Compilación Terminada :D!'

clean:
	$(RM) thread_period data.temp
	@echo '¡Borrando :(!'
