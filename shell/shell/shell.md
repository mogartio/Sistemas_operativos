# shell

### Búsqueda en $PATH

#### ¿Cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

La familia de wrappers proporcionados por la librería estándar de c sirve como front-end para la system call execve(). Funcionan como interfaces y facilitan la ejecución de dicha syscall. Execve() recibe un puntero a un programa y un array de strings que representan argumentos, lo que hace es ejecutar dicho programa usando el array de strings como argumentos. Las funciones pertenecientes a la familia exec() pueden recibir un puntero que apunte al programa a ejecutar o también el path correspondiente. Además pueden recibir los argumentos en otros formatos.

#### ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

La llamada a exec puede fallar, es por eso que en la implementación se evalua el valor de retorno al llamar a execvp. Si dicho valor de retorno es negativo, se termina el proceso encargado de ejecutar execvp. Como este es un proceso hijo el proceso padre sigue iterando y la shell sigue corriendo.

### Procesos en segundo plano

#### Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.

1. Cuando un usuario ingresa un comando, este primero se analiza utilizando la función parse_line. Esta función devuelve una estructura cmd que contiene detalles sobre el comando, incluyendo su tipo. Si el usuario añade un & al final del comando, parse_line debería configurar el tipo de cmd a BACK, que indica que el comando debe ejecutarse en segundo plano.
2. La función run_cmd luego utiliza fork() para crear un nuevo proceso. El proceso hijo ejecutará el comando, mientras que el proceso padre continuará ejecutando el shell.
3. En el proceso padre, después de fork, se verifica si el tipo de comando es BACK. En dicho caso el padre no espera a que el hijo termine inmediatamente. En su lugar, utiliza waitpid con la opción WNOHANG, lo que permite al padre continuar su ejecución sin bloquearse. Se llama a la función print_back_info para imprimir detalles sobre el proceso en segundo plano.

### Flujo estándar

#### Investigar el significado de 2>&1, explicar cómo funciona su forma general
 - Mostrar qué sucede con la salida de cat out.txt en el ejemplo.
 - Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, 2>&1 >out.txt). ¿Cambió algo? Compararlo con el comportamiento en bash(1).

El file descriptor 1 salida estándar (`stdout`) y el file descriptor 2 representa a la salida de errores (`stderr`).

Por lo tanto lo que hace `2>&1` es redirigir el flujo de salida de error estándar hacia el mismo que apunta la salida estándar. La cual como se puede ver en nuestro ejemplo, se muestra antes que esta.

En nuestra implementación de shell, ya sea con el ejemplo o con el orden invertido ```ls -C /home /noexiste 2>&1 >out.txt``` nos da el mismo resultado. Esto se debe a que nuestra implementación hace un parseo interno que no diferencia dicho orden. 
```bash
$ ls -C /home /noexiste >out.txt 2>&1
$ cat out.txt
ls: cannot access '/noexiste': No such file or directory
/home:
user
```
En cambio, en **bash(1)** nos da el mismo resultado en el ejemplo, pero en el orden invertido nos da:
```bash
$ ls -C /home /noexiste 2>&1 >out.txt
ls: cannot access '/noexiste': No such file or directory
$ cat out.txt
/home:
user
```
Esto se debe ya que el error se redirige al stdout actual, que todavia apunta a consola. Despues de esto, se realiza la redirección de la salida estandar al archivo seleccionado, donde se escribe el resultado del comando.

### Tuberías múltiples

#### Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe
 - ¿Cambia en algo?
 - ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.

Podemos verificar el exit code usando `echo $?`. En caso de usar pipes, bash va a devolver el exit code del ultimo comando de la cadena. Se usan de ejemplo solo un pipe, pero pasaria tambien con multiples pipes.

Entonces, en `bash(1)`:

```bash
$ ls /README.md | echo Todo correcto, devuelve 0
Todo correcto, devuelve 0
$ echo $?
0
```

```bash
$ ls /noexiste | echo Primero mal, devuelve 0
Primero mal, devuelve 0
ls: cannot access '/noexiste': No such file or directory
$ echo $?
0
```

```bash
$ echo Segundo mal, devuelve <> 0 | ls /noexiste
ls: cannot access '/noexiste': No such file or directory
$ echo $?
2
```

En cambio, en nuestra implementación siempre que se usen pipes se va a devolver 0, ya que no implementa esta caracteristica:

```bash
$ ls /noexiste | echo En implementacion, primero mal, devuelve 0
En implementacion, primero mal, devuelve 0
ls: cannot access '/noexiste': No such file or directory
$ echo $?
0
```

```bash
$ echo En implementacion, segundo mal, devuelve 0 | ls /noexiste
ls: cannot access '/noexiste': No such file or directory
$ echo $?
0
```

### Variables de entorno temporarias

#### ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

La función fork() crea una copia del proceso actual, incluyendo su memoria y estado actual de ejecución, así como todas sus variables de entorno. Cualquier cambio realizado en el entorno después de fork() afectará únicamente al proceso hijo, dejando intacto el entorno del proceso padre. Esto es crucial para mantener el aislamiento y la independencia entre procesos

#### En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3).
- ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
- Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

Los comportamientos no son exactamente los mismos debido a cómo cada método gestiona las variables de entorno:
- Al utilizar setenv() después de fork(), las modificaciones en las variables de entorno solo afectan al proceso hijo. Estas variables modificadas permanecen disponibles en el hijo hasta que este llama a exec() sin "e" (que no permite especificar un entorno), momento en el cual el nuevo programa hereda todas las variables de entorno establecidas hasta ese punto.
- Al utilizar una función como execle() o execve(), se especifica un conjunto completamente nuevo de variables de entorno que solo afectarán al nuevo programa ejecutado. Estas variables son creadas dentro del espacio de memoria del nuevo proceso y no afectan el entorno del proceso que ejecuta exec().
  
Si se desea replicar el entorno del proceso hijo que incluye modificaciones hechas por setenv(), se debe construir manualmente un arreglo que combine las variables de entorno existentes con las modificaciones o adiciones deseadas.

### Pseudo-variables

#### Investigar al menos otras tres variables mágicas estándar, y describir su propósito.
- Incluir un ejemplo de su uso en bash (u otra terminal similar).

$$ - PID (identificador de proceso) del script o shell actual: Esta variable almacena el identificador de proceso del shell actual o del script en ejecución. 

```bash
$ echo "El PID de este proceso es" $$
El PID de este proceso es 13261
```

$!-  PID del último trabajo ejecutado en segundo plano: Esta variable almacena el identificador de proceso del ultimo proceso ejecutado en segundo plano.

```bash
$ echo "El PID de este proceso es" $$
El PID de este proceso es 13261
$ sleep 3 &
[1] 13679
$ echo
[1]+  Done                    sleep 3
$ echo "Ultimo pid en segundo plano" $!
Ultimo pid en segundo plano 13679
```

$_-  Esta variable almacena el último argumento del último proceso ejecutado por la shell.

```bash
$ sleep 1
$ echo "Ultimo argumento del proceso anterior $_"
Ultimo argumento del proceso anterior 1
```

### Comandos built-in

#### ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

El comando pwd podría ser implementado sin la necesidad de ser built-in. Se podría implementar un binario que imprima el resultado de la funcion getcwd y usar execvp. pwd es implementado como built-in ya que, al no tener que iniciar un nuevo proceso, se ejecuta mucho más rápido de lo que se ejecutaría si fuera un binario. 

### Segundo plano avanzado

Para poder gestionar correcta e inmediatamente (o lo antes posible) la terminación de un proceso en segundo plano, utilizamos el mecanismo de señales.
Más precisamente hacemos uso de la señal SIGCHLD, la cual es enviada por un proceso hijo al padre, cuando se frena o termina. 
Por defecto esta señal es ignorada, es por eso que configuramos un handler para esta señal en el proceso padre sh utilizando la syscall sigaction. 
En el handler usamos un while para manejar el caso en el que varios procesos hijos hayan terminado mientras el proceso padre está en espera de la señal SIGCHLD. La condición del while es:
```c
 waitpid(0, &status, WNOHANG)) > 0
```
Al establecer el primer parámetro de waitpid en 0 esperamos por la terminacion de cualquier proceso en el mismo pgid que el proceso en donde se esta ejecutando waitpid, en este caso sh. 
El tercer parametro hace que waitpid() sea no bloqueante. Esto significa que si no hay ningún proceso hijo que haya terminado, waitpid() regresa inmediatamente con un valor de retorno igual a 0 para indicar que no se ha encontrado ningún proceso hijo que haya terminado.
Ahora, nuestro objetivo es captar la señal SIGCHLD solo de los procesos hijos en segundo plano,  por lo que para cumplir con este objetivo hacemos uso de la syscall setpgid(pid, pgid) para cambiar el pgid de los procesos hijos que no sean procesos en segundo plano.
En estos procesos usamos setpgid(0, 0), para establecer como pgid el pid del proceso.
Además, utilizamos un stack alternativo para garantizar un manejo seguro de las señales, de forma que errores en el manejo de señales no afecten a la ejecución del resto del programa. 


#### ¿Por qué es necesario el uso de señales?

Es necesario el uso de señales, ya que si no, el proceso padre tendría que hacer una espera activa para saber cuándo un proceso hijo en segundo plano ha terminado. Además de que esto no sería inmediato, por ejemplo, si se estuviera ejecutando otra syscall en el proceso padre, este no se enteraría de la finalización del proceso hijo hasta después de que finalice la ejecución.
Con la notificación inmediata de la finalización de un proceso hijo, el proceso padre puede liberar los recursos asociados con ese proceso de manera oportuna.

