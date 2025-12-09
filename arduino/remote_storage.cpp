#include "remote_storage.h"
#include "local_storage.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Configuración de debug
#define DEBUG_REMOTE 1

#if DEBUG_REMOTE
  #define REMOTE_PRINT(...) Serial.print(__VA_ARGS__)
  #define REMOTE_PRINTLN(...) Serial.println(__VA_ARGS__)
  #define REMOTE_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  #define REMOTE_PRINT(...)
  #define REMOTE_PRINTLN(...)
  #define REMOTE_PRINTF(...)
#endif

/**
 * Obtener configuración desde el servidor
 */
String obtenerConfiguracion(String device_id) {
    if (WiFi.status() != WL_CONNECTED) {
        REMOTE_PRINTLN(F("[Remote] WiFi no conectado"));
        return "";
    }
    
    HTTPClient http;
    
    // Construir URL correctamente
    String url = "http://" + servidor + ":" + String(puerto) + 
                 "/web-temp/api/config.php?device_id=" + device_id;
    
    REMOTE_PRINTF("[Remote] Consultando: %s\n", url.c_str());
    
    http.begin(url);
    http.setTimeout(5000); // 5 segundos timeout
    
    int httpCode = http.GET();
    String respuesta = "";
    
    if (httpCode == HTTP_CODE_OK) {
        respuesta = http.getString();
        REMOTE_PRINTF("[Remote] Respuesta: %s\n", respuesta.c_str());
    } else {
        REMOTE_PRINTF("[Remote] Error HTTP: %d\n", httpCode);
        if (httpCode == HTTPC_ERROR_CONNECTION_FAILED) {
            REMOTE_PRINTLN(F("[Remote] Error de conexión - verifica IP del servidor"));
        }
    }
    
    http.end();
    return respuesta;
}

void obtenerConfiguracionRemota() {
    REMOTE_PRINTLN(F("[Remote] Obteniendo configuración remota..."));
    descargarConfigRemota();
}


bool descargarConfigRemota() {
    REMOTE_PRINTLN(F("[Remote] Descargando configuración remota..."));
    
    String configJson = obtenerConfiguracion(dispositivoID);
    
    if (configJson.isEmpty()) {
        REMOTE_PRINTLN(F("[Remote] No se pudo obtener configuración remota"));
        return false;
    }
    
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, configJson);
    
    if (error) {
        REMOTE_PRINTF("[Remote] Error parseando JSON: %s\n", error.c_str());
        return false;
    }
    
    bool actualizado = false;
    
   
    REMOTE_PRINTLN(F("[Remote] Configuración remota procesada"));
    return actualizado;
}

/**
 * Enviar datos al servidor (solo temperatura)
 */
bool enviarDatos(float temperatura) {
    return enviarDatos(temperatura, -999.0); // Valor especial para "sin humedad"
}

/**
 * Enviar datos al servidor (temperatura y humedad opcional)
 */
bool enviarDatos(float temperatura, float humedad) {
    if (WiFi.status() != WL_CONNECTED) {
        REMOTE_PRINTLN(F("[Remote] WiFi desconectado - No se puede enviar"));
        return false;
    }
    
    HTTPClient http;
    String url = "http://" + servidor + ":" + String(puerto) + "/web-temp/api/datos.php";
    
    REMOTE_PRINTF("[Remote] Enviando a: %s\n", url.c_str());
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000);
    
    // Crear JSON
    StaticJsonDocument<256> doc;
    doc["device_id"] = dispositivoID;
    doc["temperatura"] = temperatura;
    
    // Solo incluir humedad si es un valor válido (no -999.0)
    if (humedad > -900.0) {
        doc["humedad"] = humedad;
    }
    
    doc["timestamp"] = millis();
    doc["rssi"] = WiFi.RSSI(); // Señal WiFi
    
    String jsonData;
    serializeJson(doc, jsonData);
    
    REMOTE_PRINTF("[Remote] Datos: %s\n", jsonData.c_str());
    
    int httpCode = http.POST(jsonData);
    bool exito = false;
    
    if (httpCode == HTTP_CODE_OK) {
        String respuesta = http.getString();
        REMOTE_PRINTF("[Remote] OK: %s\n", respuesta.c_str());
        exito = true;
    } else {
        REMOTE_PRINTF("[Remote] Error HTTP: %d\n", httpCode);
        
        // Intentos de recuperación
        if (httpCode == HTTPC_ERROR_CONNECTION_FAILED) {
            REMOTE_PRINTLN(F("[Remote] Servidor no responde - verificar conexión"));
        }
    }
    
    http.end();
    return exito;
}

/**
 * Probar conexión con el servidor
 */
bool probarConexionServidor() {
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }
    
    HTTPClient http;
    String url = "http://" + servidor + ":" + String(puerto) + 
                 "/web-temp/api/config.php?device_id=test_conexion";
    
    http.begin(url);
    http.setTimeout(2000);
    
    int httpCode = http.GET();
    http.end();
    
    REMOTE_PRINTF("[Remote] Test conexión: HTTP %d\n", httpCode);
    
    // Cualquier respuesta del servidor indica que está vivo
    return (httpCode > 0);
}