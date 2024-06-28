## Autores
Trabajos hechos en conjunto con:

  -[Alejo Gomez](https://github.com/agomezalejo)
  
  -[Julian Mutchinick](https://github.com/julimuchi)
  
  -[Federico Camurri](https://github.com/kmu01)
  

El esqueleto de los trabajos fue proporcionado por la [cátedra Mendez](https://fisop.github.io/website/docentes/) de FIUBA.

## Descripción de los trabajos

### TP1 - Shell
Se desarrolló la funcionalidad mínima que caracteriza a un interprete de comandos shell.

### TP2 - Scheduler 

Implementación del mecanismo de cambio de contexto para procesos y el scheduler (planificador) sobre un sistema operativo preexistente.
El kernel utilizado es una modificación de JOS, un exokernel educativo con licencia libre del grupo de [Sistemas Operativos Distribiudos](https://pdos.csail.mit.edu/) del MIT

### TP3 - Filesystem

Implementacion de un sistema de archivos (o filesystem) para Linux. Este utiliza el mecanismo de [FUSE](https://en.wikipedia.org/wiki/Filesystem_in_Userspace) (Filesystem in USErspace) provisto por el [kernel](https://www.kernel.org/doc/html/latest/filesystems/fuse.html), que permite definir en modo usuario la implementación de un filesystem. Gracias a ello, el mismo tiene la interfaz VFS y puede ser accedido con las syscalls y programas habituales (read, open, ls, etc).

La implementación del filesystem es enteramente en memoria: tanto archivos como directorios son representados mediante estructuras que viven en memoria RAM. Por esta razón, buscamos un sistema de archivos que apunte a la velocidad de acceso, y no al volumen de datos o a la persistencia. Aún así, los datos de nuestro filesystem estan representados en disco por un archivo.
