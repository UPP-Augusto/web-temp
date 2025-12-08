/******************************************************************************/
/*************************** MÓDULO DE SENSOR DHT11 ***************************/
/******************************************************************************/
/** Este módulo obtiene los datos del sensor DHT11 usando la librería        **/
/**                                                                          **/
/******************************************************************************/

/******************************************************************************/
/******************** TERMINALES USADAS POR EL SENSORT DHT11*******************/
/******************************************************************************/

/******************************************************************************/
/************** VARIABLES GLOBALES PARA LA CONEXIÓN POR DEFECTO ***************/
/******************************************************************************/


/******************************************************************************/
/*************** CONFIGURACIÓN INICIAL DE TERMINALES Y VARIABLES **************/
/******************************************************************************/
void M3ConfDHT11() {
  // Configuramos los datos de nuestra red WiFi
  log(F("(DHT11)Sin Configuracion"), logNoticia);
}

void M4ObtenerDatosDHT11(){
  
  int temperature = 0;
  int humidity = 0;
  // Attempt to read the temperature and humidity values from the DHT11 sensor.
  int result = dht11.readTemperatureHumidity(temperature, humidity);
  
  long int timestamp=millis();

  // crear el dato en formato JSON
  String json = "{ \"Temperatura\": "+String(temperature)+", \"Humedad\": "+String(humidity)+", \"Tiempo\": "+String(timestamp)+" }";

  if (result == 0) {
      Serial.print(json);
  } else {
      // Print error message based on the error code.
      Serial.println(DHT11::getErrorString(result));
  }
}