# proyecto_1_SO
Proyecto 1 Sistemas operativos
Integrantes:
Diego Ignacio Pérez Torres
Antonia Renata Montero López
Diego Matus Salas
Roberto Cruz Pinto

Comandos para la compilación y ejecución de comandos:

El archivo test_shell.c es el código de la shell de Linux desarrollada, para ejecutarla:
gcc -o test_shell test_shell.c   //compilar
./test_shell                     //ejecutar

Para la primera parte de la tarea, la shell funciona al igual que la shell integrada de Linux, es decir, funcionan los mismos comandos y las pipes se realizan de la misma forma.
Podemos ejecutar comandos como: ls, ps, ps aux, wc -l, top, touch, cat, mkdir, rmdir, rm.
Mientras que las pipes pueden ser: ps -aux | sort -nr -k 4 | head -20; cat archivo.txt | wc -l; ls -al | wc -l; 
Para finalizar la ejecución de la shell, se debe escribir: exit

Para la segunda parte de la tarea, se tomaron en consideración los requisitos para la creación de un comando personalizado de favs. Los comandos posibles son:
favs crear /ruta/carpeta/archivo.txt: crea un archivo para guardar comandos favoritos. Debe otorgarse el nombre del archivo, y opcionalmente la ruta.
favs mostrar: despliega los comandos favoritos existentes
favs eliminar1, eliminar2,...: elimina comandos favoritos asociados a su número identificador. Los números deben estar separados por comas, sin espacios.
favs buscar cmd: busca y despliega los comandos favoritos que contengan el substring cmd
favs borrar: borra todos los comandos en la lista de favoritos
favs num ejecutar: ejecuta el comando asociado a su número identificador que está guardado en la lista de favoritos
favs cargar /ruta/carpeta/archivo.txt: carga el archivo con comandos favoritos guardados y los despliega en pantalla. Debe otorgarse la ruta donde está el archivo
favs guardar: guarda los comandos agregados a favoritos en sesión actual.

Para la creación de un recordatorio personalizado:
set recordatorio cant_segundos mensaje: setea un recordatorio con mensaje personalizado que se ejecutará después de la cantidad de segundos deseados. Todos los parámetros son necesarios, siendo cant_segundos la cantidad de segundos antes que se despliegue el mensaje, y mensaje siendo el mensaje personalizado.

