DHT dht(DHTPIN, DHTTYPE);

const long DHT_READ_INTERVAL = 10000;
unsigned long lastDHTReadTime = 0;
float humidity = 0;
float temperature = 0;

void configureDHTSensor() {
    dht.begin();
}

void readDHTSensor() {
    if(millis() - lastDHTReadTime >= DHT_READ_INTERVAL) {
        lastDHTReadTime = millis();
        humidity = dht.readHumidity();
        temperature = dht.readTemperature();
        log("(Sensor) Lectura T = " + String(temperature) + " C, H = " + String(humidity) + " %", logInfo);
    }
}