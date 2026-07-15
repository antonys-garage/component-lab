// MAX7219 Demo Collection Skeleton for ESP32-C3
// Wiring:
// DIN=GPIO7 CLK=GPIO6 CS=GPIO5
// Button GPIO2 to GND to change modes

#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define DATA_PIN 7
#define CLK_PIN 6
#define CS_PIN 5
#define BTN_PIN 2

MD_MAX72XX mx(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

int mode=0,last=HIGH;

void clear(){mx.clear();}

void matrixRain(){
  static int d[32]; static bool init=false;
  if(!init){for(int i=0;i<32;i++) d[i]=random(-8,8); init=true;}
  mx.clear();
  for(int x=0;x<32;x++){
    if(d[x]>=0&&d[x]<8) mx.setPoint(d[x],x,true);
    if(d[x]-1>=0) mx.setPoint(d[x]-1,x,true);
    d[x]++; if(d[x]>12)d[x]=random(-8,0);
  }
  mx.update(); delay(60);
}

void bounce(){
  static int x=0,dx=1;
  mx.clear(); mx.setPoint(3,x,true); mx.update();
  x+=dx; if(x==31||x==0) dx=-dx;
  delay(40);
}

void randomPixels(){
  mx.clear();
  for(int i=0;i<40;i++) mx.setPoint(random(8),random(32),true);
  mx.update(); delay(120);
}

void setup(){
  pinMode(BTN_PIN,INPUT_PULLUP);
  randomSeed(analogRead(0));
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY,3);
}

void loop(){
  int s=digitalRead(BTN_PIN);
  if(last==HIGH && s==LOW){mode=(mode+1)%3; mx.clear(); delay(200);}
  last=s;
  switch(mode){
    case 0: matrixRain(); break;
    case 1: bounce(); break;
    case 2: randomPixels(); break;
  }
}
