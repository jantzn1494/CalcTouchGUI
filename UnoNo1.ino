//Ebrahim Broachwala
//Code for Uno number 1

#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
//SoftwareSerial mega2(2,3);

LiquidCrystal lcd(12, 11, 10, 9, 8, 7);
String data = "No Serial Data";
int value;
String result;

void setup() {
  analogWrite(6, 100);
  lcd.begin(16, 2);
  //mega2.begin(9600);
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    data = Serial.readString();
    delay(50);
    //data.substring(0, data.length());
    //Serial.println(data);
  }

  value = map(analogRead(A0), 0, 1023, 0, 255);
  Serial.write(value);
  displayData();
  delay (100);
}

void displayData() {
  lcd.clear();
  String dx;
  if (value < 10)
    dx = "   " + String(value);
  else if (value < 100)
    dx = "  " + String(value);
  else if ( value < 1000)
    dx = " " + String(value);
  else
    dx = String(value);
  lcd.setCursor(0, 0);
  lcd.print(data);
  lcd.setCursor(0, 1);
  lcd.print(result);
}
