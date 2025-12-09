#ifndef RECORDS_H
#define RECORDS_H

class Records {
    private:
        String timestamp;
        float temperature;
        float humidity;
    public:
        Records() { this->timestamp = ""; this->temperature = 0; this->humidity = 0;}
        String getTimestamp() { return String(timestamp); }
        String getTemperature() { return String(temperature); }
        String getHumidity() { return String(humidity); }
        void setTimestamp(String timestamp) { this->timestamp = timestamp; }
        void setTemperature(float temperature) { this->temperature = temperature; }
        void setHumidity(float humidity) { this->humidity = humidity; }
};

#endif