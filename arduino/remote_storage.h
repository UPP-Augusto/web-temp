#ifndef REMOTE_STORAGE_H
#define REMOTE_STORAGE_H

#include <Arduino.h>

extern String servidor;
extern int puerto;
extern String dispositivoID;

// Prototipos de funciones
bool descargarConfigRemota();
bool enviarDatos(float temperatura);
bool enviarDatos(float temperatura, float humedad);
String obtenerConfiguracion(String device_id);
bool probarConexionServidor();
void obtenerConfiguracionRemota(); 

#endif