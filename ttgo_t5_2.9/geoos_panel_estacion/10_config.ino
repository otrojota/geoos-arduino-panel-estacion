#include "EEPROM.h"

String zrepoURL   = "https://zrepo.geoos.org";
//String zrepoURL   = "http://192.168.0.17:8096";
String zrepoToken = "geoos-public";
DynamicJsonDocument doc(1024 * 80);
DynamicJsonDocument docProveedores(1024 * 10);

void zrepoGET(String url) {
  HTTPClient http;
  http.useHTTP10(true);
  http.begin(url);
  int code = http.GET();
  if (code != 200) {
    Serial.print("HTTP Error: ");  
    Serial.println(code);
    String st = http.getString();
    Serial.println(st);
    delay(10000);
    return;
  }
  deserializeJson(doc, http.getStream());
  http.end();
}


void getProveedoresEstaciones() {
  zrepoGET(zrepoURL + "/dim/rie.proveedor/rows?token=" + zrepoToken + "&columns=[]");
  String st="";
  serializeJson(doc, st);
  deserializeJson(docProveedores, st);
}

void getEstaciones() {
  zrepoGET(zrepoURL + "/dim/rie.estacion/rows?token=" + zrepoToken + "&filter={\"tipo\":\"meteo\"}&columns=[\"proveedor\"]");
}

void cfg_setup() {
  EEPROM.begin(512);
}

void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0'; // the character may appear in a weird way, you should read: 'only one backslash and 0'
  return String(data);
}

int tieneRed() {
  String marca = readStringFromEEPROM(0);
  return marca == "GEO";
}
String getWiFiSSID() {
  if (!tieneRed()) return "_null_";
  return readStringFromEEPROM(10); // Max 64
}
String getWiFiPwd() {
  if (!tieneRed()) return "_null_";
  return readStringFromEEPROM(76); // Max 64
}

int tieneEstacion() {
  String marca = readStringFromEEPROM(4);
  return marca == "GEO";
}
String getCodigoEstacion() {
  if (!tieneEstacion()) return "valparaiso";
  return readStringFromEEPROM(142); // Max 40
}

void saveWIFI(String newSSID, String newPWD) {
   writeStringToEEPROM(0, "GEO");
   writeStringToEEPROM(10, newSSID);
   writeStringToEEPROM(76, newPWD);
   EEPROM.commit();
}
void saveEstacion(String codigo) {
   writeStringToEEPROM(4, "GEO");
   writeStringToEEPROM(142, codigo);
   EEPROM.commit();
}

void clearWIFI() {
   writeStringToEEPROM(0, "   ");
   writeStringToEEPROM(10, "X");
   writeStringToEEPROM(76, "X");
   EEPROM.commit();
}
