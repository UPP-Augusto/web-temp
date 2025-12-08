/******************************************************************************/
/************** FUNCIÓN PARA CAMBIAR DE ESTADO Y MOSTRAR MSJS *****************/
/******************************************************************************/
void CambiarEstado(int sigEstado) {
  switch (sigEstado) {
    case estadoPrueba:
      log(F("EdoPrueba"), logNoticia);
      break;
    case estadoError:
      log(F("EdoError"), logNoticia);
      break;
    case estadoConfiguracion:
      log(F("EdoConfiguracion"), logNoticia);
      break;
    case estadoConexionWiFi:
      log(F("EdoConexionWiFi"), logNoticia);
      break;
    case estadoConfigMDns:
      log(F("EstadoConfigMDns"), logNoticia);
      break;
    case estadoEspera:
      log(F("EdoEspera"), logNoticia);
      break;
    case estadoSinEstado:
    default:  //Llamado de un Estado que no existe
      log(F("EdoNoDeclarado"), logError);
      sigEstado = estadoError;
  }
  Estado = sigEstado;
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
void EstadoPrueba() {
  if (M3StartWebServer())
    CambiarEstado(estadoEspera);
  else
    CambiarEstado(estadoError);
  /*
    // Espera 5 segundos
    if (millis() - tiempoAnt > 10000) {
    tiempoAnt = millis();

    // Buscamos los dispositivos con el servicio
    // String servidores=M2mDnsDescubrirServicio("Gld-Unicon", "tcp");

    // Obtenemos el primero de ellos
    /*int pos=servidores.indexOf(",");
    if(pos==-1){ // Solo es uno

    }
    IPAddress ip;
    if (!WiFi.hostByName("AugusGamer.local", ip)) { // Get the IP address of the NTP server
      Serial.println("DNS lookup failed. Rebooting.");
    }
    else {
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      Serial.print("IP resuelta por hostByName ");
      Serie.println(ipStr);
    }
    }*/
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
void EstadoError() {
  // Esperar 5 segundos
  if (millis() - tiempoAnt > 4500) {
    tiempoAnt = millis();
    CambiarEstado(estadoError);
  }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void EstadoConfiguracion() {
  A1ConfGeneral();
  A5ConfSpiffs();
  A2ConfLog();
  M2ConfmDNS();
  M3ConfWebServer();
  A3Config();    

  log(F("(Estados)Intentando conexión WiFi..."), logInfo);
  if (wiFiManagerConnectionWiFi()) {
    CambiarEstado(estadoConexionWiFi);
  } else {
    log(F("(Estados)Fallo Al conectar WiFi. Reiniciando..."), logError);
    delay(3000);
    ESP.restart();
  }
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
void EstadoEspera() {
  webServer.handleClient();
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);

  if (millis() - tSegAnt > 30000) {
    segundos++;
    log(F("(EdoEspera)."), logInfo);
    tSegAnt = millis();
  }
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
void EstadoConexionWiFi() {
  wiFiManagerInfo();

  // TODO: Inicializar otros servicios necesarios
  
  CambiarEstado(estadoConfigMDns);
}


void EstadoConfigMDns() {
  // Si se configura correctamente pasa al siguiente estado
  if (M2StartmDNS())
    CambiarEstado(estadoPrueba);
  else
    CambiarEstado(estadoError);
}
