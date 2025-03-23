#include <WiFi.h>
#include <WebServer.h>

// Broches des photodiodes
#define PHOTODIODE_HAUT 4 // Photodiode haute (GPIO32)
#define PHOTODIODE_BAS 5  // Photodiode basse (GPIO33)

// julien
#define LED_PIN38
#define NUM_LEDS 1

// aurélien
#define PUL_PIN_ROT 9
#define DIR_PIN_ROT 10
#define EN_PIN_ROT 10
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

int ROT_OK = 0;
int TRANS_OK = 0;
bool moteurBloque = false;

// webserver
const char *ssid = "ESP32-S3_AP";
const char *password = "motdepasse123";
WebServer server(80);

void setup()
{
    // aurélien
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

    // lirePhotodiodes test
    pinMode(PHOTODIODE_HAUT, INPUT);
    pinMode(PHOTODIODE_BAS, INPUT);

    // webserver
    Serial.begin(115200);
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("Adresse IP du point d'accès: ");
    Serial.println(IP);
    server.on("/", handleRoot);
    server.on("/lirePhotodiodes", HTTP_GET, []()
              { server.send(200, "text/plain", lirePhotodiodes()); });

    server.on("/mode_manuel", []()
              { MODE = MODE_MANUEL; });
    server.on("/mode_asservissement", []()
              { MODE = MODE_ASSERVISSEMENT; });

    server.on("/rotation_droite", []()
              { MOUVEMENT = ROT_DROITE; });
    server.on("/rotation_gauche", []()
              { MOUVEMENT = ROT_GAUCHE; });
    server.on("/translation_arriere", []()
              { MOUVEMENT = TRANS_ARRIERE; });
    server.on("/translation_avant", []()
              { MOUVEMENT = TRANS_AVANT; });
    server.on("/stop", []()
              { MOUVEMENT = ARRET; });

    server.begin();
    Serial.println("Serveur HTTP démarré");
}

void loop()
{
    // Aurélien
    if (MODE == MODE_MANUEL)
    {
        if (MOUVEMENT == ROT_DROITE)
        { // on part à droite
            tournerRot(DROITE);
        }
        else if (MOUVEMENT == ROT_GAUCHE)
        { // on part à gauche
            tournerRot(GAUCHE);
        }
        else if (MOUVEMENT == TRANS_AVANT)
        { // on part vers l'avant
            tournerTrans(AVANT);
        }
        else if (MOUVEMENT == TRANS_ARRIERE)
        { // on part vers l'arrière
            tournerTrans(ARRIERE);
        }
        else
        { // si on appuye pas soit on revient à une valeur de 0 ou de 5 mais il faut sortir des valeurs qu'on avait jusqu'à maintenant pour arrêter la parabole
            Serial.println("On bouge plus");
        }
    }
    if (MODE == MODE_ASSERVISSEMENT)
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
            { // On asservi par rapport à la température
            }
        }
    }

    // webserver
    server.handleClient();
}

void handleRoot()
{
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

void stopMoteur()
{
    digitalWrite(PUL_PIN, HIGH);
}

void faireRot()
{
    int valeurDroite = 500; // analogRead(PHOTODIODE_DROITE);
    int valeurGauche = 500; // analogRead(PHOTODIODE_GAUCHE);

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
    int valeurHaute = 5000; // analogRead(PHOTODIODE_HAUT);
    int valeurBasse = 500;  // analogRead(PHOTODIODE_BAS);

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
    for (int i = 0; i < 50; i++)
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