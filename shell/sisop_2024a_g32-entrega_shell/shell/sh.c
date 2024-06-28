#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"

char prompt[PRMTLEN] = { 0 };
#define STACK_SIZE (1024 * 1024)

stack_t alternative_stack;

// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

void
handler(int signum)
{
	int status;
	pid_t child_pid;

	while ((child_pid = waitpid(0, &status, WNOHANG)) > 0) {
		char message[100];
		int length = snprintf(message,
		                      sizeof(message),
		                      "===> Proceso en segundo plano "
		                      "terminado: (PID=%d) (Estado=%d)\n",
		                      child_pid,
		                      status);
		if (length < 0 || (size_t) length >= sizeof(message)) {
			perror("snprintf");
			exit(EXIT_FAILURE);
		}

		if (write(STDOUT_FILENO, message, strlen(message)) < 0) {
			perror("write");
			exit(EXIT_FAILURE);
		}
	}
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	struct sigaction action;
	action.sa_handler = handler;
	action.sa_flags = SA_RESTART | SA_ONSTACK;

	alternative_stack.ss_sp = malloc(STACK_SIZE);
	if (alternative_stack.ss_sp == NULL) {
		perror("malloc");
		exit(1);
	}
	alternative_stack.ss_size = STACK_SIZE;
	alternative_stack.ss_flags = 0;

	if (sigaltstack(&alternative_stack, NULL) == -1) {
		perror("sigaltstack");
		free(alternative_stack.ss_sp);
		exit(1);
	}

	if (sigaction(SIGCHLD, &action, NULL) == -1) {
		perror("sigaction");
		free(alternative_stack.ss_sp);
		exit(1);
	}

	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

int
main(void)
{
	init_shell();

	run_shell();

	free(alternative_stack.ss_sp);

	return 0;
}
