#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	envid_t id_actual = sys_getenvid();
	int prioridad_inicial = sys_get_prioridad(id_actual);
	cprintf("La id del proceso actual es %d tiene %d tickets\n",
	        id_actual,
	        prioridad_inicial);

	sys_cambiar_prioridad(id_actual, 50);
	cprintf("Le voy a sacar 50 tickets a %d\n", id_actual);
	int prioridad_nueva = sys_get_prioridad(id_actual);
	if (prioridad_inicial >= prioridad_nueva) {
		cprintf("El proceso actual es %d tenia %d tickets pero ahora "
		        "tiene %d\n",
		        id_actual,
		        prioridad_inicial,
		        prioridad_nueva);
	} else {
		cprintf("No se pudo modificar la cantidad de tickets del "
		        "proceso %d\n",
		        id_actual);
	}
}