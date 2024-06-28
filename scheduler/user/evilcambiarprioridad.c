#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	envid_t id_actual = sys_getenvid();
	int prioridad_inicial = sys_get_prioridad(id_actual);
	cprintf("La id del proceso actual es %d tiene %d tickets\n",
	        id_actual,
	        prioridad_inicial);
	cprintf("le voy a agregar un ticket a %d\n", id_actual);
	sys_cambiar_prioridad(id_actual, prioridad_inicial + 1);
	int prioridad_nueva = sys_get_prioridad(id_actual);
	if (prioridad_nueva > prioridad_inicial) {
		cprintf("La id del proceso actual es %d tenia %d tickets pero "
		        "ahora tiene %d\n",
		        id_actual,
		        prioridad_inicial,
		        prioridad_nueva);
	} else {
		cprintf("No se le pudo agregar tickets al proceso %d\n",
		        id_actual);
	}
}