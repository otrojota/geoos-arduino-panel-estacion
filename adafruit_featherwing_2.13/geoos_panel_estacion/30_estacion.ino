#define D_WIDTH  250
#define D_HEIGHT 122

#define GxEPD_BLACK EPD_BLACK
#define GxEPD_WHITE EPD_WHITE

long long currentTime, lastSerieTime;
String fmtTime;
String statName;
unsigned long lastUpdate;

bool hayActual = false;
float temperatura, presion, velocidadViento, direccionViento;
String tiempoActual;

const unsigned char gImage_IMG_TEMP[] PROGMEM = {
  // http://javl.github.io/image2cpp/     'temp-32x16', 32x16px
  0x03, 0xe0, 0x41, 0x00, 0x0f, 0xf8, 0x4d, 0xb0, 0x18, 0x0c, 0x4d, 0xb0, 0x33, 0xe6, 0x00, 0x00, 
  0x66, 0x33, 0xff, 0xf8, 0x6c, 0x1b, 0xff, 0xfe, 0x58, 0x08, 0x00, 0x06, 0x58, 0x0f, 0xff, 0xe2, 
  0x58, 0x0f, 0xff, 0xe2, 0x58, 0x0c, 0x00, 0x06, 0x6c, 0x1b, 0xff, 0xfe, 0x66, 0x33, 0xff, 0xf8, 
  0x33, 0xe6, 0x00, 0x00, 0x18, 0x0c, 0x00, 0x00, 0x0f, 0xf8, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00
};
const unsigned char gImage_IMG_PRESION[] PROGMEM = {
  // 'barometer', 32x32px
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 
  0x00, 0x7f, 0xfe, 0x00, 0x01, 0xf8, 0x1f, 0x80, 0x03, 0xc0, 0x03, 0xc0, 0x07, 0x81, 0x81, 0xe0, 
  0x0f, 0x20, 0x04, 0xf0, 0x1c, 0x20, 0x04, 0x38, 0x3c, 0x00, 0x00, 0x3c, 0x38, 0x01, 0x80, 0x1c, 
  0x74, 0x01, 0x80, 0x2e, 0x72, 0x01, 0x80, 0x4e, 0x60, 0x01, 0x80, 0x06, 0xe0, 0x01, 0x80, 0x07, 
  0xe0, 0x01, 0x80, 0x07, 0xe0, 0x01, 0x80, 0x07, 0xec, 0x01, 0x80, 0x37, 0xe0, 0x01, 0x80, 0x07, 
  0xe0, 0x01, 0x80, 0x07, 0xe0, 0x01, 0x80, 0x07, 0x60, 0x03, 0xc0, 0x06, 0x70, 0x03, 0xc0, 0x0e, 
  0x76, 0x03, 0xc0, 0x6e, 0x30, 0x03, 0xc0, 0x0c, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x03, 0xc0, 0x00, 
  0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const unsigned char gImage_IMG_TIEMPO[] PROGMEM = {
  // 'clock-circular-outline (1)', 24x24px
  0x00, 0x00, 0x00, 0x01, 0xff, 0x80, 0x03, 0xc3, 0xc0, 0x0e, 0x00, 0x70, 0x1c, 0x00, 0x38, 0x18, 
  0x00, 0x18, 0x30, 0x00, 0x0c, 0x60, 0x18, 0x06, 0x60, 0x18, 0x06, 0x60, 0x18, 0x06, 0x40, 0x18, 
  0x02, 0x40, 0x18, 0x02, 0x40, 0x08, 0x02, 0x40, 0x0c, 0x02, 0x60, 0x06, 0x06, 0x60, 0x03, 0x06, 
  0x60, 0x01, 0x06, 0x30, 0x00, 0x0c, 0x18, 0x00, 0x18, 0x1c, 0x00, 0x38, 0x0e, 0x00, 0x70, 0x03, 
  0xc3, 0xc0, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00
};


void getCurrentTime() {
  zrepoGET(zrepoURL + "/now");  
}
void getEstacion() {
  zrepoGET(zrepoURL + "/dim/rie.estacion/rows/" + codigoEstacion + "?token=" + zrepoToken);
}

void getTemperaturaYPresion() {
  long long startTime = currentTime - 1000 * 60 * 60 * 24 * 2;
  char qStart[21], qEnd[21];
  sprintf(qStart, "%lld", startTime);
  sprintf(qEnd, "%lld", currentTime + 1000 * 60 * 30);
  String q = zrepoURL + "/data/multi-var/time-serie?startTime=" + qStart + "&endTime=" + qEnd + "&minimize=true&token=" + zrepoToken + "&filter={\"estacion\":\"" + codigoEstacion + "\"}&temporality=30m&variables=[\"rie.temp\",\"rie.presion_atm\"]";
  //Serial.println("Query Temperatura y Presion");
  //Serial.println(q);
  zrepoGET(q);
  if (doc["rie.temp"].size() > 0) {
    int n = doc["rie.temp"].size();
    lastSerieTime = doc["rie.temp"][n - 1]["t"];
    //Serial.println("lastSerieTime"); Serial.println(lastSerieTime);
  } else {
    lastSerieTime = 0;
  }
}

void getResumenActual() {
  long long startTime = currentTime;
  if (lastSerieTime > 0) {
    // lastSerieTime son bloques de 30 minutos.El inicio puede ser 25 minutos a 30 minutos
    startTime = lastSerieTime + 1000 * 60 * 25;
    if (startTime > currentTime) startTime = currentTime;
  }
  long long endTime = startTime + 1000 * 60 * 5;
  //Serial.println("startTime, endTime para resument actual");
  //Serial.println(startTime);
  //Serial.println(endTime);
  char qStart[21], qEnd[21];
  int i=6;
  hayActual = false;
  while (i > 0) {
    sprintf(qStart, "%lld", startTime);
    sprintf(qEnd, "%lld", endTime);

    String q = zrepoURL + "/data/multi-var/period-summary?startTime=" + qStart + "&endTime=" + qEnd + "&token=" + zrepoToken + "&filter={\"estacion\":\"" + codigoEstacion + "\"}&variables=[\"rie.temp\",\"rie.presion_atm\",\"rie.vel_media_viento\",\"rie.dir_viento\"]";
    //Serial.println("Query period");
    //Serial.println(q);
    zrepoGET(q);
    //serializeJson(doc, Serial);
    int n = doc["rie.temp"]["n"].as<int>();
    if (n > 0) {
      hayActual = true;
      temperatura = doc["rie.temp"]["value"].as<float>() / doc["rie.temp"]["n"].as<int>();
      presion = doc["rie.presion_atm"]["value"].as<float>() / doc["rie.presion_atm"]["n"].as<int>();      
      tiempoActual = fmtHour(doc["rie.temp"]["maxTimeLocale"]);
      // Algunas estaciones pueden no reportar viento
      velocidadViento = -9999.0, direccionViento = -9999;      
      if (doc.containsKey("rie.vel_media_viento")) velocidadViento = doc["rie.vel_media_viento"]["value"].as<float>() / doc["rie.vel_media_viento"]["n"].as<int>();
      if (doc.containsKey("rie.dir_viento")) direccionViento = doc["rie.dir_viento"]["value"].as<float>() / doc["rie.dir_viento"]["n"].as<int>();
      // Forzar salida
      i = 0;
    }
    i--;
    startTime -= 1000 * 60 * 5;
    endTime -= 1000 * 60 * 5;
  }  
}

void updateEstacion(bool inicial) {
  if (!inicial) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdate < 60000 * 3 && currentMillis >= lastUpdate) return;
  }
  getCurrentTime();
  currentTime = doc["time"];
  fmtTime = doc["fmtShort"].as<String>();      
  if (inicial) {
    getEstacion();
    statName = doc["name"].as<String>();
  }
  
  display.clearBuffer();
  //display.fillScreen(GxEPD_WHITE);
  drawHeader(0, 0, D_WIDTH, 12);
  getTemperaturaYPresion();
  coord x0 = 0, y0 = 14, w0 = (coord)(D_WIDTH / 2) - 20, h0 = (coord)((D_HEIGHT - 14) / 2);
  drawTempChart(x0, y0, w0, h0);

  coord x1 = x0, y1 = y0 + h0 - 1, w1 = w0, h1 = D_HEIGHT - y1;
  drawPresChart(x1, y1, w1, h1);

  getResumenActual();

  coord x2 = (coord)(w0 + 1), y2 = 14, w2 = (coord)((D_WIDTH - w0)/ 2), h2 = (coord)((D_HEIGHT - 14) / 2);
  drawTemp(x2, y2, w2, h2);

  coord x3 = (coord)(x2 + w2), y3 = 14, w3 = w2, h3 = (coord)((D_HEIGHT - 14) / 2);
  drawPresion(x3, y3, w3, h3);

  coord x4 = x2, y4 = y0 + h0 - 1, w4 = w2, h4 = (coord)((D_HEIGHT - 14) / 2);
  drawViento(x4, y4, w4, h4);

  coord x5 = x3, y5 = y4, w5 = w3, h5 = h4;
  drawHora(x5, y5, w5, h5);
  
  //display.update();  
  display.display();
  lastUpdate = millis();
}

void est_iniciaEstacion() {
  //Serial.println("Inicia Estacion:" + codigoEstacion);
  updateEstacion(true);
}

void loop_estacion() {
  updateEstacion(false);
}

void drawHeader(coord x0, coord y0, coord w, coord h) {
  // https://github.com/olikraus/u8g2/wiki/fntlistall
  //display.fillRect(x0, y0, w, h, GxEPD_WHITE);
  fonts.setFont(u8g2_font_prospero_bold_nbp_tr);
  fonts.setFontMode(1);
  fonts.setFontDirection(0);
  fonts.setForegroundColor(GxEPD_BLACK);
  fonts.setBackgroundColor(GxEPD_WHITE);
  //fonts.setCursor(5,h - 1);
  //fonts.print(fmtTime);
  fonts.setCursor(D_WIDTH - fonts.getUTF8Width(statName.c_str()) - 5, h - 1);
  fonts.print(statName);
  display.drawFastHLine(x0, h, w, GxEPD_BLACK);
}

String fmtDate(JsonArray l) {
  String cero ="0", vacio = "";
  int MM = l[1], DD = l[2], hh=l[3], mm=l[4];
  return (DD<10?cero:vacio) + DD + "/" + (MM<10?cero:vacio) + MM + " " + (hh<10?cero:vacio) + hh + ":" + (mm<10?cero:vacio) + mm;
}
String fmtHour(JsonArray l) {
  String cero ="0", vacio = "";
  int hh=l[3], mm=l[4];
  return (hh<10?cero:vacio) + hh + ":" + (mm<10?cero:vacio) + mm;
}
void drawTempChart(coord x, coord y, coord w, coord h) {
  DynamicJsonDocument data = doc["rie.temp"];
  
  display.fillRect(x, y, w, h, GxEPD_WHITE);
  //display.drawRect(x, y, w, h, GxEPD_BLACK);

  fonts.setFont(u8g2_font_pcsenior_8f);
  fonts.setFontMode(1);
  fonts.setFontDirection(0);
  fonts.setForegroundColor(GxEPD_BLACK);
  fonts.setBackgroundColor(GxEPD_WHITE);

  coord x1 = x + 30, y1 = y + 5, w1 = w - 30 - 5, h1 = h - 2 - 5 - 8;
  String st = "°C";
  fonts.setCursor(x1 + w1/2 -  fonts.getUTF8Width(st.c_str()) / 2, y1 + 8);
  fonts.print(st);

  fonts.setFont(u8g2_font_5x7_tf);

  display.drawFastVLine(x1, y1, h1, GxEPD_BLACK);
  display.drawFastHLine(x1, y1 + h1, w1, GxEPD_BLACK);
  
  if (data.size() == 0) return;

  JsonObject dia = data[0];
  long long t0 = dia["t"];
  JsonArray l = dia["l"];
  fonts.setCursor(x1 + 5, y + h - 2);
  String fmt = fmtHour(l);
  fonts.print(fmt);
  
  dia = data[data.size() - 1];
  long long t1 = dia["t"];
  l = dia["l"];
  st = fmtHour(l);
  fonts.setCursor(x + w - fonts.getUTF8Width(st.c_str()) - 5, y + h - 2);
  fmt = fmtHour(l);
  fonts.print(fmt);  
  
  float min, max;
  for (int i=0; i<data.size(); i++) {
    JsonObject d = data[i];
    float v = d["v"].as<float>() / d["n"].as<int>();
    if (i == 0 || v < min) min = v;
    if (i == 0 || v > max) max = v;    
  }
  int iMin = (int)min, iMax = (int)max + 1;  
  st = String(iMin);
  fonts.setCursor(x1 - fonts.getUTF8Width(st.c_str()) - 5, y1 + h1);
  fonts.print(st);
  st = String(iMax);
  fonts.setCursor(x1 - fonts.getUTF8Width(st.c_str()) - 5, y1 + 7);
  fonts.print(st);

  int xOld, yOld;
  for (int i=0; i<data.size(); i++) {
    JsonObject d = data[i];
    float v = d["v"].as<float>() / d["n"].as<int>();
    int y = y1 + h1 - (int)((v - iMin) / (iMax - iMin) * h1);
    long long t = d["t"];
    int x = x1 + (int)(w1 * (float)(t - t0) / (float)(t1 - t0));
    if (i > 0) {
      display.drawLine(xOld, yOld, x, y, GxEPD_BLACK);
    }
    xOld = x; yOld = y;
  }
}

void drawPresChart(coord x, coord y, coord w, coord h) {
  DynamicJsonDocument data = doc["rie.presion_atm"];
  
  display.fillRect(x, y, w, h, GxEPD_WHITE);
  //display.drawRect(x, y, w, h, GxEPD_BLACK);

  fonts.setFont(u8g2_font_pcsenior_8f);
  fonts.setFontMode(1);
  fonts.setFontDirection(0);
  fonts.setForegroundColor(GxEPD_BLACK);
  fonts.setBackgroundColor(GxEPD_WHITE);

  coord x1 = x + 30, y1 = y + 5, w1 = w - 30 - 5, h1 = h - 2 - 5 - 8;
  String st = "hPa";
  fonts.setCursor(x1 + w1/2 -  fonts.getUTF8Width(st.c_str()) / 2, y1 + 8);
  fonts.print(st);

  fonts.setFont(u8g2_font_5x7_tf);

  display.drawFastVLine(x1, y1, h1, GxEPD_BLACK);
  display.drawFastHLine(x1, y1 + h1, w1, GxEPD_BLACK);

  if (data.size() == 0) return;
  
  JsonObject dia = data[0];
  long long t0 = dia["t"];
  JsonArray l = dia["l"];
  fonts.setCursor(x1 + 5, y + h - 2);
  String fmt = fmtHour(l);
  fonts.print(fmt);
  
  dia = data[data.size() - 1];
  long long t1 = dia["t"];
  l = dia["l"];
  st = fmtHour(l);
  fonts.setCursor(x + w - fonts.getUTF8Width(st.c_str()) - 5, y + h - 2);
  fmt = fmtHour(l);
  fonts.print(fmt);  
  
  float min, max;
  for (int i=0; i<data.size(); i++) {
    JsonObject d = data[i];
    float v = d["v"].as<float>() / d["n"].as<int>();
    if (i == 0 || v < min) min = v;
    if (i == 0 || v > max) max = v;    
  }
  int iMin = (int)min, iMax = (int)max + 1;  
  st = String(iMin);
  fonts.setCursor(x1 - fonts.getUTF8Width(st.c_str()) - 5, y1 + h1);
  fonts.print(st);
  st = String(iMax);
  fonts.setCursor(x1 - fonts.getUTF8Width(st.c_str()) - 5, y1 + 7);
  fonts.print(st);

  int xOld, yOld;
  for (int i=0; i<data.size(); i++) {
    JsonObject d = data[i];
    float v = d["v"].as<float>() / d["n"].as<int>();
    int y = y1 + h1 - (int)((v - iMin) / (iMax - iMin) * h1);
    long long t = d["t"];
    int x = x1 + (int)(w1 * (float)(t - t0) / (float)(t1 - t0));
    if (i > 0) {
      display.drawLine(xOld, yOld, x, y, GxEPD_BLACK);
    }
    xOld = x; yOld = y;
  }
}

void drawTemp(coord x, coord y, coord w, coord h) {
  display.fillRect(x, y, w, h, GxEPD_WHITE);
  display.drawRect(x, y, w, h, GxEPD_BLACK);
  display.drawBitmap(x + 4, y + 6, gImage_IMG_TEMP, 32, 16, GxEPD_BLACK);
  if (!hayActual) return;
  
  fonts.setFont(u8g2_font_t0_22_tf);
  fonts.setFontMode(1);
  fonts.setFontDirection(0);
  fonts.setForegroundColor(GxEPD_BLACK);
  fonts.setBackgroundColor(GxEPD_WHITE);
  String unidad = "°C";
  fonts.setCursor(x + w - fonts.getUTF8Width(unidad.c_str()) - 5, y + 20);
  fonts.print(unidad);

  fonts.setFont(u8g2_font_t0_22b_tn);
  char str[10];
  dtostrf(temperatura, 5, 2, str );
  fonts.setCursor(x + w - fonts.getUTF8Width(str) - 5, y + h - 4);
  fonts.print(str);
}
void drawPresion(coord x, coord y, coord w, coord h) {
  display.fillRect(x, y, w, h, GxEPD_WHITE);
  display.drawRect(x, y, w, h, GxEPD_BLACK);
  display.drawBitmap(x + 4, y + 4, gImage_IMG_PRESION, 32, 16, GxEPD_BLACK);
  if (!hayActual) return;
  
  fonts.setFont(u8g2_font_t0_22_tf);
  fonts.setFontMode(1);
  fonts.setFontDirection(0);
  fonts.setForegroundColor(GxEPD_BLACK);
  fonts.setBackgroundColor(GxEPD_WHITE);
  String unidad = "hPa";
  fonts.setCursor(x + w - fonts.getUTF8Width(unidad.c_str()) - 5, y + 20);
  fonts.print(unidad);

  fonts.setFont(u8g2_font_t0_22b_tn);
  char str[10];
  dtostrf(presion, 6, 1, str );
  fonts.setCursor(x + w - fonts.getUTF8Width(str) - 5, y + h - 4);
  fonts.print(str);
}
void drawViento(coord x, coord y, coord w, coord h) {
  display.fillRect(x, y, w, h, GxEPD_WHITE);
  display.drawRect(x, y, w, h, GxEPD_BLACK);
  coord xc = (coord)(x + w/2), yc = (coord)(y + h/2);
  coord r;
  if (w > h) r = (coord)(h / 2);
  else r = (coord)(w / 2);
  r -= 7;
  //display.drawCircle(xc, yc, r, GxEPD_BLACK);
  display.drawCircle(xc, yc, r-2, GxEPD_BLACK);

  fonts.setFont(u8g2_font_5x8_tf);
  fonts.setFontMode(1);
  fonts.setFontDirection(0);
  fonts.setForegroundColor(GxEPD_BLACK);
  fonts.setBackgroundColor(GxEPD_WHITE);
  
  String st = "N";
  fonts.setCursor(xc - fonts.getUTF8Width(st.c_str()) / 2, yc - r + 1);
  fonts.print(st);
  st = "S";
  fonts.setCursor(xc - fonts.getUTF8Width(st.c_str()) / 2, yc + r + 6);
  fonts.print(st);
  st = "W";
  fonts.setCursor(xc - r - fonts.getUTF8Width(st.c_str()) - 1, yc + 5);
  fonts.print(st);
  st = "E";
  fonts.setCursor(xc + r + 1, yc +5);
  fonts.print(st);

  double ang = PI / 8.0;  
  coord x1 = (coord)(xc + r * cos(ang)), y1 = (coord)(yc + r * sin(ang));
  coord x2 = (coord)(xc + (r + 3) * cos(ang)), y2 = (coord)(yc + (r + 3) * sin(ang));
  display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);  
  ang += PI / 8;
  x1 = (coord)(xc + r * cos(ang)); y1 = (coord)(yc + r * sin(ang));
  x2 = (coord)(xc + (r + 5) * cos(ang)); y2 = (coord)(yc + (r + 5) * sin(ang));
  display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);
  ang += PI / 8;
  x1 = (coord)(xc + r * cos(ang)); y1 = (coord)(yc + r * sin(ang));
  x2 = (coord)(xc + (r + 3) * cos(ang)); y2 = (coord)(yc + (r + 3) * sin(ang));
  display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);

  ang += PI / 8; 
  ang += PI / 8;
  x1 = (coord)(xc + r * cos(ang)); y1 = (coord)(yc + r * sin(ang));
  x2 = (coord)(xc + (r + 3) * cos(ang)); y2 = (coord)(yc + (r + 3) * sin(ang));
  display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);
  ang += PI / 8;
  x1 = (coord)(xc + r * cos(ang)); y1 = (coord)(yc + r * sin(ang));
  x2 = (coord)(xc + (r + 5) * cos(ang)); y2 = (coord)(yc + (r + 5) * sin(ang));
  display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);
  ang += PI / 8;
  x1 = (coord)(xc + r * cos(ang)); y1 = (coord)(yc + r * sin(ang));
  x2 = (coord)(xc + (r + 3) * cos(ang)); y2 = (coord)(yc + (r + 3) * sin(ang));
  display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);

  ang += PI / 8; 
  ang += PI / 8;
  x1 = (coord)(xc + r * cos(ang)); y1 = (coord)(yc + r * sin(ang));
  x2 = (coord)(xc + (r + 3) * cos(ang)); y2 = (coord)(yc + (r + 3) * sin(ang));
  display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);
  ang += PI / 8;
  x1 = (coord)(xc + r * cos(ang)); y1 = (coord)(yc + r * sin(ang));
  x2 = (coord)(xc + (r + 5) * cos(ang)); y2 = (coord)(yc + (r + 5) * sin(ang));
  display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);
  ang += PI / 8;
  x1 = (coord)(xc + r * cos(ang)); y1 = (coord)(yc + r * sin(ang));
  x2 = (coord)(xc + (r + 3) * cos(ang)); y2 = (coord)(yc + (r + 3) * sin(ang));
  display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);

  ang += PI / 8; 
  ang += PI / 8;
  x1 = (coord)(xc + r * cos(ang)); y1 = (coord)(yc + r * sin(ang));
  x2 = (coord)(xc + (r + 3) * cos(ang)); y2 = (coord)(yc + (r + 3) * sin(ang));
  display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);
  ang += PI / 8;
  x1 = (coord)(xc + r * cos(ang)); y1 = (coord)(yc + r * sin(ang));
  x2 = (coord)(xc + (r + 5) * cos(ang)); y2 = (coord)(yc + (r + 5) * sin(ang));
  display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);
  ang += PI / 8;
  x1 = (coord)(xc + r * cos(ang)); y1 = (coord)(yc + r * sin(ang));
  x2 = (coord)(xc + (r + 3) * cos(ang)); y2 = (coord)(yc + (r + 3) * sin(ang));
  display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);

  if (!hayActual) return;
  if (velocidadViento >= 0) {
    String unidad = "kts";
    fonts.setCursor(xc - fonts.getUTF8Width(unidad.c_str()) / 2, yc + 12);
    fonts.print(unidad);
    
    fonts.setFont(u8g2_font_t0_12b_tr);
    char str[10];
    dtostrf(velocidadViento, 3, 1, str );
    fonts.setCursor(xc - fonts.getUTF8Width(str) / 2, yc + 2);
    fonts.print(str);
  }
  if (direccionViento > -900) {
    ang = -PI / 2 + direccionViento * PI / 180.0;
    double delta = PI / 15.0;
    coord x3, y3;
    x1 = (coord)(xc + (r - 5) * cos(ang - delta));  y1 = (coord)(yc + (r - 5) * sin(ang - delta));
    x2 = (coord)(xc + (r + 3) * cos(ang));          y2 = (coord)(yc + (r + 3) * sin(ang));
    x3 = (coord)(xc + (r - 5) * cos(ang + delta));  y3 = (coord)(yc + (r - 5) * sin(ang + delta));
    display.drawTriangle(x1,y1, x2,y2, x3,y3, GxEPD_BLACK);
    display.fillTriangle(x1,y1, x2,y2, x3,y3, GxEPD_BLACK);
    // display.fillCircle(x1, y1, 4, GxEPD_BLACK);
  }
}
void drawHora(coord x, coord y, coord w, coord h) {
  display.fillRect(x, y, w, h, GxEPD_WHITE);
  display.drawRect(x, y, w, h, GxEPD_BLACK);
  display.drawBitmap(x + 4, y + 6, gImage_IMG_TIEMPO, 24, 24, GxEPD_BLACK);
  if (!hayActual) return;
  
  fonts.setFont(u8g2_font_courR08_tr);
  fonts.setFontMode(1);
  fonts.setFontDirection(0);
  fonts.setForegroundColor(GxEPD_BLACK);
  fonts.setBackgroundColor(GxEPD_WHITE);  
  
  String st = "Hora";
  fonts.setCursor(x + w - fonts.getUTF8Width(st.c_str()) - 5, y + 16);
  fonts.print(st);
  st = "Datos";
  fonts.setCursor(x + w - fonts.getUTF8Width(st.c_str()) - 5, y + 24);
  fonts.print(st);
  
  fonts.setFont(u8g2_font_t0_22b_tn);  
  fonts.setCursor(x + w - fonts.getUTF8Width(tiempoActual.c_str()) - 5, y + h - 4);
  fonts.print(tiempoActual);
}
