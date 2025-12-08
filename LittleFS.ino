/******************************************************************************/
/******************** MÓDULO DE SISTEMA DE ARCHIVOS LittleFS ********************/
/******************************************************************************/
void A5ConfSpiffs(){
  log(F("(LittleFS)Configurando"), logInfo);
  // Monta el sistema de archivos
  if (!LittleFS.begin()) {
    log(F("(LittleFS)Error montando el sistema de archivos"), logError);
    ESP.restart();
    return;
  }
  else
    log(F("(LittleFS)Se ha Montado el sistema de archivos flash"), logNoticia);
}

void A5InfoSpiifs() {
  Serie.println(F("------SISTEMA ARCHIVOS------"));
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  while(file){
      String fileName = file.name();
      size_t fileSize = file.size();
      Serie.printf("!FILE:%s, SIZE:%s\n", fileName.c_str(), formatBytes(fileSize).c_str());
      file = root.openNextFile();
  }
}
