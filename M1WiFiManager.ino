const String passwordAccessPoint = "12345678";
WiFiManager wiFiManager;

void wiFiManagerConfiguration() {
  wiFiManager.setDebugOutput(false);
  wiFiManager.setAPCallback(wiFiManagerAccessPoint);
  wiFiManager.setSaveConfigCallback(wiFiManagerSuccessConfiguration);
  wiFiManager.setConfigPortalTimeout(180);
}

void wiFiManagerAccessPoint(WiFiManager *myWiFiManager) {
  log(F("(WiFiManager)Modo Access Point iniciado"), logNoticia);
  Serie.println(F("Conectese a la red: "));
  Serie.println(myWiFiManager->getConfigPortalSSID());
  Serie.println(F("Para acceder a la configuración accede a: "));
  Serie.println(WiFi.softAPIP());
}

void wiFiManagerSuccessConfiguration() {
  log(F("(WiFiManager)Datos guardados correctamente"), logNoticia);
  Serie.println(F("Reiniciaando o conectando..."));
}

void wiFiManagerConnectionWiFi() {

}

void wiFiManagerResetCredentials() {

}

void  wiFiManagerInfo() {

}
