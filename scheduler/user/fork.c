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
	int prioridad_nueva = sys_get_prioridad(id_actual);

	cprintf("Le voy a sacar 50 tickets a %d\n", id_actual);
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
	cprintf("Forkeo a %d\n", id_actual);
	int pid = fork();


	if (pid == 0) {
		cprintf("soy el hijo de %d, mi id es %d y tengo %d tickets\n",
		        id_actual,
		        sys_getenvid(),
		        sys_get_prioridad(sys_getenvid()));
	} else {
	}
}