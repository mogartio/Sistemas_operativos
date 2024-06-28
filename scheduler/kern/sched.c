#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

#define MAX_PIDS 100000

static int cantidad_llamadas_sched;

// Basado en codigo de la universidad de Virginia
static unsigned random_seed = 1;
static int contador_llamadas_pid[100000];


static int pids[MAX_PIDS];
static int indice_pid = 0;

void
imprimir_estadisticas()
{
	cprintf("Cantidad de llamadas a sched: %d\n", cantidad_llamadas_sched);
	cprintf("Los procesos ejecutados cronologicamente:\n");
	int i = 0;
	for (i; i < indice_pid; i++) {
		cprintf("[%d]\n", pids[i]);
		contador_llamadas_pid[pids[i]]++;
	}
	for (i = 0; i < 10000; i++) {
		if (contador_llamadas_pid[i] == 0) {
			continue;
		}
		cprintf("El proceso [%d] fue ejecutado %d veces\n",
		        i,
		        contador_llamadas_pid[i]);
	}
}


#define RANDOM_MAX ((1u << 31u) - 1u)
unsigned
lcg_parkmiller(unsigned *state)
{
	const unsigned N = 0x7fffffff;
	const unsigned G = 48271u;
	unsigned div =
	        *state / (N / G); /* max : 2,147,483,646 / 44,488 = 48,271 */
	unsigned rem =
	        *state % (N / G); /* max : 2,147,483,646 % 44,488 = 44,487 */

	unsigned a = rem * G;       /* max : 44,487 * 48,271 = 2,147,431,977 */
	unsigned b = div * (N % G); /* max : 48,271 * 3,399 = 164,073,129 */

	return *state = (a > b) ? (a - b) : (a + (N - b));
}

unsigned
next_random()
{
	return lcg_parkmiller(&random_seed);
}
// cuenta la cantidad de tickets pertenecientes a envs RUNNABLE
int
calcular_tickets()
{
	struct Env *idle;

	int contador = 0;
	for (int current_pos = 0; current_pos < NENV; current_pos++) {
		idle = &envs[current_pos];
		if (idle->env_status == ENV_RUNNABLE) {
			contador = contador + idle->tickets;
		}
	}
	return contador;
}

void sched_halt(void);

// Choose a user environment to run and run it.
void
sched_yield(void)
{
	struct Env *idle;
	cantidad_llamadas_sched++;
#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here - Round robin

	int starting_pos = curenv ? ENVX(curenv->env_id) + 1 : 0;
	// Search for next env.
	for (int current_pos = starting_pos; current_pos < NENV + starting_pos;
	     current_pos++) {
		idle = &envs[current_pos % NENV];
		if (idle->env_status == ENV_RUNNABLE) {
			break;
		}
		idle = NULL;
	}
	if (idle) {
		pids[indice_pid] = idle->env_id;
		indice_pid++;
		env_run(idle);
	}

	if (!idle && curenv && curenv->env_status == ENV_RUNNING) {
		pids[indice_pid] = curenv->env_id;
		indice_pid++;
		env_run(curenv);
	}
	if (!idle && (!curenv || curenv->env_status != ENV_RUNNING)) {
		sched_halt();
	}
#endif

#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// Your code here - Priorities
	int contador = 0;
	idle = curenv;
	int tickets_totales = calcular_tickets();
	if (tickets_totales == 0) {
		if (curenv && curenv->env_status == ENV_RUNNING) {
			pids[indice_pid] = curenv->env_id;
			indice_pid++;
			env_run(curenv);
		} else {
			sched_halt();
		}
	}
	int ganador = next_random() % tickets_totales;
	int current_pos = 0;
	for (current_pos; current_pos < NENV; current_pos++) {
		idle = &envs[current_pos];
		if (idle->env_status == ENV_RUNNABLE) {
			contador = contador + idle->tickets;
		} else {
			continue;
		}
		if (contador > ganador) {
			pids[indice_pid] = idle->env_id;
			indice_pid++;
			env_run(idle);
			break;  // Encontramos al ganador!
		}
	}
#endif
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;
	// cprintf("DEBUGGING\n");

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		imprimir_estadisticas();
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
