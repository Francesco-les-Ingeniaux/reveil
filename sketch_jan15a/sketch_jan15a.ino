//Bibliothèques nécessaires
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //Mise en place des librairies pour le LCD en I2C

//NbR correspond au nombre de réveils
#define NbR 2

//N correspond au nombre d'écrans différents
#define N NbR+1

LiquidCrystal_I2C lcd(0x3F, 16, 2); //Initialisation de la communication I2C avec l'écran

//Déclaration des variables globales

unsigned long ul_Temps = 0UL; //Pour compter le temps depuis le début du programme
unsigned long ul_Tempsnouveau = 0UL; //Pour compter le différentiel d'une seconde

unsigned long ul_Blink= 0UL; //Pour compter le différentiel entre les clignotements lorsque l'on passe en mode settings
bool state = 0; //Pour savoir si on affiche ou pas lors du clignotement

int secondes=0; //Compteur des secondes de l'heure courante
int minutes=0; //Compteur des minutes de l'heure courante
int heures=0; //Compteur des heures de l'heure courante

int etatSettings = 0; //Pour savoir dans quel écran on se trouve (Heure courante, Mise à l'heure, Mise en route des alarmes)
int etatSwitch = 0; //Pour savoir combien de fois on a appuyé sur switch donc sur quel paramètres (heures, minutes, secondes ou ON/OFF)

int pinRelais = 4; //Pin pour lequel le relais est mis en place

//Déclaration des structures
typedef struct bouton{ 
  int pin; //Pour savoir à quel pin on branche le bouton
  bool actif=0; //Pour savoir si le bouton est appuyé ou non
};
bouton MesBoutons[4]; //Tableau de boutons (4 - Settings, Switch, -, +)

typedef struct reveil{
  int heures=0; //Compteur des heures du réveil
  int minutes=0; //Compteur des minutes du réveil
  bool set=0; //Pour savoir si le réveil est mis en place ou pas
  bool actif=0; //Pour savoir si le réveil est activé ou pas (sonne)
};
reveil MesReveils[NbR]; //Tableau de réveils

void setup() {

  //Déclaration des pins pour les boutons
  MesBoutons[0].pin = 6;
  MesBoutons[1].pin = 7;
  MesBoutons[2].pin = 8;
  MesBoutons[3].pin = 9;
  
  //Initialisation du LCD
  lcd.init(); //Initialisation du lcd
  lcd.backlight(); //Démarrage du rétroéclairage
  
  //On active le pin du relais en mode output (sortie)
  pinMode(pinRelais, OUTPUT);

  
  for (int i=0 ; i<4 ; ++i)
  {
    pinMode( MesBoutons[i].pin, INPUT); //On active les pin des boutons en input (entrée)
    digitalWrite( MesBoutons[i].pin, HIGH); //On set les pin des boutons en état haut
  }
}

void loop() {
 
 ul_Temps=millis(); //On récupère le temps depuis le lancement de l'arduino

//TEST DES BOUTONS

 for (int i=0 ; i<4 ; ++i) //On teste si les boutons sont appuyés
 {
   if (digitalRead(MesBoutons[i].pin)==LOW&&MesBoutons[i].actif==0) //Si le courant reçu est à l'état bas (0) et que le bouton n'est pas appuyé
   {
    MesBoutons[i].actif=1; //Si le bouton est appuyé on le passe à 1
    if (i==0)
      actionSettings(); //On effection les actions pour le bouton settings
    if (i==1)
      actionSwitch(); //On effection les actions pour le bouton switch
    if (i==2)
      actionMoins(); //On effection les actions pour le bouton moins
    if (i==3)
      actionPlus(); //On effection les actions pour le bouton Plus
   }
   else if (digitalRead(MesBoutons[i].pin)==HIGH&&MesBoutons[i].actif==1) //Sinon, si le bouton a été appuyé mais qu'il ne l'est plus
   {
    MesBoutons[i].actif=0; //On le fait repasser à 0
   }
 }

//TEST DU TEMPS

 if (secondes >= 60) //Si 60 secondes se sont écoulées, on incrémente les minutes
    {   
      minutes+=1;
      secondes=0; //On réinitialise les secondes également
    }
 if (minutes >= 60) //Si 60 minutes se sont écoulées, on incrémente les heures
    {
      heures+=1;
      minutes=0; //On réinitialise les minutes également
    }
 if (heures >=24) //Si 24 heures se sont écoulées, on revient à 0
       heures=0;
 if (secondes < 0) //Si on revient en arrière sous les 0 secondes, on remonte à 59
      secondes=59;
 if (minutes < 0) //Si on revient en arrière sous les 0 minutes, on remonte à 59
      minutes=59;
 if (heures < 0) //Si on revient en arrière sous les 0 heures, on remonte à 23
      heures=23;

      
 if (ul_Temps - ul_Tempsnouveau > 1000) //Si le temps entre les deux mesures de temps est supérieur à 1000 millisecondes
 {
    ul_Tempsnouveau=ul_Temps; //la nouvelle mesure de temps prend la valeur de la première
    secondes+=1; //On incrémente les secondes
 }
 if (ul_Temps < ul_Tempsnouveau) //Si jamais il y a eu une remise à zéro (après 50 jours), on réinitialise le temps 
 {
  ul_Tempsnouveau=0;
 }


 //TEST DES ETATS DU REVEIL
 //On affiche le mode voulu sur l'écran LCD
 if (etatSettings==0)
    affichageEtat0(); //Mode temps courant
 if (etatSettings==1)
    affichageEtat1(); //Mode settings du temps courant
 if (etatSettings>=2)
    affichageEtatN(); //Mode réveils
   
}

void affichageEtat0() //Sous programme d'affichage
{
      lcd.setCursor(4,0);
      if (heures<10)
      {
        lcd.print("0");
      }
      lcd.print(heures);
      lcd.print(":");
      if (minutes<10)
      {
        lcd.print("0");
      }
      lcd.print(minutes);
      lcd.print(":");
      if (secondes<10)
      {
        lcd.print("0");
      }
      lcd.print(secondes);
      lcd.setCursor(2,1);
      lcd.print("Current Time");
     
}

void affichageEtat1()
{         
      lcd.setCursor(4,0);
      blinkState();
        if (state==1 && etatSwitch==0)
        {
          if (heures<10)
          {
            lcd.print("0");
          }
          lcd.print(heures);
        }
        else if (etatSwitch==0)
          lcd.print("  ");
        else if (etatSwitch)
          {
            if (heures<10)
            {
              lcd.print("0");
            }
            lcd.print(heures);
          }
       
      lcd.print(":");
      
      if (state==1 && etatSwitch==1)
        {
          if (minutes<10)
          {
            lcd.print("0");
          }
          lcd.print(minutes);
        }
        else if (etatSwitch==1)
          lcd.print("  ");
        else if (etatSwitch!=1)
          {
            if (minutes<10)
            {
              lcd.print("0");
            }
            lcd.print(minutes);
          }
      
      lcd.print(":");
      if (state==1 && etatSwitch==2)
        {
          if (secondes<10)
          {
            lcd.print("0");
          }
          lcd.print(secondes);
        }
        else if (etatSwitch==2)
          lcd.print("  ");
        else if (etatSwitch!=2)
          {
            if (secondes<10)
            {
              lcd.print("0");
            }
            lcd.print(secondes);
          }
       lcd.setCursor(2,1);
       lcd.print("Setting Time");
}

void affichageEtatN()
{
     int i=etatSettings;
        lcd.setCursor(4,0);
        blinkState();
        if (state==1 && etatSwitch==0)
         {
            if (MesReveils[i-NbR].heures<10)
            {
              lcd.print("0");
            }
            lcd.print(MesReveils[i-NbR].heures);
          }
          else if (etatSwitch==0)
            lcd.print("  ");
          else if (etatSwitch)
          {
             if (MesReveils[i-NbR].heures<10)
             {
               lcd.print("0");
             }
               lcd.print(MesReveils[i-NbR].heures);
           }
           lcd.print(":");
      
        if (state==1 && etatSwitch==1)
        {
          if (MesReveils[i-NbR].minutes<10)
          {
            lcd.print("0");
          }
          lcd.print(MesReveils[i-NbR].minutes);
        }
        else if (etatSwitch==1)
          lcd.print("  ");
        else if (etatSwitch!=1)
          {
            if (MesReveils[i-NbR].minutes<10)
            {
              lcd.print("0");
            }
            lcd.print(MesReveils[i-NbR].minutes);
          }
        lcd.print(" ");
        
        if (state==1 && etatSwitch==2)
        {
          if (MesReveils[i-NbR].set)
              lcd.print("ON");
          else
              lcd.print("OFF");
        }
        else if (etatSwitch==2)
         {
            if (MesReveils[i-NbR].set)
                lcd.print("  ");
            else
                lcd.print("   ");
         }
        else if (etatSwitch!=2)
          {
            if (MesReveils[i-NbR].set)
              lcd.print("ON");
          else
              lcd.print("OFF");
          }
         lcd.setCursor(0,1);
         lcd.print("Setting Alarm ");
         lcd.print(i-1);
}

void actionSettings()
{
  if (etatSettings >= 0 && etatSettings < N)
    etatSettings++;
  else 
    etatSettings=0;
  etatSwitch=0;
  lcd.clear();
}

void actionSwitch()
{
  if (etatSettings >= 1)
  {
    if (etatSwitch >= 0 && etatSwitch < 2)
      etatSwitch++;
    else 
      etatSwitch=0;
  }
}

void actionPlus()
{
  if (etatSettings == 1)
  {
    if (etatSwitch==0)
      heures+=1;
    if (etatSwitch==1)
      minutes+=1;
    if (etatSwitch==2)
      secondes+=1;
  }
  if (etatSettings >= 2)
  { 
    int i=etatSettings;
    if (etatSwitch==0)
      MesReveils[i-NbR].heures+=1;
    if (etatSwitch==1)
      MesReveils[i-NbR].minutes+=1;
    if (etatSwitch==2)
    {
      MesReveils[i-NbR].set=1;
      lcd.clear();
    }
  }
}

void actionMoins()
{
    if (etatSettings == 1)
    {
      if (etatSwitch==0)
        heures-=1;
      if (etatSwitch==1)
        minutes-=1;
      if (etatSwitch==2)
        secondes-=1;
    }
    if (etatSettings >= 2)
    { 
      int i=etatSettings;
      if (etatSwitch==0)
        MesReveils[i-NbR].heures-=1;
      if (etatSwitch==1)
        MesReveils[i-NbR].minutes-=1;
      if (etatSwitch==2)
      {
        MesReveils[i-NbR].set=0;
        lcd.clear();
      }
    }
}

void blinkState()
{
  if(ul_Temps - ul_Blink > 750) 
   {
        ul_Blink=ul_Temps;
        if (state==0)
        { 
          state=1;
        }
         else
        {
           state=0;  
        }
    }
}
