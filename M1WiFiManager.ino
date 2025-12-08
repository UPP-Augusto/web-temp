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

bool wiFiManagerConnectionWiFi() {
  wiFiManagerConfiguration();
  if (!wiFiManager.autoConnect(accessPointNetworkName.c_str(), accessPointPassword.c_str())) {
    log(F("(WiFiManager)No se pudo conectar a la red"), logError);
    return false;
  }
  log(F("(WiFiManager)Conectado a la red"), logNoticia);
  return true;
}

void wiFiManagerResetCredentials() {
  log(F("(WiFiManager)Reestableciendo credenciales WiFi..."), logAdvertencia);
  wiFiManager.resetSettings();
}

void  wiFiManagerInfo() {
  Serie.println(F("!SSID: "));
  Serie.println(WiFi.SSID());
  Serie.println(F("!IP: "));
  Serie.println(WiFi.localIP());
  Serie.println(F("!MAC: "));
  Serie.println(WiFi.macAddress());
  Serie.printf("!StatusWiFi: %d\n", WiFi.status());
  if (Datos.verbosidad >= logDebug) {
    // Imprime información de debug
    WiFi.printDiag(SerialLog);
  }
  Serie.println(F("!Señal: "));
  Serie.println(WiFi.RSSI());
}
