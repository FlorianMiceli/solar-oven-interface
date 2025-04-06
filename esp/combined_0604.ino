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
#define PIN_TEMPERATURE 2   // <<< corrigé ici
#define LED_PIN 38
#define NUM_LEDS 1

int ROT_OK = 0;
int TRANS_OK = 0;

// Variables pour suivre le temps sans soleil
unsigned long lastCheckTime = 0;
unsigned long absenceStartTime = 0;  // Temps de début de l'absence de soleil
unsigned long maxAbsenceTime = 60000;  // 1 minute (max absence avant alerte)
bool soleilTrouve = true;  // Indicateur si on a trouvé le soleil ou pas

bool moteurBloque = false;
bool moteurActif = false;

const int margeHaute = 5;
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

enum EtatAsservissement
{
    ROTATION,
    TRANSLATION,
    TEMPERATURE
};

Mode MODE = MODE_MANUEL;
Mouvement MOUVEMENT = ARRET;
EtatAsservissement etatAsserv = ROTATION;
float TARGET_TEMPERATURE = 45;

const char *ssid = "ESP32-S3_AP";
const char *password = "motdepasse123";
WebServer server(80);

void setCorsHeaders()
{
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
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

    Serial.begin(115200);
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("Adresse IP du point d'accès: ");
    Serial.println(IP);

    server.on("/", handleRoot);
    server.on("/lirePhotodiodes", HTTP_GET, []() {
        setCorsHeaders();
        server.send(200, "text/plain", lirePhotodiodes());
    });

    server.on("/mode_manuel", []() {
        setCorsHeaders();
        MODE = MODE_MANUEL;
        server.send(200, "text/plain", "Route passed");
    });

    server.on("/mode_asservissement", []() {
        setCorsHeaders();
        MODE = MODE_ASSERVISSEMENT;
        server.send(200, "text/plain", "Route passed");
    });

    server.on("/rotation_droite", []() {
        setCorsHeaders();
        MOUVEMENT = ROT_DROITE;
        server.send(200, "text/plain", "Route passed");
    });

    server.on("/rotation_gauche", []() {
        setCorsHeaders();
        MOUVEMENT = ROT_GAUCHE;
        server.send(200, "text/plain", "Route passed");
    });

    server.on("/translation_arriere", []() {
        setCorsHeaders();
        MOUVEMENT = TRANS_ARRIERE;
        server.send(200, "text/plain", "Route passed");
    });

    server.on("/translation_avant", []() {
        setCorsHeaders();
        MOUVEMENT = TRANS_AVANT;
        server.send(200, "text/plain", "Route passed");
    });

    server.on("/stop", []() {
        setCorsHeaders();
        MOUVEMENT = ARRET;
        server.send(200, "text/plain", "Route passed");
    });

    server.on("/getMode", []() {
        setCorsHeaders();
        server.send(200, "application/json", MODE == MODE_MANUEL ? "manuel" : "asservissement");
    });

    server.on("/getTargetTemperature", []()
            {
    setCorsHeaders();
    server.send(200, "application/json", String(TARGET_TEMPERATURE)); });

    server.on("/getTemperature", []()
              {
        setCorsHeaders();
        server.send(200, "application/json", fakeTemperature()); });
    // server.send(200, "application/json", ); String(lireTemperature()); });

    server.on("/setTargetTemperature", []()
              {
        setCorsHeaders();
        if (server.hasArg("value")) {
            TARGET_TEMPERATURE = server.arg("value").toInt();
            server.send(200, "application/json", String(TARGET_TEMPERATURE));
        } else {
            server.send(400, "text/plain", "Missing value parameter");
        } });

    server.begin();
    Serial.println("Serveur HTTP démarré");
}


String fakeTemperature()
{
    float time = millis() / 1000.0;
    float random_offset = random(-250, 250) / 1000.0;
    return String(time + random_offset);
}

void loop()
{
    if (MODE == MODE_MANUEL)
    {
        ROT_OK = 0;
        TRANS_OK = 0;
        if (MOUVEMENT == ROT_DROITE)
            tournerRot(DROITE);
        if (MOUVEMENT == ROT_GAUCHE)
            tournerRot(GAUCHE);
        if (MOUVEMENT == TRANS_AVANT)
            tournerTrans(AVANT);
        if (MOUVEMENT == TRANS_ARRIERE)
            tournerTrans(ARRIERE);
        if (MOUVEMENT == ARRET)
        {
            Serial.println("On bouge plus");
            stopMoteurTrans();
        }
    }
    else if (MODE == MODE_ASSERVISSEMENT)
    {
    // Appel à la fonction soleilAbsent() pour vérifier l'absence de soleil
    String resultat = soleilAbsent();
    Serial.println(resultat);  // Affichage du résultat pour debug (peut être utilisé pour avertir l'utilisateur)
        switch (etatAsserv)
        {
            case ROTATION:
                faireRot();
                if (ROT_OK == 1)
                    etatAsserv = TRANSLATION;
                break;
            case TRANSLATION:
                
                faireTrans();
                if (abs (analogRead(PHOTODIODE_DROITE) - analogRead(PHOTODIODE_GAUCHE))<300){
                  etatAsserv = ROTATION;
                  ROT_OK = 0;
                  TRANS_OK = 0;
                }
                else if (TRANS_OK == 1)
                    etatAsserv = TEMPERATURE;
                break;
            case TEMPERATURE:
                if (abs (analogRead(PHOTODIODE_DROITE) - analogRead(PHOTODIODE_GAUCHE))<300){
                  etatAsserv = ROTATION;
                  ROT_OK = 0;
                  TRANS_OK = 0;
                }
                else if (abs(analogRead(PHOTODIODE_HAUT) - analogRead(PHOTODIODE_BAS))<300){
                  etatAsserv = TRANSLATION;
                  TRANS_OK = 0;
                }
                controleTemperature(15);
                break;
        }
    }
    server.handleClient();
}

void handleRoot()
{
    setCorsHeaders();
    String html = "<html><head><script>function updateValues() {var xhttp = new XMLHttpRequest();xhttp.onreadystatechange = function() {if (this.readyState == 4 && this.status == 200) {var values = this.responseText.split(',');document.getElementById('tensionHaut').innerHTML = values[0];document.getElementById('tensionBas').innerHTML = values[1];}};xhttp.open('GET', '/lirePhotodiodes', true);xhttp.send();}setInterval(updateValues, 1000);</script></head><body><h1>Tensions des photodiodes</h1><p>Photodiode haute : <span id='tensionHaut'></span> V</p><p>Photodiode basse : <span id='tensionBas'></span> V</p></body></html>";
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


void stopMoteurTrans()
{
    digitalWrite(EN_PIN_TRANS, LOW);
    digitalWrite(DIR_PIN_TRANS, LOW);
    digitalWrite(PUL_PIN_TRANS, HIGH);
}

String soleilAbsent() {
    unsigned long currentMillis = millis();

    // Si 1 seconde est écoulée, on vérifie l'absence de soleil
    if (currentMillis - lastCheckTime >= 1000) {
        lastCheckTime = currentMillis;  // Mise à jour du dernier temps vérifié

        int valeurDroite = analogRead(PHOTODIODE_DROITE);
        int valeurGauche = analogRead(PHOTODIODE_GAUCHE);
        int valeurHaute = analogRead(PHOTODIODE_HAUT);
        int valeurBasse = analogRead(PHOTODIODE_BAS);

        // Vérification de l'absence de soleil (luminosité faible sur les photodiodes)
        if (valeurDroite < 300 && valeurGauche < 300 && valeurHaute < 300 && valeurBasse < 300) {
            if (absenceStartTime == 0) {
                absenceStartTime = currentMillis;  // Enregistrer le temps de début de l'absence de soleil
            }
            soleilTrouve = false;
        } else {
            absenceStartTime = 0;  // Réinitialiser si on trouve du soleil
            soleilTrouve = true;
        }
    }

    // Vérifier si l'absence de soleil dure trop longtemps
    if (!soleilTrouve && currentMillis - absenceStartTime >= maxAbsenceTime) {
        return "Soleil absent depuis trop longtemps";
    }

    return "Soleil détecté";
}

void faireRot()
{
    int valeurDroite = analogRead(PHOTODIODE_DROITE);
    int valeurGauche = analogRead(PHOTODIODE_GAUCHE);

    if (abs(valeurDroite - valeurGauche) < 250)
    {
        Serial.println("Soleil trouvé en rotation !");
        if ((valeurDroite > 500) && (valeurGauche > 500))
            ROT_OK = 1;
        return;
    }
    else if (valeurDroite > valeurGauche)
    {
        ROT_OK = 0;
        tournerRot(DROITE);
    }
    else
    {
        ROT_OK = 0;
        tournerRot(GAUCHE);
    }
}

void faireTrans()
{
    int etatBoutonDebut = digitalRead(PIN_BOUTON_DEBUT);
    int etatBoutonFin = digitalRead(PIN_BOUTON_FIN);

    if (etatBoutonFin == LOW)
        stopEtRetourArriere(ARRIERE);
    else if (etatBoutonDebut == LOW)
        stopEtRetourArriere(AVANT);

    if (digitalRead(PIN_BOUTON_DEBUT) == HIGH && digitalRead(PIN_BOUTON_FIN) == HIGH)
        moteurBloque = false;

    if (!moteurBloque)
        chercherSoleilTrans();
}

void chercherSoleilTrans()
{
    int valeurHaute = analogRead(PHOTODIODE_HAUT);
    int valeurBasse = analogRead(PHOTODIODE_BAS);

    if (abs(valeurHaute - valeurBasse) < 250)
    {
        stopMoteurTrans();
        Serial.println("Soleil trouvé en translation !");
        if ((valeurHaute > 500) && (valeurBasse > 500))
            TRANS_OK = 1;
        return;
    }
    else if (valeurHaute > valeurBasse)
    {
        tournerTrans(AVANT);
        TRANS_OK = 0;
    }
    else
    {
        tournerTrans(ARRIERE);
        TRANS_OK = 0;
    }
}

void stopEtRetourArriere(bool directionActuelle)
{
    moteurBloque = true;
    digitalWrite(PUL_PIN_TRANS, LOW);
    delay(500);
    bool directionInverse = (directionActuelle == AVANT) ? ARRIERE : AVANT;
    digitalWrite(DIR_PIN_TRANS, directionInverse);

    for (int i = 0; i < 430; i++)
    {
        digitalWrite(PUL_PIN_TRANS, HIGH);
        delayMicroseconds(500);
        digitalWrite(PUL_PIN_TRANS, LOW);
        delayMicroseconds(500);
    }

    digitalWrite(PUL_PIN_TRANS, LOW);
}

void tournerRot(bool direction)
{
    digitalWrite(EN_PIN_ROT, LOW);
    digitalWrite(DIR_PIN_ROT, direction);
    digitalWrite(PUL_PIN_ROT, HIGH);
    delayMicroseconds(500);
    digitalWrite(PUL_PIN_ROT, LOW);
    delayMicroseconds(500);
}

void tournerTrans(bool direction)
{
    digitalWrite(EN_PIN_TRANS, LOW);
    digitalWrite(DIR_PIN_TRANS, direction);
    digitalWrite(PUL_PIN_TRANS, HIGH);
    delayMicroseconds(500);
    digitalWrite(PUL_PIN_TRANS, LOW);
    delayMicroseconds(500);
}

float lireTemperature()
{
    int donneBrut = analogRead(PIN_TEMPERATURE);  // <<< modif ici
    float valeurEnVolt = (donneBrut * 3.3) / 4095;
    float valeurEnAmpere = valeurEnVolt / 5500.0;
    float valeurEnMicroAmpere = valeurEnAmpere * 1000000;
    float temperatureCelsius = valeurEnMicroAmpere - 273;
    Serial.println(temperatureCelsius);
    float temperatureOffset = temperatureCelsius + 9;
    delay(2000);
    return temperatureOffset;
}

void controleTemperature(int tempConsigne)
{
    float temperature = lireTemperature();
    Serial.print("Température mesurée : ");
    Serial.println(temperature);

    if (temperature >= tempConsigne + margeHaute && !moteurActif)
    {
        Serial.println("Température trop haute, déplacement du moteur vers la gauche...");
        tournerRot(GAUCHE);
        moteurActif = true;
    }
    else if (temperature <= tempConsigne - margeBasse && moteurActif)
    {
        Serial.println("Température trop basse, repositionnement du moteur vers la droite...");
        ROT_OK = 0;
        TRANS_OK = 0;
        moteurActif = false;
        etatAsserv = ROTATION;
    }
}