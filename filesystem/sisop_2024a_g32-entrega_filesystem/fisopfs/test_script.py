import os
import subprocess
import sys
import time

# Función para ejecutar un comando y retornar su salida
def run_command(command):
    result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    return result.stdout.decode().strip(), result.stderr.decode().strip()

# Función para imprimir encabezado de prueba
def print_test_header(test_name):
    print(f"\n{'='*10} {test_name} {'='*10}\n")

# Función para imprimir pie de prueba
def print_test_footer():
    print(f"\n{'='*30}\n")

# Función para verificar el resultado esperado
def verify_result(test_name, command, expected_output, actual_output, stderr_output):
    print(f"Corriendo: {command}")
    print(f"Esperado: {expected_output}")
    print(f"Resultado: {actual_output}")
    if actual_output == expected_output:
        print(f"\033[92m✔ {test_name}\033[0m\n")  # Tick verde
        return True
    else:
        print(f"\033[91m✘ {test_name}\033[0m\n")  # Cruz roja
        print(f"Error stderr: {stderr_output}")
        return False

# Test de creación de archivos
def test_creacion_archivos():
    test_name = "Creación de archivos"
    print_test_header(test_name)
    command = "touch testfile && ls -1 testfile"
    expected_output = "testfile"
    stdout, stderr = run_command(command)
    if verify_result(test_name, command, expected_output, stdout, stderr):
        return True
    print_test_footer()
    return False

# Test de creación de directorios
def test_creacion_directorios():
    test_name = "Creación de directorios"
    print_test_header(test_name)
    command = "mkdir testdir && ls -1d testdir"
    expected_output = "testdir"
    stdout, stderr = run_command(command)
    if verify_result(test_name, command, expected_output, stdout, stderr):
        return True
    print_test_footer()
    return False

# Test de lectura de directorios
def test_lectura_directorios():
    test_name = "Lectura de directorios"
    print_test_header(test_name)
    command = "ls -1a"
    expected_output = ".\n..\ntestdir\ntestfile"
    stdout, stderr = run_command(command)
    filtered_stdout = '\n'.join(line for line in stdout.split('\n') if line != 'test')
    if verify_result(test_name, command, expected_output, filtered_stdout, stderr):
        return True
    print_test_footer()
    return False

# Test de escritura y lectura de archivos
def test_escritura_lectura_archivos():
    test_name = "Escritura y Lectura de archivos"
    print_test_header(test_name)
    command = "echo 'nuevo contenido' > testfile && cat testfile"
    expected_output = "nuevo contenido"
    stdout, stderr = run_command(command)
    if verify_result(test_name, command, expected_output, stdout, stderr):
        return True
    print_test_footer()
    return False

# Test de append en archivos
def test_append_archivos():
    test_name = "Append en archivos"
    print_test_header(test_name)
    command = "echo 'mas contenido' >> testfile && cat testfile"
    expected_output = "nuevo contenido\nmas contenido"
    stdout, stderr = run_command(command)
    if verify_result(test_name, command, expected_output, stdout, stderr):
        return True
    print_test_footer()
    return False

# Test de borrado de archivo
def test_borrado_archivo():
    test_name = "Borrado de archivo"
    print_test_header(test_name)
    command = "rm testfile && ls -1 testfile"
    expected_output = ""
    stdout, stderr = run_command(command)
    if verify_result(test_name, command, expected_output, stdout, stderr):
        return True
    print_test_footer()
    return False

# Test de borrado de directorio
def test_borrado_directorio():
    test_name = "Borrado de directorio"
    print_test_header(test_name)
    command = "rmdir testdir && ls -1d testdir"
    expected_output = ""
    stdout, stderr = run_command(command)
    if verify_result(test_name, command, expected_output, stdout, stderr):
        return True
    print_test_footer()
    return False

# Test de stat
def test_stat_archivo():
    test_name = "Stat de archivo"
    print_test_header(test_name)
    command = "stat testfile"
    stdout, stderr = run_command(command)
    if "Size:" in stdout and "File:" in stdout:
        print(f"\033[92m✔ {test_name}\033[0m\n")  # Tick verde
        return True
    else:
        print(f"\033[91m✘ {test_name}\033[0m\n")  # Cruz roja
        print(f"Error stderr: {stderr}")
        return False
    print_test_footer()
    return False

# Test de sobrescribir un archivo
def test_sobrescribir_archivo():
    test_name = "Sobrescribir archivo"
    print_test_header(test_name)
    
    # Leer el contenido actual del archivo
    command_read = "cat testfile"
    current_content, stderr = run_command(command_read)
    if stderr:
        print(f"Error al leer el archivo: {stderr}")
        return False
    
    print(f"Contenido actual de testfile:\n{current_content}\n")

    # Sobrescribir el archivo
    command_overwrite = "echo 'contenido sobrescrito' > testfile && cat testfile"
    expected_output = "contenido sobrescrito"
    stdout, stderr = run_command(command_overwrite)
    if verify_result(test_name, command_overwrite, expected_output, stdout, stderr):
        return True
    print_test_footer()
    return False

# Función principal que ejecuta todos los tests
def main():
    # Ruta del directorio montado => se pasa como argumento
    if len(sys.argv) < 2:
        print("Uso: python script.py <ruta_directorio_montado>")
        sys.exit(1)
    
    mount_dir = sys.argv[1]

    os.chdir(mount_dir)

    print("Iniciando tests...\n")

    total_tests = 9
    passed_tests = 0

    if test_creacion_archivos():
        passed_tests += 1
    
    if test_creacion_directorios():
        passed_tests += 1
    
    if test_lectura_directorios():
        passed_tests += 1
    
    if test_escritura_lectura_archivos():
        passed_tests += 1
    
    if test_append_archivos():
        passed_tests += 1

    if test_stat_archivo():
        passed_tests += 1

    if test_sobrescribir_archivo():
        passed_tests += 1

    if test_borrado_archivo():
        passed_tests += 1
    
    if test_borrado_directorio():
        passed_tests += 1

    print("Finalizaron todos los test.\n")
    print(f"Total tests superados: {passed_tests}/{total_tests}")

if __name__ == "__main__":
    main()
