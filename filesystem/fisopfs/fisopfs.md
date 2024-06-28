# fisop-fs
## Diseño de estructuras

La estructura de datos central de la implementación es el inodo. Este contiene la información pertinente a cada uno de los archivos (es decir, la metadata). Puntualmente, los datos que se almacenan en cada inodo son:
	-Si dicho inodo está libre o no
	-El tipo de elemento que contiene (directorio o archivo)
	-El path 
	-El path del directorio padre
	-La hora en la que se creó y  se modificó y se leyó por última vez
	-Su contenido
	-Estadísticas de usuario

El filesystem consiste en un array de 50 inodos, por lo que la implementación realizada es estática.

## Búsqueda de archivo por path:

Para lograr encontrar un archivo específico dado un path, se implementó la función encontrar_indice() que recibe el path y devuelve el índice que el archivo correspondiente a dicho path ocupa en el array de inodos. Para lograr esto se itera dicho array comparando los paths de todos los inodos con el path recibido.

## Persistencia en disco:

La implementación tiene la particularidad de estar cargada en memoria en vez de en disco. Solamente se interactúa con el disco per se en dos circunstancias específicas: cuando se llama a init() y cuando se llama a destroy().

### Destroy():

Esta función itera el array principal de inodos y escribe la información pertinente a cada uno de ellos en un archivo en .fisopfs que vive en disco y puede ser especificado por el usuario a la hora de correr el programa. De esta forma los archivos quedan guardados en el disco y se logra la persistencia.

### Init():

Esta función lee el archivo .fisopfs donde previamente se había escrito la información del filesystem mediante el uso de Destroy(). Lee la información que había quedado cargada en el disco y restaura los inodos y los agrega nuevamente al array de inodos.


## Pruebas de Sistema de Archivos

## Instrucciones para Ejecutar el Script

1. Montar el sistema de archivos en un directorio con el flag `-f`:
   ```bash
   ./fisopfs -f <directorioDeMontaje>
   ```
   Por ejemplo:
   ```bash
   ./fisopfs -f prueba/
   ```

2. En otra terminal, ejecutar el script desde el mismo directorio `/fisopfs`:
   ```bash
   python3 test_script.py <directorioDeMontaje>
   ```
   Por ejemplo:
   ```bash
   python3 test_script.py ./prueba
   ```

## Descripción de las Pruebas

El script realiza las siguientes pruebas sobre el sistema de archivos:

1. **Creación de archivos**: Verifica que se pueda crear un archivo y que éste aparezca en el listado del directorio.
2. **Creación de directorios**: Verifica que se pueda crear un directorio y que éste aparezca en el listado del directorio.
3. **Lectura de directorios**: Verifica que se pueda listar el contenido de un directorio.
4. **Escritura y lectura de archivos**: Verifica que se pueda escribir en un archivo y luego leer su contenido.
5. **Append en archivos**: Verifica que se pueda añadir contenido a un archivo existente y luego leer el contenido completo.
6. **Borrado de archivo**: Verifica que se pueda borrar un archivo y que éste desaparezca del listado del directorio.
7. **Borrado de directorio**: Verifica que se pueda borrar un directorio y que éste desaparezca del listado del directorio.
8. **Stat de archivo**: Verifica que se pueda obtener información de estado (stat) de un archivo.
9. **Sobrescribir archivo**: Verifica que se pueda sobrescribir el contenido de un archivo existente.

### Nota

El test de persistencia se realizó manualmente y se encuentra documentado en la imagen llamada `PruebaPersistencia`.
 