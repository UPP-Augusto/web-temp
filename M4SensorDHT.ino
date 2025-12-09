#include "Records.h"

DHT dht(DHTPIN, DHTTYPE);

const long DHT_READ_INTERVAL = 10000;
unsigned long lastDHTReadTime = 0;
float humidity = 0;
float temperature = 0;
Records DHTRecord;

void configureDHTSensor() {
    dht.begin();
}

void readDHTSensor() {
    if(millis() - lastDHTReadTime >= DHT_READ_INTERVAL) {
        lastDHTReadTime = millis();
        humidity = dht.readHumidity();
        temperature = dht.readTemperature();
        log("(Sensor) Lectura T = " + String(temperature) + " C, H = " + String(humidity) + " %", logInfo);
        DHTRecord.setTimestamp(String(millis()));
        DHTRecord.setTemperature(temperature);
        DHTRecord.setHumidity(humidity);
        dataBaseSaveData(DHTRecord);
    }
}