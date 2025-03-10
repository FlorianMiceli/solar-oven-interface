#include <WiFi.h>
#include <WebServer.h>

// Broches des photodiodes
#define PHOTODIODE_HAUT 4  // Photodiode haute (GPIO32)
#define PHOTODIODE_BAS 5   // Photodiode basse (GPIO33)

const char* ssid = "ESP32-S3_AP";
const char* password = "motdepasse123";
WebServer server(80);

void setup() {
  Serial.begin(115200);

  pinMode(PHOTODIODE_HAUT, INPUT);
  pinMode(PHOTODIODE_BAS, INPUT);
  
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Adresse IP du point d'accès: ");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/lirePhotodiodes", HTTP_GET, []() {
    server.send(200, "text/plain", lirePhotodiodes());
  });
  server.begin();
  Serial.println("Serveur HTTP démarré");
}

void loop() {
  server.handleClient();

}

void handleRoot() {
  String html = "<html><head>";
  html += "<script>";
  html += "function updateValues() {";
  html += "  var xhttp = new XMLHttpRequest();";
  html += "  xhttp.onreadystatechange = function() {";
  html += "    if (this.readyState == 4 && this.status == 200) {";
  html += "      var values = this.responseText.split(',');";
  html += "      document.getElementById('tensionHaut').innerHTML = values[0];";
  html += "      document.getElementById('tensionBas').innerHTML = values[1];";
  html += "    }";
  html += "  };";
  html += "  xhttp.open('GET', '/lirePhotodiodes', true);";
  html += "  xhttp.send();";
  html += "}";
  html += "setInterval(updateValues, 1000);";
  html += "</script>";
  html += "</head><body>";
  html += "<h1>Tensions des photodiodes</h1>";
  html += "<p>Photodiode haute : <span id='tensionHaut'></span> V</p>";
  html += "<p>Photodiode basse : <span id='tensionBas'></span> V</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

String lirePhotodiodes() {
  int valeurHaut = analogRead(PHOTODIODE_HAUT);
  int valeurBas = analogRead(PHOTODIODE_BAS);
  float tensionHaut = valeurHaut * (3.3 / 4095.0);
  float tensionBas = valeurBas * (3.3 / 4095.0);
  return String(tensionHaut) + "," + String(tensionBas);
}