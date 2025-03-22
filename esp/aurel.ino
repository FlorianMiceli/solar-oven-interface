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

int ROT_OK = 0; //variable qu'on changera pour passer de la recherche du soleil sur la rotation à la recherche du soleil sur la translation
int TRANS_OK = 0;
bool moteurBloque = false;



void setup() {

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
}

void loop() {
    int Mode = 0;/*ici faut que tu mette la ligne qui nous permettra de récupérer la valeur que tu nous envoi;
    // pour le reste du programme je propose qu'on affecte à chaque bouton une valeur par exemple quand on clique sur droite cela envoi 1 qui nous 
    //envoi une valeur à nous par la suite de faire le mouvement qu'on veut dans ce programme 
    il nous faut donc une valeur : 
                                    - une valeur pour tourner à droite 
                                    - une valeur pour tourner à gauche 
                                    - une valeur pour translater vers l'avant 
                                    - une valeur pour translater vers l'arrière 
                                    - il faut aussi qu'on renvoi la valeur de la température tjr en direct
    */
    if ( Mode == 1){
      int Bouger = 4;//ici on met la valeur qui nous permet de récupérer la valeur pour connaitre le sens de rotation dans lequel on veut aller
      if ( Bouger == 1 ){//on part à droite 
          tournerRot(DROITE);
      }
      else if ( Bouger == 2 ){ //on part à gauche 
          tournerRot(GAUCHE);
      }
      else if ( Bouger == 3 ){//on part vers l'avant 
          tournerTrans(AVANT);
      }
      else if (Bouger == 4){// on part vers l'arrière 
          tournerTrans(ARRIERE);
      }
      else {// si on appuye pas soit on revient à une valeur de 0 ou de 5 mais il faut sortir des valeurs qu'on avait jusqu'à maintenant pour arrêter la parabole
        Serial.println("On bouge plus");
      }
    }
    else{

    
    if ( ROT_OK == 0) {
      faireRot();
    }
    else {
      if ( TRANS_OK == 0){
        faireTrans();
      }
      else {//On asservi par rapport à la température 
        
      }
    }
    }
}

void faireRot() {
    int valeurDroite = 500; // analogRead(PHOTODIODE_DROITE);
    int valeurGauche = 500;  // analogRead(PHOTODIODE_GAUCHE);

    if (abs(valeurDroite - valeurGauche) < 100) {
        Serial.println("Soleil trouvé en rotation !");
        ROT_OK = 1;
        return;
        
    }
    else if (valeurDroite > valeurGauche ){//on tourne à droite
      ROT_OK =0;
      tournerRot(DROITE);
    }
    else{// On tourne à gauche pour remettre bien la lumière
      ROT_OK=0;
      tournerRot(GAUCHE);
    }

}
void faireTrans(){
  int etatBoutonDebut = digitalRead(PIN_BOUTON_DEBUT);
  int etatBoutonFin = digitalRead(PIN_BOUTON_FIN);
  //ici il y a une faille dans le programme étant donnée que les boutons sont "borné" cela veut dire que dans si on va dans le sens arriere sur la
  //translation et qu'on appuye sur le bouton de fin on continue
  //d'aller en arrière, mais on est pas censé arriver sur cette interrupteur en arriere voila c pour ça. 
  if (etatBoutonFin == LOW){
    stopEtRetourArriere(ARRIERE);
    
  }
  else if (etatBoutonDebut == LOW){
    stopEtRetourArriere(AVANT);
    
  }
  if (digitalRead(PIN_BOUTON_DEBUT) == HIGH && digitalRead(PIN_BOUTON_FIN) ==HIGH){
    moteurBloque = false;
  }
  if(!moteurBloque){
    chercherSoleilTrans();
  }
}
void chercherSoleilTrans() {
    int valeurHaute = 5000; // analogRead(PHOTODIODE_HAUT);
    int valeurBasse = 500;  // analogRead(PHOTODIODE_BAS);

    if (abs(valeurHaute - valeurBasse) < 100) {//On fait rien
        Serial.println("Soleil trouvé en rotation !");
        return;
        TRANS_OK = 1;
        
    }
    else if (valeurHaute > valeurBasse ){//on monte la parabole
      tournerTrans(AVANT);
      TRANS_OK = 0;
    }
    else{// On descend la parabole
      tournerTrans(ARRIERE);
      TRANS_OK = 0;
    }

}



void stopEtRetourArriere(bool directionActuelle) {
    moteurBloque = true; // Bloquer le moteur pour éviter qu'il continue d'avancer

    // 🔹 Arrêter immédiatement le moteur
    digitalWrite(PUL_PIN_TRANS, LOW);
    delay(500); // Pause pour bien stopper

    // 🔹 Déterminer la direction inverse
    bool directionInverse = (directionActuelle == AVANT) ? ARRIERE : AVANT;
    
    

    digitalWrite(DIR_PIN_TRANS, directionInverse);

    // 🔹 Faire reculer le moteur (~10 000 pas -> 1/2 tour)
    for (int i = 0; i < 10000; i++) { 
        digitalWrite(PUL_PIN_TRANS, HIGH);
        delayMicroseconds(500);
        digitalWrite(PUL_PIN_TRANS, LOW);
        delayMicroseconds(500);
    }

    // 🔹 Arrêter totalement le moteur après le retour arrière
    digitalWrite(PUL_PIN_TRANS, LOW);
}
void tournerRot(bool direction) {
    digitalWrite(EN_PIN_ROT, LOW); // Active le driver
    digitalWrite(DIR_PIN_ROT, direction);
    
    // Génère 200 pas
    for(int i = 0; i < 50; i++) {
        digitalWrite(PUL_PIN_ROT, HIGH);
        delayMicroseconds(500);
        digitalWrite(PUL_PIN_ROT, LOW);
        delayMicroseconds(500);
    }
}
void tournerTrans(bool direction){
    digitalWrite(EN_PIN_TRANS, LOW); // Active le driver
    digitalWrite(DIR_PIN_TRANS, direction);
    
    // Génère 200 pas
    for(int i = 0; i < 500; i++) {
        digitalWrite(PUL_PIN_TRANS, HIGH);
        delayMicroseconds(500);
        digitalWrite(PUL_PIN_TRANS, LOW);
        delayMicroseconds(500);
      
}
}
