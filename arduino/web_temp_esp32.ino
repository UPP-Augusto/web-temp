#include <WiFi.h>
#include <Arduino.h>
#include "local_storage.h"
#include "remote_storage.h"

const char* WIFI_SSID = "NombreDeTuRed";      // Cambiar
const char* WIFI_PASSWORD = "TuPassword";     // Cambiar

unsigned long ultimoEnvio = 0;
unsigned long ultimaReconexion = 0;
int intervaloEnvio = 30;          // Segundos entre envíos
float tempAlerta = 35.0;          // Temperatura de alerta
bool sistemaListo = false;

void conectarWiFi();
void manejarConexionWiFi();
void enviarDatosPeriodicamente();
void mostrarEstado();

void setup() {
    Serial.begin(115200);
    delay(2000); // Esperar estabilización USB
    
    Serial.println(F("\n======================================"));
    Serial.println(F("    WEB TEMP - SISTEMA ESP32"));
    Serial.println(F("    Base de Datos Integrada"));
    Serial.println(F("======================================\n"));
    
    // 1. Inicializar sistema de archivos
    Serial.println(F("[Sistema] Inicializando almacenamiento..."));
    A5ConfSpiffs();
    A5InfoSpiffs(); // Opcional: ver archivos
    
    // 2. Cargar configuración local
    Serial.println(F("[Sistema] Cargando configuración..."));
    if (cargarConfig()) {
        Serial.println(F("[Sistema] Configuración local cargada"));
    } else {
        Serial.println(F("[Sistema] Configuración por defecto"));
        
        // Configurar valores iniciales
        servidor = "192.168.1.100"; // Cambiar por IP del servidor del proyecto
        puerto = 80;
        
        if (guardarConfig()) {
            Serial.println(F("[Sistema] Configuración guardada"));
        }
    }
    
    // 3. Conectar WiFi
    conectarWiFi();
    
    // 4. Si hay WiFi, probar servidor
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("[Sistema] Probando servidor..."));
        if (probarConexionServidor()) {
            Serial.println(F("[Sistema] Servidor disponible ✓"));
            
            // Obtener configuración remota
            Serial.println(F("[Sistema] Obteniendo configuración remota..."));
            obtenerConfiguracionRemota();
        } else {
            Serial.println(F("[Sistema] Servidor no disponible ✗"));
            Serial.println(F("[Sistema] Usando configuración local"));
        }
    }
    
    sistemaListo = true;
    Serial.println(F("\n[Sistema] LISTO para operar"));
    Serial.println(F("======================================\n"));
}

void loop() {
    // Manejar reconexión WiFi periódica
    manejarConexionWiFi();
    
    // Enviar datos periódicamente
    enviarDatosPeriodicamente();
    
    // Mostrar estado cada 60 segundos
    static unsigned long ultimoEstado = 0;
    if (millis() - ultimoEstado > 60000) {
        mostrarEstado();
        ultimoEstado = millis();
    }
    
    delay(1000); // Ciclo de 1 segundo
}

void conectarWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }
    
    Serial.print(F("[WiFi] Conectando a: "));
    Serial.println(WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.setAutoReconnect(true);
    
    unsigned long inicio = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - inicio < 15000) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("\n[WiFi] Conectado ✓"));
        Serial.printf("  IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("  MAC: %s\n", WiFi.macAddress().c_str());
        
        // Actualizar dispositivoID si está vacío
        if (dispositivoID.isEmpty()) {
            dispositivoID = WiFi.macAddress();
            dispositivoID.replace(":", "");
            dispositivoID.toLowerCase();
            guardarConfig();
            Serial.printf("  ID generado: %s\n", dispositivoID.c_str());
        }
    } else {
        Serial.println(F("\n[WiFi] ERROR: No se pudo conectar"));
    }
    
    ultimaReconexion = millis();
}

void manejarConexionWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }
    
    // Intentar reconexión cada 30 segundos
    if (millis() - ultimaReconexion > 30000) {
        Serial.println(F("[WiFi] Intentando reconexión..."));
        WiFi.disconnect();
        delay(1000);
        conectarWiFi();
    }
}

void enviarDatosPeriodicamente() {
    if (millis() - ultimoEnvio < intervaloEnvio * 1000) {
        return;
    }
    
    // SIMULAR LECTURA DE SENSORES (reemplazar con sensores reales)
    float temperatura = 25.0 + (random(-20, 20) / 10.0); // 23-27°C
    float humedad = 60.0 + (random(-15, 15) / 10.0);     // 58.5-61.5%
    
    Serial.printf("\n[Sensor] Lectura: %.1f°C, %.1f%% HR\n", temperatura, humedad);
    
    // Verificar alerta
    if (temperatura > tempAlerta) {
        Serial.printf("[ALERTA] Temperatura alta: %.1f°C > %.1f°C\n", 
                     temperatura, tempAlerta);
    }
    
    // Enviar datos si hay WiFi
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("[Envío] Enviando al servidor..."));
        
        if (enviarDatos(temperatura, humedad)) {
            Serial.println(F("[Envío] Envío exitoso ✓"));
        } else {
            Serial.println(F("[Envío] Falló el envío ✗"));
        }
    } else {
        Serial.println(F("[Envío] WiFi desconectado - No se puede enviar"));
        // Aquí podrías guardar en buffer local para enviar después
    }
    
    ultimoEnvio = millis();
}

void mostrarEstado() {
    Serial.println(F("\n--- ESTADO DEL SISTEMA ---"));
    Serial.printf("Tiempo activo: %lu minutos\n", millis() / 60000);
    Serial.printf("WiFi: %s\n", WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado");
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("  Señal: %d dBm\n", WiFi.RSSI());
        Serial.printf("  IP: %s\n", WiFi.localIP().toString().c_str());
    }
    
    Serial.printf("ID Dispositivo: %s\n", dispositivoID.c_str());
    Serial.printf("Servidor: %s:%d\n", servidor.c_str(), puerto);
    Serial.printf("Intervalo: %d segundos\n", intervaloEnvio);
    Serial.printf("Temp. alerta: %.1f°C\n", tempAlerta);
    Serial.printf("Último envío: %lu segundos\n", (millis() - ultimoEnvio) / 1000);
    Serial.println(F("----------------------------"));
}
