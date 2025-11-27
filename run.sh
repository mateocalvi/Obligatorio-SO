#!/bin/bash

# Lista de binarios sin extensión
archivos=("cocinero" "mozo" "repostero" "main")

# Compilar los archivos .c correspondientes
for archivo in "${archivos[@]}"; do
    if [ -f "${archivo}.c" ]; then
        gcc "${archivo}.c" -o "$archivo"
        echo "Compilado: ${archivo}.c → $archivo"
    else
        echo "No se encontró ${archivo}.c, se omite."
    fi
done

# Ejecutar main si se compiló correctamente
if [ -f "main" ]; then
    echo "Ejecutando main..."
    ./main
else
    echo "No se pudo ejecutar main: no existe el archivo binario."
fi

# Borrar archivos binarios
for archivo in "${archivos[@]}"; do
    if [ -f "$archivo" ]; then
        rm "$archivo"
        echo "Binario eliminado: $archivo"
    fi
done