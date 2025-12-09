#include "local_storage.h"

#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// Definir variables globales
String servidor = "";
int puerto = 0;
String dispositivoID = "";

/**
 * Inicializar LittleFS
 */
void A5ConfSpiffs() {
    Serial.println(F("[LocalStorage] Montando LittleFS..."));
    
    if (!LittleFS.begin(true)) { // true = formatear si falla
        Serial.println(F("[LocalStorage] ERROR: Falló montaje y formato"));
        ESP.restart();
    }
    
    Serial.println(F("[LocalStorage] LittleFS montado OK"));
    
    // Mostrar información de almacenamiento
    size_t total = LittleFS.totalBytes();
    size_t usado = LittleFS.usedBytes();
    Serial.printf("[LocalStorage] Total: %u bytes, Usado: %u bytes, Libre: %u bytes\n",
                 total, usado, total - usado);
}

/**
 * Mostrar archivos en LittleFS
 */
void A5InfoSpiffs() {
    Serial.println(F("[LocalStorage] --- Archivos en LittleFS ---"));
    
    File root = LittleFS.open("/");
    if (!root) {
        Serial.println(F("[LocalStorage] ERROR: No se pudo abrir directorio raíz"));
        return;
    }
    
    File file = root.openNextFile();
    int contador = 0;
    
    while (file) {
        contador++;
        Serial.printf("[LocalStorage] Archivo: %s -> %d bytes\n",
                     file.name(), file.size());
        file = root.openNextFile();
    }
    
    if (contador == 0) {
        Serial.println(F("[LocalStorage] (vacío)"));
    }
    
    root.close();
    Serial.println(F("[LocalStorage] ----------------------------"));
}

/**
 * Cargar configuración desde archivo
 */
bool cargarConfig() {
    if (!LittleFS.exists("/config.json")) {
        Serial.println(F("[LocalStorage] No existe config.json -> primera ejecución"));
        return false;
    }
    
    File file = LittleFS.open("/config.json", "r");
    if (!file) {
        Serial.println(F("[LocalStorage] ERROR: No se pudo abrir config.json para lectura"));
        return false;
    }
    
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.printf("[LocalStorage] ERROR parseando JSON: %s\n", error.c_str());
        return false;
    }
    
    // Cargar valores
    servidor = doc["servidor"].as<String>();
    puerto = doc["puerto"] | 80;
    dispositivoID = doc["dispositivoID"].as<String>();
    
    Serial.printf("[LocalStorage] Config cargada -> servidor: %s:%d, ID: %s\n",
                 servidor.c_str(), puerto, dispositivoID.c_str());
    return true;
}

/**
 * Guardar configuración en archivo
 */
bool guardarConfig() {
    StaticJsonDocument<512> doc;
    
    // Guardar valores actuales
    doc["servidor"] = servidor;
    doc["puerto"] = puerto;
    doc["dispositivoID"] = dispositivoID;
    
    // Opcional: guardar timestamp
    doc["ultima_actualizacion"] = millis();
    
    File file = LittleFS.open("/config.json", "w");
    if (!file) {
        Serial.println(F("[LocalStorage] ERROR: No se pudo abrir config.json para escritura"));
        return false;
    }
    
    if (serializeJson(doc, file) == 0) {
        Serial.println(F("[LocalStorage] ERROR: Falló escritura en config.json"));
        file.close();
        return false;
    }
    
    file.close();
    Serial.println(F("[LocalStorage] config.json guardado correctamente"));
    return true;
}


bool formatearAlmacenamiento() {
    Serial.println(F("[LocalStorage] ⚠️  Formateando LittleFS..."));
    
    if (LittleFS.format()) {
        Serial.println(F("[LocalStorage] LittleFS formateado correctamente"));
        return true;
    } else {
        Serial.println(F("[LocalStorage] ERROR: No se pudo formatear LittleFS"));
        return false;
    }
}