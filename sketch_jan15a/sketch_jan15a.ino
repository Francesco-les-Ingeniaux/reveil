//Bibliothèques nécessaires
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //Mise en place des librairies pour le LCD en I2C
#include <DS3231.h> //Mise en place des librairies pour la RTC

//NbR correspond au nombre de réveils
#define NbR 2

//N correspond au nombre d'écrans différents
#define N NbR+1
// hahaha


DS3231 Clock; //Objet de type DS3231 (RTC)
bool h24, AM; //Deux booléen obligatoires 
 

LiquidCrystal_I2C lcd(0x3F, 16, 2); //Initialisation de la communication I2C avec l'écran

//Déclaration des variables globales

unsigned long ul_Temps = 0UL; //Pour compter le temps depuis le début du programme
unsigned long ul_Sleep=0UL; //Pour compter le différentiel entre la transition en mode veille ou non
unsigned long ul_Blink= 0UL; //Pour compter le différentiel entre les clignotements lorsque l'on passe en mode settings
bool state = 0; //Pour savoir si on affiche ou pas lors du clignotement
bool sleep=0; //Pour savoir si le réveil est en veille ou pas
bool sonne=0; //Pour savoir si le réveil sonne ou pas

int etatSettings = 0; //Pour savoir dans quel écran on se trouve (Heure courante, Mise à l'heure, Mise en route des alarmes)
int etatSwitch = 0; //Pour savoir combien de fois on a appuyé sur switch donc sur quel paramètres (heures, minutes, secondes ou ON/OFF)

int pinRelais = 2; //Pin pour lequel le relais est mis en place

//Déclaration des structures
typedef struct bouton{ 
  int pin; //Pour savoir à quel pin on branche le bouton
  bool actif=0; //Pour savoir si le bouton est appuyé ou non
};

bouton MesBoutons[4]; //Tableau de boutons (4 - Settings, Switch, -, +)
// Bla bla
// tralala
  
typedef struct reveil{
  int heures=0; //Compteur des heures du réveil
  int minutes=0; //Compteur des minutes du réveil
  bool set=0; //Pour savoir si le réveil est mis en place ou pas
};
reveil MesReveils[NbR]; //Tableau de réveils

void setup() {
   Clock.setClockMode(true); //Formate l'heure en 24h
   //Déclaration des pins pour les boutons
   MesBoutons[0].pin = 11;
   MesBoutons[1].pin = 10;
   MesBoutons[2].pin = 9;
   MesBoutons[3].pin = 8;
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
 sleepState(); //Pour le mode veille
 alarm(); //Pour le mode alarme
 testBouton(); //On teste les boutons et on active leur fonction
 testEtat(); //On teste les états et on afficge l'état dans lequel on se trouve
  delay(5);
}

bool IsThereAlarm() //Sous programme qui retourn 1 si un réveil est enclenché 0 sinon
{
  for (int i=0 ; i < NbR ; ++i)
    if (MesReveils[i].set)
      return 1;
  return 0;
}

void affichageEtat0() //Sous programme d'affichage
{
      lcd.setCursor(4,0); //On place le curseur là où on veut écrire tel que (emplacement, ligne)
      if (Clock.getHour(h24, AM)<10) //Pour pouvoir mettre un 0 avant les dizaines 
         lcd.print("0");
      lcd.print(Clock.getHour(h24, AM)); //On affiche les heures avec la fonction get de la classe DS3231
      
      lcd.print(":");
      
      if (Clock.getMinute()<10)//Pour pouvoir mettre un 0 avant les dizaines 
         lcd.print("0");
      lcd.print(Clock.getMinute()); //On affiche les minutes
      
      lcd.print(":");
      
      if (Clock.getSecond()<10)//Pour pouvoir mettre un 0 avant les dizaines 
         lcd.print("0");
      lcd.print(Clock.getSecond()); //On affiche les secondes
      
      lcd.setCursor(2,1); //On place le curseur sur la seconde ligne 
      lcd.print("Current Time");

      if (IsThereAlarm()) //Si un réveil est enclenché
      { 
        lcd.setCursor(15,0);
        lcd.print("."); //On met un point sur l'écran principal
      }     
}

void testEtat()
{
  
 //On affiche le mode voulu sur l'écran LCD
 if (etatSettings==0)
    affichageEtat0(); //Mode temps courant
 if (etatSettings==1)
    affichageEtat1(); //Mode settings du temps courant
 if (etatSettings>=2)
    affichageEtatN(); //Modes des réveils
    
}
void testBouton()
{
  for (int i=0 ; i<4 ; ++i) //On teste si les boutons sont appuyés
 {
   if (digitalRead(MesBoutons[i].pin)==LOW&&MesBoutons[i].actif==0) //Si le courant reçu est à l'état bas (0) et que le bouton n'est pas appuyé
   {    
      MesBoutons[i].actif=1; //Si le bouton est appuyé on le passe à 1
      if (!sleep&&!sonne)
      {
        if (i==0)
          actionSettings(); //On effection les actions pour le bouton settings
        if (i==1)
          actionSwitch(); //On effection les actions pour le bouton switch
        if (i==2)
          actionMoins(); //On effection les actions pour le bouton moins
        if (i==3)
          actionPlus(); //On effection les actions pour le bouton Plus
      }
      ul_Sleep=ul_Temps;
      sleep=0;
      lcd.clear();
      if (sonne)
      {
        sonne=0;
        digitalWrite(pinRelais, LOW);
      }
   }
   else if (digitalRead(MesBoutons[i].pin)==HIGH&&MesBoutons[i].actif==1) //Sinon, si le bouton a été appuyé mais qu'il ne l'est plus
   {
      MesBoutons[i].actif=0; //On le fait repasser à 0
   }
 }
}

void affichageEtat1() //Le principe va être le même pour les deux sous programmes suivant : On fait clignoter le paramètre pointé par SWITCH soit l'heure, les minutes, les secondes ou on/off et si on appuie sur switch, et bien on change
{         
      lcd.setCursor(4,0);
      blinkState(); //Sous programme pour faire passer un booléen de 0 à 1 pour savoir si 750 millisecondes se sont écoulées
        if (state==1 && etatSwitch==0) //Si jamais c'est le cas et que l'on pointe sur les heures
        {
          if (Clock.getHour(h24, AM)<10) //Si on est sur des heures sans dizaine on met un zéro (esthétique)
          {
            lcd.print("0");
          }
          lcd.print(Clock.getHour(h24, AM)); //On affiche les heures
        }
        else if (etatSwitch==0) //Sinon on vide la case, ce qui à pour effet de clignoter
          lcd.print("  ");
        else if (etatSwitch) //Si jamais on ne pointe plus les heures avec Switch, alors on affiche les heures normalement.
          {
            if (Clock.getHour(h24, AM)<10) //Si on est sur des heures sans dizaine on met un zéro (esthétique)
            {
              lcd.print("0");
            }
            lcd.print(Clock.getHour(h24, AM));
          }
       
      lcd.print(":");
      
      if (state==1 && etatSwitch==1) //Le principe est le même que pour les heures
        {
          if (Clock.getMinute()<10)
          {
            lcd.print("0");
          }
          lcd.print(Clock.getMinute());
        }
        else if (etatSwitch==1)
          lcd.print("  ");
        else if (etatSwitch!=1)
          {
            if (Clock.getMinute()<10)
            {
              lcd.print("0");
            }
            lcd.print(Clock.getMinute());
          }
      
      lcd.print(":");
      if (state==1 && etatSwitch==2) //Le principe est le même que pour les heures
        {
          if (Clock.getSecond()<10)
          {
            lcd.print("0");
          }
          lcd.print(Clock.getSecond());
        }
        else if (etatSwitch==2)
          lcd.print("  ");
        else if (etatSwitch!=2)
          {
            if (Clock.getSecond()<10)
            {
              lcd.print("0");
            }
            lcd.print(Clock.getSecond());
          }
       lcd.setCursor(2,1);
       lcd.print("Setting Time");
}

void affichageEtatN() //Même configuration qu'en haut sauf que cette fois on agit sur le tableau de structures "MesReveils"
{
        int i=etatSettings; //Pas nécessaire, juste pour la lisibilité
        lcd.setCursor(4,0);
        blinkState();
        if (state==1 && etatSwitch==0)
         {
            if (MesReveils[i-NbR].heures<10) //Si i vaut 2, et que NbR vaut 2, on agit sur la case 0 du tableau donc le premier réveil, et ainsi de suite
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
  if (etatSettings >= 0 && etatSettings < N) //Si l'état dans lequel on se trouve est compris entre 0 et le nombre d'états maximum possibles
    etatSettings++; //On monte les états
  else 
    etatSettings=0; //Sinon on revient à 0
  etatSwitch=0; //On replace aussi etatSwitch à 0
  lcd.clear(); //On efface l'écran pour éviter d'avoir des problèmes de restes sur l'écran
}

void actionSwitch()
{
  if (etatSettings >= 1) //Si on est sur un autre état que celui d'affichage de l'heure
  {
    if (etatSwitch >= 0 && etatSwitch < 2) //Si l'état dans lequel on se trouve est compris entre 0 et le nombre d'états maximum possibles (ici 2 pour heures/minutes/secondes ou ON OFF)
      etatSwitch++; //On augmente
    else 
      etatSwitch=0; //Sinon on revient à 0 (aux heures, donc)
  }
}

void actionPlus() //le bouton plus à des fonctions différentes selon l'état dans lequel on est
{
  if (etatSettings == 1)  //Si on est sur les paramètres de l'horloge
  {
    if (etatSwitch==0)
    {
      //pas d'appui sur switch et on fait monter les heures en appuyant sur plus
        if (Clock.getHour(h24, AM)+ 1>23) //Si jamais on dépasse 24h on revient à 0
            Clock.setHour(0);
        else
            Clock.setHour(Clock.getHour(h24, AM)+1);
    }
    if (etatSwitch==1) //1 appui sur switch et on fait monter les minutes en appuyant sur plus
    {
     
        if (Clock.getMinute()+1>59) //Si jamais on dépasse 59 minutes on revient à 0
          Clock.setMinute(0);
        else
          Clock.setMinute(Clock.getMinute()+1);
    }
    if (etatSwitch==2) //2 appuis sur switch et on fait monter les secondes en appuyant sur plus
    {
      
      if (Clock.getSecond()+1>59) //Si jamais on dépasse 59 secondes on revient à 0
         Clock.setSecond(0);
      else
         Clock.setSecond(Clock.getSecond()+1);
    }
  }
  if (etatSettings >= 2) //Si on est sur les paramètres des réveils 
  { 
    int i=etatSettings;
    if (etatSwitch==0) //pas d'appui sur switch et on fait monter les heures en appuyant sur plus
      MesReveils[i-NbR].heures+=1; 
    if (etatSwitch==1) //1 appui sur switch et on fait monter les minutes en appuyant sur plus
      MesReveils[i-NbR].minutes+=1;
    if (etatSwitch==2) //2 appuis sur switch et on active ou on éteint l'activation de l'alarme
    {
      MesReveils[i-NbR].set=1; //Si c'est le cas, le réveil est bien activé, on passe à 1
      lcd.clear(); //On rend l'affichage propre
    }
     if (MesReveils[i-NbR].minutes >= 60) //Si 60 minutes se sont écoulées, on incrémente les heures
        MesReveils[i-NbR].minutes=0; //On réinitialise les minutes égalemenT
    if (MesReveils[i-NbR].heures >=24) //Si 24 heures se sont écoulées, on revient à 0
        MesReveils[i-NbR].heures=0;
  }
}

void actionMoins() //Même principe que pour plus, mais dans l'autre sens
{
    if (etatSettings == 1)
    {
      if (etatSwitch==0)
      {
        
        //pas d'appui sur switch et on fait baisser les heures en appuyant sur plus
        if (Clock.getHour(h24, AM)- 1<0)
            Clock.setHour(23);
        else
          Clock.setHour(Clock.getHour(h24, AM)-1);
      }
      if (etatSwitch==1) //1 appui sur switch et on fait baisser les minutes en appuyant sur plus
      {
        if (Clock.getMinute()-1 < 0)
            Clock.setMinute(59);
        else
            Clock.setMinute(Clock.getMinute()-1);
      }
      if (etatSwitch==2) //2 appuis sur switch et on fait baisser les secondes en appuyant sur plus
      {
        if (Clock.getSecond()-1 < 0)
            Clock.setSecond(59);
        else
            Clock.setSecond(Clock.getSecond()-1);
      }
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
       if (MesReveils[i-NbR].minutes < 0) //Si on revient en arrière sous les 0 minutes, on remonte à 59
            MesReveils[i-NbR].minutes=59;
       if (MesReveils[i-NbR].heures < 0) //Si on revient en arrière sous les 0 heures, on remonte à 23
            MesReveils[i-NbR].heures=23;
    }
}

void blinkState() //Petit sous programme permettant de faire passer un booléen de 0 à 1 pour montrer s'il s'est écoulé 750 millisecondes
{
  if(ul_Temps - ul_Blink > 750) //Si le différentiel est de 750 ms
   {
        ul_Blink=ul_Temps;
        if (state==0)
            state=1;
         else
            state=0;
    }
}

void sleepState() //Pour mettre en place le mode
{
  
  if(ul_Temps - ul_Sleep > 20000) //Si le différentiel est de 20 secondes
    sleep=1;    
  if (sleep)
  {
    lcd.noBacklight();  //On coupe le rétroéclairage
  }
  else
    lcd.backlight(); //On rallume l'éclairage
  
}

void alarm()
{
    for ( int i =0 ; i<NbR ; ++i)
      if (MesReveils[i].heures==Clock.getHour(h24, AM)&&MesReveils[i].minutes==Clock.getMinute()&&Clock.getSecond()==0&&MesReveils[i].set)  //Si jamais le reveil est à la même heure que celle actuelle
      {         
           sonne=1; //On met le booléen à 1
           digitalWrite(pinRelais, HIGH); //On active la carte MP3
           lcd.backlight(); //On rallume l'éclairage
      }
}
