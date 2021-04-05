/*
 * Se usan elementos desde:
 * https://github.com/esp8266/Arduino/blob/master/libraries/DNSServer/examples/CaptivePortalAdvanced/handleHttp.ino
 * https://github.com/tzapu/WiFiManager
 */

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

#define WFS_INICIAL     0
#define WFS_CONECTANDO  1
#define WFS_ERROR       2
#define WFS_CONECTADO   3

int wifi_status = WFS_INICIAL;
int driver_status = -1;

DNSServer  dnsServer;
//IPAddress apIP(172, 217, 28, 1);
IPAddress apIP(192, 168, 10, 10);
IPAddress netMsk(255, 255, 255, 0);

WebServer  server;
const byte DNS_PORT = 53;

void wifi_setup() {  
  // Buscar si el ap_name está usado y generar uno nuevo
  int n = WiFi.scanNetworks();
  String apName = String(wi_ap_name);
  int existe = 0;
  int tries=0;
  do {
    Serial.println("Buscando wifi:" + apName);
    existe = 0;
    for (int i=0; i<n; i++) {
      String wifiName = WiFi.SSID(i);
      if (apName == wifiName) {existe = 1; break;}
    }
    if (existe) {
      apName = String(wi_ap_name) + "_" + (++tries);
    }
  } while(existe);
  wi_ap_name = apName;
  Serial.println("Usando apName");Serial.println(wi_ap_name);
  driver_status = WiFi.status();
  if (ssid == "_null_") wifi_status = WFS_ERROR;
  else {
    if (pwd != "_null_") {
      WiFi.begin(ssid.c_str(), pwd.c_str());
    } else {
      WiFi.begin(ssid.c_str());
    }
    wifi_status = WFS_CONECTANDO;
    delay(1000);
  }
}

void wifi_loop() {
  int statusChanged = 0;
  int st = WiFi.status();    
  if (st != driver_status) {
    Serial.print("WiFi Status Change:");
    Serial.print(driver_status);
    Serial.print(" => ");
    Serial.println(st);
    driver_status = st;
    statusChanged = 1;
  }
  if (statusChanged) {
    if (wifi_status == WFS_CONECTADO || wifi_status == WFS_CONECTANDO) {
      switch(st) {
        case WL_CONNECTED:
          wifi_status = WFS_CONECTADO;
          break;
        case WL_CONNECT_FAILED:
        case WL_CONNECTION_LOST:
        case WL_DISCONNECTED:
          WiFi.disconnect();
          Serial.println("Error de Conexion");
          Serial.println(st);
          wifi_status = WFS_ERROR;
          break;
        case WL_NO_SSID_AVAIL:
          clearWIFI();
          ESP.restart();
          break; 
      }
    }
  }
  if (estado == ST_CONFIG || estado == ST_ESTACION) {
    server.handleClient();
    dnsServer.processNextRequest();
  }
}

int wifi_conectado() {
  return wifi_status == WFS_CONECTADO;
}
int wifi_error() {
  return wifi_status == WFS_ERROR;
}

void wifi_iniciaConfig() {
  Serial.println("Inicia Config");  
  sc_configure();

  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(wi_ap_name.c_str());
  delay(500); // Without delay I've seen the IP address blank
  Serial.println();
  Serial.print("AP IP address: ");
  Serial.println(apIP);

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/", pg_root);
  server.on("/restart", pg_restart);
  server.on("/wifi", pg_wifi);
  server.on("/wifisave", pg_wifi_save);
  server.on("/stat", pg_stat);
  server.on("/statsave", pg_stat_save);
  server.on("/generate_204", pg_root);
  server.on("/fwlink", pg_root);
  server.onNotFound(pg_not_found);
  server.begin(); // Web server start
  Serial.println("HTTP Web Server iniciado");
}

void wifi_silentConfig() {
  Serial.println("Inicia Silent Config");  

  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(wi_ap_name.c_str());
  delay(500); // Without delay I've seen the IP address blank
  Serial.println();
  Serial.print("AP IP address: ");
  Serial.println(apIP);

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/", pg_root);
  server.on("/restart", pg_restart);
  server.on("/wifi", pg_wifi);
  server.on("/wifisave", pg_wifi_save);
  server.on("/stat", pg_stat);
  server.on("/statsave", pg_stat_save);
  server.on("/generate_204", pg_root);
  server.on("/fwlink", pg_root);
  server.onNotFound(pg_not_found);
  server.begin(); // Web server start
  Serial.println("HTTP Web Server iniciado");
}

const char HTTP_HEADER[] PROGMEM      = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM       = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char HTTP_HEADER_END[] PROGMEM  = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char HTTP_CANCELAR[] PROGMEM    = "<hr style='margin-top: 10px;'/><form action='restart'><button id='cmdCancelar' type='submit'>Cancelar</button></form>";
const char HTTP_END[] PROGMEM         = "</div></body></html>";
const char HTTP_ITEM[] PROGMEM        = "<div><a href='#p' onclick='selectWIFI(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_ROOT_SCRIPT_1[] PROGMEM = "<script>function configWIFI() {document.getElementById('msgScan').style.display='inherit'; document.getElementById('cmdWIFI').style.display='none'; document.getElementById('cmdStat').style.display='none'; setTimeout(_ => document.forms[0].submit(), 10);}</script>";
const char HTTP_ROOT_SCRIPT_2[] PROGMEM = "<script>function configStat() {document.getElementById('msgStat').style.display='inherit'; document.getElementById('cmdStat').style.display='none'; document.getElementById('cmdWIFI').style.display='none'; setTimeout(_ => document.forms[1].submit(), 10);}</script>";
const char HTTP_FORM_WIFI[] PROGMEM   = "<form method='get' action='wifisave'><input id='s' name='s' length=32 placeholder='SSID'><br/><input id='p' name='p' length=64 type='password' placeholder='password'><br/><button type='submit'>Grabar</button></form>";
const char HTTP_WIFI_SCRIPT[] PROGMEM = "<script>function selectWIFI(l) {document.getElementById('s').value=l.innerText||l.textContent; document.getElementById('p').focus();} function save() {document.forms[0].submit();}</script>";
const char HTTP_FORM_STAT[] PROGMEM   = "<form method='get' action='statsave'><input id='e' name='e' type='hidden'></form>";
const char HTTP_STAT_SCRIPT[] PROGMEM = "<script>function selectStat(codigo) {document.getElementById('e').value=codigo; document.forms[0].submit();}</script>";

void pg_root() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  
  String page = FPSTR(HTTP_HEADER);
  page.replace("{v}", "GEOOS");
  page += FPSTR(HTTP_ROOT_SCRIPT_1);
  page += FPSTR(HTTP_ROOT_SCRIPT_2);
  page += FPSTR(HTTP_STYLE);
  page += FPSTR(HTTP_HEADER_END);
  page += "<h1>Configuración</h1><hr />";
  if (wifi_status == WFS_CONECTADO) {
    page += "<p>Actualmente se encuentra conectado a la red ";
    page += ssid;
    page += "</p>";
  } else {
    page += "<p>No está conectado a Internet</p>";
  }
  page += "<form action='wifi'><button id='cmdWIFI' type='submit' onclick='configWIFI()'>Configurar WIFI</button></form>";
  page += "<div id='msgScan' style='display:none;'>Buscando Redes ...</div>";
  if (wifi_status == WFS_CONECTADO) {
    page += "<form action='stat'><button id='cmdStat' type='submit' onclick='configStat()'>Cambiar Estación</button></form>";
    page += "<div id='msgStat' style='display:none;'>Buscando Estaciones ...</div>";
  }

  page += FPSTR(HTTP_CANCELAR);
  page += FPSTR(HTTP_END);

  server.sendHeader("Content-Length", String(page.length()));
  server.send(200, "text/html", page);
}

void pg_restart() {
  ESP.restart();
}

void pg_wifi() {
  Serial.print("Escaneando... ");
  int n = WiFi.scanNetworks();
  Serial.println(n);

  int indices[n];
  for (int i = 0; i < n; i++) {
    indices[i] = i;
  }
  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++) {
      if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
        std::swap(indices[i], indices[j]);
      }
    }
  }
    
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  
  String page = FPSTR(HTTP_HEADER);
  page.replace("{v}", "GEOOS");
  page += FPSTR(HTTP_STYLE);
  page += FPSTR(HTTP_WIFI_SCRIPT);
  page += FPSTR(HTTP_HEADER_END);
  page += "<h1>Conectar a Red</h1><hr />";

  for (int i = 0; i < n; i++) {
    int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));
    String item = FPSTR(HTTP_ITEM);
    String rssiQ;
    rssiQ += quality;
    item.replace("{v}", WiFi.SSID(indices[i]));
    item.replace("{r}", rssiQ);
    if (WiFi.encryptionType(indices[i]) != WIFI_AUTH_OPEN) {
      item.replace("{i}", "l");
    } else {
      item.replace("{i}", "");
    }
    page += item;
    delay(0);
  }

  page += FPSTR(HTTP_FORM_WIFI);
  page += FPSTR(HTTP_CANCELAR);
  page += FPSTR(HTTP_END);

  server.sendHeader("Content-Length", String(page.length()));
  server.send(200, "text/html", page);
}

void pg_wifi_save() {
  String newSSID = server.arg("s");
  String newPWD = server.arg("p");
  if (newSSID.length() == 0) ESP.restart();
  if (newSSID == "clear") clearWIFI();
  else saveWIFI(newSSID, newPWD);
  ESP.restart();
}

void pg_stat() {
  Serial.print("Buscando Estaciones... ");
  getProveedoresEstaciones();
  Serial.println("Proveedores"); Serial.println(docProveedores.size()); Serial.println(docProveedores[0]["name"].as<String>());
  getEstaciones();
  Serial.println("Estaciones"); Serial.println(doc.size());
  
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String page = FPSTR(HTTP_HEADER);
  page.replace("{v}", "GEOOS");
  page += FPSTR(HTTP_STYLE);
  
  page += FPSTR(HTTP_STAT_SCRIPT);
  page += FPSTR(HTTP_HEADER_END);
  page += "<h1>Monitorear Estación</h1><hr />";

  page += "<ul>";
  for (int i = 0; i < docProveedores.size(); i++) {
    DynamicJsonDocument p = docProveedores[i];
    String prov = "<li>" + p["name"].as<String>() + "<ul>";
    int nEstaciones = 0;
    for (int j=0; j<doc.size(); j++) {
      if (doc[j]["proveedor"].as<String>() == p["code"].as<String>()) {
        nEstaciones++;
        prov += "<li><a href='#' onclick='selectStat(\"" + doc[j]["code"].as<String>() + "\")'>" + doc[j]["name"].as<String>() + "</a></li>";
      }
    }
    if (nEstaciones) {
      prov += "</ul></li>";
      page += prov;
    }
    delay(0);
  }
  page += "</ul>";

  page += FPSTR(HTTP_FORM_STAT);
  page += FPSTR(HTTP_CANCELAR);  
  page += FPSTR(HTTP_END);

  server.sendHeader("Content-Length", String(page.length()));
  server.send(200, "text/html", page);
}

void pg_stat_save() {
  String newStat = server.arg("e");
  saveEstacion(newStat);
  ESP.restart();
}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
    Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

void pg_not_found() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");

  for (uint8_t i = 0; i < server.args(); i++) {
    message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n");
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/plain", message);
}

boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}
int getRSSIasQuality(int RSSI) {
  int quality = 0;

  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}
