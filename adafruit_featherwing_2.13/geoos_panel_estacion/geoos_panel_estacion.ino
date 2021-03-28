#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <HTTPClient.h>

#define ST_INICIANDO  0
#define ST_CONFIG     1
#define ST_ESTACION   2

int estado = ST_INICIANDO, estadoAnterior = -1;
String ssid, pwd, codigoEstacion;
String wi_ap_name = "GEOOS_Config";
const char *myHostname = "estacion-geoos";

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println("Iniciando...");
  sc_setup();  
  sc_iniciando();
  
  cfg_setup();
  ssid = getWiFiSSID();
  pwd  = getWiFiPwd();
  codigoEstacion = getCodigoEstacion();
  Serial.println(ssid);
  Serial.println(pwd);
  Serial.println(codigoEstacion);
  wifi_setup();
}

void loop() {
  wifi_loop();
  if (estado == ST_INICIANDO) {
    if (wifi_error()) {wifi_iniciaConfig(); estado = ST_CONFIG;}
    else if (wifi_conectado()) {
      est_iniciaEstacion(); 
      wifi_silentConfig();
      estado = ST_ESTACION;
    }
  } else if (estado == ST_ESTACION) {
    loop_estacion();    
  }
  
  if (estado != estadoAnterior) {
    Serial.print("Estado:"); Serial.println(estado);
    estadoAnterior = estado;
  }
}
