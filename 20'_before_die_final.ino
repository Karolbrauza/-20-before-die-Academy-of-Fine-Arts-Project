/*
 * --------------------Projekt - "20' before die"--------------------
 ********************************************************************
 *Opis dzialania:
 *Kod realizuje zapalanie diod w momencie kiedy zostanie wykryty przeplyw wody,
 *do komunikacji z przeplywomierzem zaadoptowano wzorcowy kod dostepny pod adresem:
 *https://www.instructables.com/How-to-Use-Water-Flow-Sensor-Arduino-Tutorial/
 *(zachowano opisy w j. angielskim wzbogcne o polskie komentarze)
 *kod wykorzystuje rowniez obsluge przerwan do zliczania impulsow z przeplywomierza,
 *przyblizenie dzialania przerwan pod adresem:
 *https://forbot.pl/blog/kurs-arduino-ii-przerwania-kontaktron-czujnik-pir-id16792
 
 *Czas pelnego zapalenia realizowany w 3 etapach:
 *1. W 10s zapala sie 1/3  w kolejne 10s pozostale 2/3, kolejno miganie ledow oswietlajacych "20" na lustrze i stopniowo pekniecia
 *2. W 10s zapala sie 2/3  w kolejne 10s pozostale 1/3, kolejno miganie ledow oswietlajacych "20" na lustrze i stopniowo pekniecia
 *3. W 20s zapala sie 3/3  , kolejno miganie ledow oswietlajacych "20" na lustrze i stopniowo pekniecia
 
 *Wznowienie od punktu zatrzymania obowiazuje do 5min od przerwania przeplywu wody, po tym czasie uklad sie zeruje i odlicza od nowa
 
 *Autor: Karol Brauza
 */

#include <Adafruit_NeoPixel.h> //biblioteka do oblsugi diod
#define PIN 6 // pin sygnalowy dla diod
#define NUMPIXELS 239 // liczba diod
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); //deklaracja paska ledowego o nazwie"pixels"
uint32_t white = pixels.Color(255, 178, 102); // zmienna "white" przechowujaca kolor swiecenia kazdej z diod (r, g, b) w zakresie 0-255
uint32_t black = pixels.Color(0, 0, 0);
int al = 239; // zmienna pomocnicza do odliczania zapalania diod
int al1 = 239;
int al2 = 239;
byte statusLed    = 13;

byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;


unsigned long oldTime;
unsigned long oldTime1;
unsigned long oldTime2;

   unsigned long staryCzas;
void setup() 
{
  pixels.begin(); 
  
  // Initialize a serial connection for reporting values to the host
  Serial.begin(9600);
   
  // Set up the status LED line as an output
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING); //wywołanie fukncji pulseCounter, gdy na pinie 2 następuje zmiana wartości ze stanu wysokiego na niski.

}

void twenties()
{

      for(int i=0; i<2; i++)
      {
      delay(400);
      pixels.fill(white, 104, 6);
      pixels.fill(white, 72, 6);
      pixels.fill(white, 42, 6);
      pixels.show();
      delay(400);
      pixels.fill(black, 104, 6);
      pixels.fill(black, 72, 6);
      pixels.fill(black, 42, 6);
      pixels.show();
      }
}
void cracks()
{
      for(int i=110;i>0;i--)
      {
      pixels.setPixelColor(i, white);
      pixels.show();
      delay(90);
      }
}

void cracks1()
{
      int j = 0;
      for(int i=37;i>0;i--)
      {
      j=j+3;
      pixels.setPixelColor(j, white);
      pixels.show();
      delay(270);
      }
}


void cracks2()
{
      int j = 0;
      for(int i=55;i>0;i--)
      {
      j=j+2;
      pixels.setPixelColor(j, white);
      pixels.show();
      delay(181);
      }
}

void firstStage()
{

    if((flowRate > 0) && (al>197)){
      if((millis() - staryCzas) > 238)
      {
      Serial.print("Stary czas: ");
      Serial.print(staryCzas);
      pixels.setPixelColor(al, white);
      pixels.show();
      al--;
      staryCzas = millis();
      }else if((flowRate > 0) && (al<199)){
      cracks1(); 
      }

}
}

void secondStage()
{
  
    if((flowRate > 0) && (al>155)){
      if((millis() - staryCzas) > 238)
      {
      Serial.print("Stary czas: ");
      Serial.print(staryCzas);
      pixels.setPixelColor(al, white);
      pixels.show();
      al--;
      staryCzas = millis();
      }else if((flowRate > 0) && (al<158)){
      cracks2(); 
      }

}
}


void thirdStage()
{

      if((flowRate > 0) && (al>110)){
      if((millis() - staryCzas) > 238)
      {
      Serial.print("Stary czas: ");
      Serial.print(staryCzas);
      pixels.setPixelColor(al, white);
      pixels.show();
      al--;
      staryCzas = millis();
      }else if((flowRate > 0) && (al<115)){
      cracks(); 
      pixels.clear();
      al=239; 
      }

}
}

void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}

void loop() {

//Poniższy kod odpowiada za obsluge przeplywomierza wraz z wyswietlaniem parametrow przeplywu w monitorze portu szeregowego
   if((millis() - oldTime) > 200)    // Only process counters once per second
  { 
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    //flowRate=1;
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
      
    unsigned int frac;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");        
    Serial.print(totalMilliLitres);
    Serial.println("mL"); 
    Serial.print("\t");       // Print tab space
  Serial.print(totalMilliLitres/1000);
  Serial.print("L");


    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }

/* Kod odpowiedzialny za zapalanie ledow*/
   //if((millis() - oldTime1) > 10){ 
    if(flowRate>0){
      pixels.fill(white, al, al1);
     if(al>197) {
      Serial.print(millis() - oldTime2);
      Serial.print("s1\n");
      firstStage();
      }else if((al<198) && (al>155)){
      Serial.print(millis() - oldTime2);
      Serial.print("s2\n");
      secondStage();  
      }else if((al <156) && (al>110)){
      Serial.print(millis() - oldTime2);
      Serial.print("s3\n");
      thirdStage();
      }else if(al <=3){
      al=239;
      oldTime2 = millis();
      }
      oldTime1 = millis();
    }else if(flowRate < 1){
      pixels.clear();
      pixels.show();
      al1=239-al;   
    }
//}
}
