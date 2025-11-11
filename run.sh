#!/bin/bash

# Colores
ROJO='\e[31m'
VERDE='\e[32m'
AMARILLO='\e[33m'
CYAN='\e[36m'
RESET='\e[0m'

# Lista de binarios sin extensión
archivos=("cocinero" "mozo" "repostero" "main")

# Intentar borrar cada ejecutable si existe
for archivo in "${archivos[@]}"; do
    if [ -f "$archivo" ]; then
        rm "$archivo"
        echo -e "${ROJO}Eliminado: $archivo${RESET}"
    fi
done

# Compilar los archivos .c correspondientes
for archivo in "${archivos[@]}"; do
    if [ -f "${archivo}.c" ]; then
        gcc "${archivo}.c" -o "$archivo"
        echo -e "${VERDE}Compilado: ${archivo}.c → $archivo${RESET}"
    else
        echo -e "${AMARILLO}No se encontró ${archivo}.c, se omite.${RESET}"
    fi
done

# Ejecutar main si se compiló correctamente
if [ -f "main" ]; then
    echo -e "${CYAN}Ejecutando main...${RESET}"
    ./main
else
    echo -e "${ROJO}No se pudo ejecutar main: no existe el archivo binario.${RESET}"
fi
