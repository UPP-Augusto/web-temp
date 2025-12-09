#ifndef LOCAL_STORAGE_H
#define LOCAL_STORAGE_H

#include <Arduino.h>

extern String servidor;       // IP o dominio del servidor
extern int puerto;            // puerto del servidor
extern String dispositivoID;  // MAC sin dos puntos

// Funciones de LittleFS
void A5ConfSpiffs();          // montar LittleFS
void A5InfoSpiffs();          // mostrar archivos (debug)

// Gestión de config.json
bool cargarConfig();
bool guardarConfig();
bool formatearAlmacenamiento(); // Para emergencias

#endif