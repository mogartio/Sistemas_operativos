#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	envid_t id_actual = sys_getenvid();
	int prioridad = sys_get_prioridad(id_actual);
	cprintf("La id del proceso actual es %d tiene %d tickets\n",
	        id_actual,
	        prioridad);
}