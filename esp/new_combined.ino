#include <WiFi.h>
#include <WebServer.h>

#define PUL_PIN_ROT 9
#define DIR_PIN_ROT 10
#define EN_PIN_ROT 11
#define PUL_PIN_TRANS 15
#define DIR_PIN_TRANS 16
#define EN_PIN_TRANS 17
#define PHOTODIODE_DROITE 6
#define PHOTODIODE_GAUCHE 7
#define PHOTODIODE_HAUT 4
#define PHOTODIODE_BAS 5
#define GAUCHE LOW
#define DROITE HIGH
#define AVANT HIGH
#define ARRIERE LOW
#define PIN_BOUTON_DEBUT 18
#define PIN_BOUTON_FIN 8
#define TEMPERATURE 2
#define LED_PIN38
#define NUM_LEDS 1

int ROT_OK = 0; // variable qu'on changera pour passer de la recherche du soleil sur la rotation à la recherche du soleil sur la translation
int TRANS_OK = 0;
bool moteurBloque = false;
bool moteurActif = false; // Indique si le moteur a déjà tourné (pour le contrôle de la température)

const int margeHaute = 5; // Seuil supérieur : dépassement avant action
const int margeBasse = 5;

enum Mode
{
    MODE_MANUEL,
    MODE_ASSERVISSEMENT
};

enum Mouvement
{
    ARRET,
    ROT_DROITE,
    ROT_GAUCHE,
    TRANS_AVANT,
    TRANS_ARRIERE
};

Mode MODE = MODE_MANUEL;
Mouvement MOUVEMENT = ARRET;

// webserver
const char *ssid = "ESP32-S3_AP";
const char *password = "motdepasse123";
WebServer server(80);

void setCorsHeaders()
{
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET");
    server.sendHeader("Access-Control-Allow-Headers", "*");
}

void setup()
{

    pinMode(PUL_PIN_ROT, OUTPUT);
    pinMode(DIR_PIN_ROT, OUTPUT);
    pinMode(EN_PIN_ROT, OUTPUT);

    pinMode(PUL_PIN_TRANS, OUTPUT);
    pinMode(DIR_PIN_TRANS, OUTPUT);
    pinMode(EN_PIN_TRANS, OUTPUT);

    pinMode(PHOTODIODE_DROITE, INPUT);
    pinMode(PHOTODIODE_GAUCHE, INPUT);

    pinMode(PHOTODIODE_HAUT, INPUT);
    pinMode(PHOTODIODE_BAS, INPUT);

    pinMode(PIN_BOUTON_DEBUT, INPUT_PULLUP);
    pinMode(PIN_BOUTON_FIN, INPUT_PULLUP);

    // webserver
    Serial.begin(115200);
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("Adresse IP du point d'accès: ");
    Serial.println(IP);

    server.on("/", handleRoot);

    server.on("/lirePhotodiodes", HTTP_GET, []()
              {
        setCorsHeaders();
        server.send(200, "text/plain", lirePhotodiodes()); });

    server.on("/mode_manuel", []()
              {
        setCorsHeaders();
        MODE = MODE_MANUEL;
        server.send(200, "text/plain", "Route passed"); });

    server.on("/mode_asservissement", []()
              {
        setCorsHeaders();
        MODE = MODE_ASSERVISSEMENT;
        server.send(200, "text/plain", "Route passed"); });

    server.on("/rotation_droite", []()
              {
        setCorsHeaders();
        MOUVEMENT = ROT_DROITE;
        server.send(200, "text/plain", "Route passed"); });

    server.on("/rotation_gauche", []()
              {
        setCorsHeaders();
        MOUVEMENT = ROT_GAUCHE;
        server.send(200, "text/plain", "Route passed"); });

    server.on("/translation_arriere", []()
              {
        setCorsHeaders();
        MOUVEMENT = TRANS_ARRIERE;
        server.send(200, "text/plain", "Route passed"); });

    server.on("/translation_avant", []()
              {
        setCorsHeaders();
        MOUVEMENT = TRANS_AVANT;
        server.send(200, "text/plain", "Route passed"); });

    server.on("/stop", []()
              {
        setCorsHeaders();
        MOUVEMENT = ARRET;
        server.send(200, "text/plain", "Route passed"); });

    server.on("/getMode", []()
              {
        setCorsHeaders();
        server.send(200, "application/json", MODE == MODE_MANUEL ? "manuel" : "asservissement"); });

    server.begin();
    Serial.println("Serveur HTTP démarré");
}

void loop()
{
    if (MODE == MODE_MANUEL)
    {
        if (MOUVEMENT == ROT_DROITE)
        { // on part à droite
            tournerRot(DROITE);
        }
        if (MOUVEMENT == ROT_GAUCHE)
        { // on part à gauche
            tournerRot(GAUCHE);
        }
        if (MOUVEMENT == TRANS_AVANT)
        { // on part vers l'avant
            tournerTrans(AVANT);
        }
        if (MOUVEMENT == TRANS_ARRIERE)
        { // on part vers l'arrière
            tournerTrans(ARRIERE);
        }
        if (MOUVEMENT == ARRET)
        { // si on appuye pas soit on revient à une valeur de 0 ou de 5 mais il faut sortir des valeurs qu'on avait jusqu'à maintenant pour arrêter la parabole
            Serial.println("On bouge plus");
        }
    }
    else if (MODE == MODE_ASSERVISSEMENT)
    {

        if (ROT_OK == 0)
        {
            faireRot();
        }
        else
        {
            if (TRANS_OK == 0)
            {
                faireTrans();
            }
            else
            { // On asservi par rapport à la températurz
                controleTemperature(15);
            }
        }
    }

    // webserver
    server.handleClient();
}

void handleRoot()
{
    setCorsHeaders();
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

String lirePhotodiodes()
{
    int valeurHaut = analogRead(PHOTODIODE_HAUT);
    int valeurBas = analogRead(PHOTODIODE_BAS);
    float tensionHaut = valeurHaut * (3.3 / 4095.0);
    float tensionBas = valeurBas * (3.3 / 4095.0);
    return String(tensionHaut) + "," + String(tensionBas);
}

void faireRot()
{
    int valeurDroite = analogRead(PHOTODIODE_DROITE);
    int valeurGauche = analogRead(PHOTODIODE_GAUCHE);

    if (abs(valeurDroite - valeurGauche) < 100)
    {
        Serial.println("Soleil trouvé en rotation !");
        ROT_OK = 1;
        return;
    }
    else if (valeurDroite > valeurGauche)
    { // on tourne à droite
        ROT_OK = 0;
        tournerRot(DROITE);
    }
    else
    { // On tourne à gauche pour remettre bien la lumière
        ROT_OK = 0;
        tournerRot(GAUCHE);
    }
}
void faireTrans()
{
    int etatBoutonDebut = digitalRead(PIN_BOUTON_DEBUT);
    int etatBoutonFin = digitalRead(PIN_BOUTON_FIN);
    // ici il y a une faille dans le programme étant donnée que les boutons sont "borné" cela veut dire que dans si on va dans le sens arriere sur la
    // translation et qu'on appuye sur le bouton de fin on continue
    // d'aller en arrière, mais on est pas censé arriver sur cette interrupteur en arriere voila c pour ça.
    if (etatBoutonFin == LOW)
    {
        stopEtRetourArriere(ARRIERE);
    }
    else if (etatBoutonDebut == LOW)
    {
        stopEtRetourArriere(AVANT);
    }
    if (digitalRead(PIN_BOUTON_DEBUT) == HIGH && digitalRead(PIN_BOUTON_FIN) == HIGH)
    {
        moteurBloque = false;
    }
    if (!moteurBloque)
    {
        chercherSoleilTrans();
    }
}
void chercherSoleilTrans()
{
    int valeurHaute = analogRead(PHOTODIODE_HAUT);
    int valeurBasse = analogRead(PHOTODIODE_BAS);

    if (abs(valeurHaute - valeurBasse) < 100)
    { // On fait rien
        Serial.println("Soleil trouvé en rotation !");
        return;
        TRANS_OK = 1;
    }
    else if (valeurHaute > valeurBasse)
    { // on monte la parabole
        tournerTrans(AVANT);
        TRANS_OK = 0;
    }
    else
    { // On descend la parabole
        tournerTrans(ARRIERE);
        TRANS_OK = 0;
    }
}

void stopEtRetourArriere(bool directionActuelle)
{
    moteurBloque = true; // Bloquer le moteur pour éviter qu'il continue d'avancer

    // 🔹 Arrêter immédiatement le moteur
    digitalWrite(PUL_PIN_TRANS, LOW);
    delay(500); // Pause pour bien stopper

    // 🔹 Déterminer la direction inverse
    bool directionInverse = (directionActuelle == AVANT) ? ARRIERE : AVANT;

    digitalWrite(DIR_PIN_TRANS, directionInverse);

    // 🔹 Faire reculer le moteur (~10 000 pas -> 1/2 tour)
    for (int i = 0; i < 10000; i++)
    {
        digitalWrite(PUL_PIN_TRANS, HIGH);
        delayMicroseconds(500);
        digitalWrite(PUL_PIN_TRANS, LOW);
        delayMicroseconds(500);
    }

    // 🔹 Arrêter totalement le moteur après le retour arrière
    digitalWrite(PUL_PIN_TRANS, LOW);
}
void tournerRot(bool direction)
{
    digitalWrite(EN_PIN_ROT, LOW); // Active le driver
    digitalWrite(DIR_PIN_ROT, direction);

    // Génère 200 pas
    for (int i = 0; i < 500; i++)
    {
        digitalWrite(PUL_PIN_ROT, HIGH);
        delayMicroseconds(500);
        digitalWrite(PUL_PIN_ROT, LOW);
        delayMicroseconds(500);
    }
}
void tournerTrans(bool direction)
{
    digitalWrite(EN_PIN_TRANS, LOW); // Active le driver
    digitalWrite(DIR_PIN_TRANS, direction);

    // Génère 200 pas
    for (int i = 0; i < 500; i++)
    {
        digitalWrite(PUL_PIN_TRANS, HIGH);
        delayMicroseconds(500);
        digitalWrite(PUL_PIN_TRANS, LOW);
        delayMicroseconds(500);
    }
}
float lireTemperature()
{
    int donneBrut = analogRead(TEMPERATURE);
    Serial.print("Valeur brut : ");
    Serial.println(donneBrut);

    float valeurEnVolt = (donneBrut * 3.3) / 4095;
    Serial.print("Valeur de la tension mesurée : ");
    Serial.println(valeurEnVolt);

    float valeurEnAmpere = valeurEnVolt / 5500; // I = U / R
    Serial.print("Valeur de l'intensité (A) déduite : ");
    Serial.println(valeurEnAmpere);

    float valeurEnMicroAmpere = valeurEnAmpere * 1000000;
    Serial.print("Valeur de l'intensité (µA) déduite : ");
    Serial.println(valeurEnMicroAmpere);

    float temperatureCelsius = valeurEnMicroAmpere - 273; // Conversion
    Serial.print("Valeur de la température en °C : ");
    Serial.println(temperatureCelsius);

    float temperatureHaute = temperatureCelsius + 10;
    float temperatureBasse = temperatureCelsius - 10;
    float temperatureOffset = temperatureCelsius + 9;
    Serial.print("La valeur vraie se trouve dans cette intervalle : ");
    Serial.print(temperatureBasse);
    Serial.print("°C ; ");
    Serial.print(temperatureHaute);
    Serial.println("°C");
    Serial.print("Valeur de la température avec un offset ");

    delay(2000);
    return temperatureOffset; // Retourner la valeur
}
void controleTemperature(int tempConsigne)
{
    float temperature = lireTemperature(); // Récupérer la température
    Serial.println("  ");
    Serial.print("Température mesurée : ");
    Serial.println(temperature);

    // Si la température dépasse de +5°C et que le moteur n'a pas encore tourné vers la gauche
    if (temperature >= tempConsigne + margeHaute && !moteurActif)
    {
        Serial.println("Température trop haute, déplacement du moteur vers la gauche...");
        tournerRot(GAUCHE); // Tourne le moteur vers la gauche
        moteurActif = true; // Empêche d'activer plusieurs fois le moteur inutilement
    }

    // Si la température descend sous -5°C de la consigne, on tourne vers la droite
    else if (temperature <= tempConsigne - margeBasse && moteurActif)
    {
        Serial.println("Température trop basse, repositionnement du moteur vers la droite...");
        tournerRot(DROITE);  // Tourne le moteur vers la droite
        moteurActif = false; // Autorise un futur déplacement
                             // 🔹 Remettre en mode automatique après ajustement de la température
        ROT_OK = 0;
        TRANS_OK = 0;
    }
}