# Sistema de Gestión de Cantina - Obligatorio S.O.

Simulación de una cantina con múltiples procesos (cocineros, repostero y mozos) que se comunican mediante memoria compartida y semáforos POSIX.

## 📋 Requisitos Previos

- Sistema operativo Linux (Ubuntu, Debian, o similar)
- Acceso a terminal con permisos de administrador (sudo)

## 🔧 Instalación de Herramientas de Compilación

### Paso 1: Actualizar el sistema
```sh
sudo apt update
```

### Paso 2: Instalar build-essential

El paquete `build-essential` incluye las herramientas necesarias para compilar programas en C
```sh
sudo apt install build-essential -y
```

### Paso 3: Verificar la instalación

Verificar que GCC se instaló correctamente:
```sh
gcc --version
```

Salida esperada:
`gcc (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0`

## 📁 Estructura del Proyecto
```
proyecto/
├── constantes.h # Archivo de cabecera con constantes globales
├── main.c # Programa principal
├── cocinero.c # Proceso cocinero
├── repostero.c # Proceso repostero
├── mozo.c # Proceso mozo
└── run.sh # Script de compilación y ejecución
```

## 🚀 Compilación y Ejecución

### Opción 1: Usando el script run.sh (Recomendado)

#### Paso 1: Darle permisos de ejecución al script
```sh
chmod +x run.sh
```
#### Paso 2: Ejecutar el script
```sh
./run.sh
```
El script automáticamente:
1. Limpia compilaciones anteriores
2. Compila todos los archivos `.c`
3. Ejecuta el programa principal

### Opción 2: Compilación manual

Si prefieres compilar manualmente, ejecuta los siguientes comandos:

Compila cada archivo
```sh
gcc -o cocinero cocinero.c -lrt -lpthread
gcc -o repostero repostero.c -lrt -lpthread
gcc -o mozo mozo.c -lrt -lpthread
gcc -o main main.c -lrt -lpthread
```
Ejecuta el programa
```sh
./main
```
**Nota:** Las banderas `-lrt` y `-lpthread` son necesarias para enlazar las librerías utilizadas.

## 💻 Uso del Programa

1. Al ejecutar el programa, te pedirá ingresar la cantidad de pedidos a entregar:
```
====================================================
   SISTEMA DE GESTIÓN DE "LA ALBONDIGA EMBRUJADA"
====================================================

Ingresa la cantidad de pedidos a entregar: 10
```
2. El sistema va a iniciar 9 procesos:
   - 3 Cocineros (generan platos)
   - 1 Repostero (genera postres)
   - 5 Mozos (retiran y entregan pedidos)

3. Vas a ver la salida en tiempo real con colores:
   - 🍳 **Azul**: Cocineros y platos
   - 🍰 **Magenta**: Repostero y postres
   - 🍽️ **Verde**: Mozos y entregas
   - **Naranja**: Pedidos restantes

4. El programa va a terminar automáticamente cuando se completen todos los pedidos.

## 🎨 Características

- **Memoria compartida POSIX**: Comunicación eficiente entre procesos
- **Semáforos**: Sincronización sin condiciones de carrera
- **Sin deadlocks**: Diseño que evita bloqueos mutuos
- **Terminación coordinada**: Usando barreras con semáforos (sin espera activa)
- **Salida colorizada**: Identificación visual de cada tipo de proceso


## 👨‍💻 Desarrollo

Para modificar el comportamiento del sistema, edita las constantes en `constantes.h`:
```c
const int MAX_PLATOS_MOSTRADOR = 27;
const int MAX_POSTRES_HELADERA = 25;
const int NUM_COCINEROS = 3;
const int NUM_MOZOS = 5;
```
