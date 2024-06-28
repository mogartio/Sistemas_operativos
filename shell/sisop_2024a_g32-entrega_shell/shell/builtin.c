#include "builtin.h"
#include "limits.h"


// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	return (strncmp(cmd, "exit", 5) == 0);
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['cs','d', '\0']
int
cd(char *cmd)
{
	if (strncmp(cmd, "cd ", 3) != 0 && strncmp(cmd, "cd", 3) != 0) {
		return false;
	}

	char *path;  // Asumimos un tamaño máximo para la ruta

	char *separator = " ";
	path = strtok(cmd, separator);   // Debería ser "cd"
	path = strtok(NULL, separator);  // La ruta después de "cd"

	if (path == NULL) {
		path = getenv("HOME");
	}

	int res = chdir(path);
	if (res == -1) {
		perror("Error al cambiar de directorio");
		return false;
	}

	snprintf(prompt, sizeof prompt, "(%s)", getcwd(path, PRMTLEN));

	return true;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strncmp(cmd, "pwd", 4) != 0) {
		return false;
	}

	char current_directory[PATH_MAX];
	int res = getcwd(current_directory, sizeof(current_directory));
	if (res == -1) {
		_exit(-1);
		return false;
	}
	printf("%s\n", current_directory);
	return true;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}
