#include <Wire.h>
#include <LiquidCrystal_I2C.h> //Mise en place des librairies

#define NbR 2
#define N NbR+1

LiquidCrystal_I2C lcd(0x3F, 16, 2); //Initialisation de l'i2c avec l'écran
  
unsigned long ul_Temps = 0UL;
unsigned long ul_Tempsnouveau = 0UL;
unsigned long ul_Blink= 0UL;
bool state = 0;
int secondes=0;
int minutes=0;
int heures=0;
int etatSettings = 0;
int etatSwitch = 0;

int pinRelais = 4;


typedef struct bouton{ 
  int pin;
  bool actif=0;
};
bouton MesBoutons[4];

typedef struct reveil{
  int heures=0;
  int minutes=0;
  int set=0;
  int actif=0;
};
reveil MesReveils[NbR];

void setup() {
  
  MesBoutons[0].pin = 6;
  MesBoutons[1].pin = 7;
  MesBoutons[2].pin = 8;
  MesBoutons[3].pin = 9;
  //Pour le LCD
  
  lcd.init(); //Initialisation du lcd
  lcd.backlight(); //Démarrage du rétroéclairage
  

  pinMode(pinRelais, OUTPUT);

  for (int i=0 ; i<4 ; ++i)
  {
    pinMode( MesBoutons[i].pin, INPUT);
    digitalWrite( MesBoutons[i].pin, HIGH);
  }
  Serial.begin(9600);
}

void loop() {
 
 ul_Temps=millis(); //On récupère le temps depuis le lancement de l'arduino

 for (int i=0 ; i<4 ; ++i) //On teste si les boutons sont appuyés
 {
   if (digitalRead(MesBoutons[i].pin)==LOW&&MesBoutons[i].actif==0){
    MesBoutons[i].actif=1;
    if (i==0)
      actionSettings();
    if (i==1)
      actionSwitch();
    if (i==2)
      actionMoins();
    if (i==3)
      actionPlus();
   }
   else if (digitalRead(MesBoutons[i].pin)==HIGH&&MesBoutons[i].actif==1)
   {
    MesBoutons[i].actif=0;
   }
 }

 if (secondes >= 60) //Si 60 secondes se sont écoulées, on incrémente les minutes
    {   
      minutes+=1;
      secondes=0;
    }
 if (minutes >= 60) //Si 60 minutes se sont écoulées, on incrémente les heures
    {
      heures+=1;
      minutes=0;
    }
 if (heures >=24) //Si 24 heures se sont écoulées, on revient à 0
       heures=0;
 if (secondes < 0) //Si 60 secondes se sont écoulées, on incrémente les minutes
      secondes=59;
 if (minutes < 0) //Si 60 minutes se sont écoulées, on incrémente les heures
      minutes=59;
 if (heures < 0) //Si 24 heures se sont écoulées, on revient à 0
      heures=23;
 if (ul_Temps - ul_Tempsnouveau > 1000) //Si le temps entre les deux mesures de temps est supérieur à 1000 millisecondes
 {
    ul_Tempsnouveau=ul_Temps;
    secondes+=1;
 }
 if (ul_Temps < ul_Tempsnouveau)
 {
  ul_Tempsnouveau=0;
 }


 
 if (etatSettings==0)
    affichageEtat0(); //On affiche le mode voulu sur l'écran LCD
 if (etatSettings==1)
    affichageEtat1();
 if (etatSettings>=2)
    affichageEtatN();
   
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
