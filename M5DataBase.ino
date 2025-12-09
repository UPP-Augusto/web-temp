#include "Records.h"

const String dataBasePath = "/sensor_log.csv";

bool isFileExist() {
    return LittleFS.exists(dataBasePath);
}

void dataBaseCreateFile() {
    File dataBaseFile = LittleFS.open(dataBasePath, "w");
    if (dataBaseFile) {
        dataBaseFile.println("timestamp,temperature,humidity");
        dataBaseFile.close();
    } 
}

void dataBaseSaveData(Records record) {
    if (!isFileExist()) dataBaseCreateFile();
    File dataBaseFile = LittleFS.open(dataBasePath, "a");
    
    if (dataBaseFile) {
        dataBaseFile.println(record.getTimestamp() + "," + String(record.getTemperature()) + "," + String(record.getHumidity()));
        dataBaseFile.close();
        log(F("(DataBase) Dato guardado"), logInfo);
    } else {
        log(F("(DataBase) Error abriendo para guardar"), logError);
    }
}

void dataBaseReadData() {
    File dataBaseFile = LittleFS.open(dataBasePath, "r");
    if (dataBaseFile) {
        while (dataBaseFile.available()) {
            String line = dataBaseFile.readStringUntil('\n');
            log("(DataBase) Leyendo datos: " + line, logInfo);
        }
        dataBaseFile.close();
    }
}

void dataBaseReset() {
    dataBaseCreateFile();
    log(F("(DataBase) Historial vaciado"), logInfo);
}