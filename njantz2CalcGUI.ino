
/* Nathan Jantz
 * njantz2
 * 654782946
 * CalcGUI for Spring 2019 Semester Project
 * 
 * Setup:
 * Setup is for 2.4" TFT touchscreen with an Arduino Mega 2560.
 * 
 * References Used:
 * https://circuitdigest.com/microcontroller-projects/arduino-touch-screen-calculator-tft-lcd-project-code
 * https://www.youtube.com/watch?v=9Ms59ofSJIY&t=300s
 * https://www.youtube.com/watch?v=g5zrq4t6tH4
 * https://github.com/JoaoLopesF/SPFD5408
 * 
 ***************Abstract******************
 * The purpose of this project is to create a functional calculator that uses touch screen  
 * GUI’s to receive inputs and display outputs.
 * 
 * This calculator will be able to graph functions and perform basic arithmetic. 
 *
 * The basic design will use three Arduinos that share data through serial communication. 
 * Two touch screen LCD's, an LED, a buzzer, a 16x2 LCD, and a potentiometer will be
 * used to receive user input and display output.

 *****************************************/

#include <pin_magic.h>
#include <SPFD5408_Adafruit_GFX.h>
#include <SPFD5408_TouchScreen.h>
#include <SPFD5408_Adafruit_TFTLCD.h>
#include <registers.h>
#include <SPFD5408_Util.h>
#include <SoftwareSerial.h>

//Screen Calibration
#define TS_MINX 125
#define TS_MINY 85
#define TS_MAXX 965
#define TS_MAXY 905

//Define Colors
#define BLACK   0x0000 //Black->White
#define BLUE    0x0017 //Blue->Yellow
#define RED     0xF800 //Red->Cyan
#define GREEN   0x0500 //Green-> Pink
#define CYAN    0x07FF //Cyan -> Red
#define PINK 0xF81F //Pink -> Green
#define YELLOW  0xFFE0 //Yellow->Blue
#define WHITE   0xFFFF //White-> Black

//Assign Pressure Settings
#define MINPRESSURE 10
#define MAXPRESSURE 1000
#define Sens 300

//Define LCD Pins
#define YP A3
#define XM A2
#define YM 9
#define XP 8
#define LCD_CS A3
#define LCD_RS A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

//Touchscreen and Communication Setup
TouchScreen ts = TouchScreen(XP, YP, XM, YM, Sens);
Adafruit_TFTLCD tft(LCD_CS, LCD_RS, LCD_WR, LCD_RD, LCD_RESET);
//SoftwareSerial uno1(19, 18); //rx, tx
//SoftwareSerial mega2(17,16);
char myByte[3];

//*********************************variables*********************************
//for menu display
String calcSymbol1[5][4] = {
//col 0     1     2     3
   {"<",  ">",  "<-", "C"}, //row 4
   {"7",  "8",  "9",  "/"}, //row 3
   {"4",  "5",  "6",  "*"}, //row 2
   {"1",  "2",  "3",  "-"}, //row 1
   {"0",  ".",  "=",  "+"}  //row 0
};
String calcSymbol2[5][4] = {
//col 0     1     2     3
   {"<",  ">",  "<-", "C"},  //row 4
   {"7",  "8",  "9",  "SI"},//row 3
   {"4",  "5",  "6",  "CO"},//row 2
   {"1",  "2",  "3",  "TA"},//row 1
   {"0",  ".",  "=",  "^"}   //row 0
};
String mainSymbol [3][1] = {
  {"CALCULATOR"},
  {"GRAPH MENU"},
};

//for touch detection
int X, Y;

//for error LED
int led = 53;

//for touch buzzer
int buzzer = 31;

//for calculator & communication
int zeroPressed = 0;
float number1, number2 = 0.0;
float Result = 0.0;
String Trig;
char oper;
boolean dec = false; 
boolean op = false; // +,-,%,*,()
boolean resultDisplay = false; 
boolean trigDisplay = false; 

//for settings
boolean BasicSet = true;
boolean SciSet = false;
boolean DegreeSet = true;
boolean RadianSet = false;

//for navigation
boolean graphMenu = false;
boolean calcMenu1 = false;
boolean calcMenu2 = false;
boolean mainMenu = true;


//*********************************Setup and Loop*********************************/
void setup(){
  Serial1.begin(9600);//communication and debugging purposes
  Serial.begin(9600); //communication with uno1
  Serial2.begin(9600); //communication setup with mega2
  tft.reset();       //Reset at start
  tft.begin(0x9341); //LCD used interface
  tft.setRotation(2); //Rotate screen
  tft.fillScreen(BLACK);

  pinMode(led,OUTPUT); //error message
  pinMode(buzzer, OUTPUT); //touch tone
}

void loop() {
  //sketch GUI
  DrawButtons();
  
  //Reads user's touch and changes X and Y accordingly
  TSPoint p = waitTouch();
  X = p.y;
  Y = p.x;
  //Performs appropriate changes to all variables
  DetectButtons();

  //debugging
  Serial.println(" ");
  
  //Send operation string to Uno
  SendString();

  //delay for communication
}

//*****************************End Setup and Loop*********************************/


/* Define all functions
 * 
 * waitTouch()
 * Finds and returns point user has touched on screen.
 * (0 <= x <= 320)
 * (0 <= y <= 240)
 */
TSPoint waitTouch(){
  TSPoint p; //point to be returned
  
  //find point while checking for valid pressure
  do{
    p = ts.getPoint();
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
  }while ((p.z < MINPRESSURE) || (p.z > MAXPRESSURE));

  //Button pressed--send and recieve volume data
  Volume();
  
  //set (p.x, p.y) coordinates
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

  return p;
}

/*
 * DetectMain()
 * Detects which button is pressed by user in main menu.
 */
void DetectMain(){
  if(X<250 && X>125){ //calc button pressed
    mainMenu = false;
    calcMenu1 = true;
    graphMenu = false;
    calcMenu2 = false;
  }
  else{ //graph button pressed
    mainMenu = false;
    calcMenu1 = true;
    graphMenu = true;
    calcMenu2 = false;
  }
}

/*
 * DetectGraph()
 * Detects which button is pressed by user in graph menu.
 */
void DetectGraph(){
  DetectCalc();
}

/*
 * DetectCalc()
 * Detects which button is pressed by user in calculator.
 */
void DetectCalc(){
  if(Y<75 && Y>0){ //column 0
    if(X<250 && X>200){ //row 0
      //back
      //switch menu
      if(calcMenu1 == true){
        //erase data
        number1=number2=Result = 0.0;
        zeroPressed = 0;
        resultDisplay = false;
        trigDisplay = false;
        op = false;
        dec = false;
        //menu switch
        calcMenu1 = false;
        mainMenu = true;
        calcMenu2 = false;
        graphMenu = false;
      }
      else{
        //menu switch
        calcMenu2 = false;
        calcMenu1 = true;
      }
    }//back
    else if(X<200 && X>150){ //row 1
      //if result is being displayed, reset
      if(resultDisplay == true){
        resultDisplay = trigDisplay = op = dec = false;
        number1 = number2 = Result = 0.0;
        zeroPressed = 0;
        trigDisplay = false;
      }
      //continue
      //seven
      if(op == false || trigDisplay == true){   //no operation
        if(dec == false){ //no decimal
          number1 = (number1*10) + 7;
        }
        else{ //decimal has been pressed
          zeroPressed++;
          number1 = number1 + (7.0/float(pow(10,zeroPressed)));
        }
      }
      else{               //operation exists
        if(dec == false){ //no decimal
          number2 = (number2*10) + 7;
        }
        else{ //decimal has been pressed
            zeroPressed++;
            number2 = number2 + (7.0/float(pow(10,zeroPressed)));
        }
      }
    }//seven
    else if(X<150 && X>100){ //row 2
      //if result is being displayed, reset
      if(resultDisplay == true){
        resultDisplay = trigDisplay = op = dec = false;
        number1 = number2 = Result = 0.0;
        zeroPressed = 0;
        trigDisplay = false;
      }
      //continue
      //four
      if(op == false || trigDisplay == true){   //no operation
        if(dec == false){ //no decimal
          number1 = (number1*10) + 4;
        }
        else{ //decimal has been pressed
          zeroPressed++;
          number1 = number1 + (4/float(pow(10,zeroPressed)));
        }
      }
      else{               //operation exists
        if(dec == false){ //no decimal
          number2 = (number2*10) + 4;
        }
        else{ //decimal has been pressed
            zeroPressed++;
            number2 = number2 + (4/float(pow(10,zeroPressed)));
        }
      }
    }//four
    else if(X<100 && X>50){ //row 3
      //if result is being displayed, reset
      if(resultDisplay == true){
        resultDisplay = trigDisplay = op = dec = false;
        number1 = number2 = Result = 0.0;
        zeroPressed = 0;
        trigDisplay = false;
      }
      //continue
      //one
      if(op == false || trigDisplay == true){   //no operation
        if(dec == false){ //no decimal
          number1 = (number1*10) + 1;
        }
        else{ //decimal has been pressed
          zeroPressed++;
          number1 = number1 + (1/float(pow(10,zeroPressed)));
        }
      }
      else{               //operation exists
        if(dec == false){ //no decimal
          number2 = (number2*10) + 1;
        }
        else{ //decimal has been pressed
            zeroPressed++;
            number2 = number2 + (1/float(pow(10,zeroPressed)));
        }
      }
    }//one
    else if(X<50 && X>0){ //row 4
      //if result is being displayed, reset
      if(resultDisplay == true){
        resultDisplay = trigDisplay = op = dec = false;
        number1 = number2 = Result = 0.0;
        zeroPressed = 0;
        trigDisplay = false;
      }
      //continue
      //zero
      if(op == false || trigDisplay == true){ //no operation
        if(dec == false){ //no decimal
          number1 = (number1*10);
        }
        else{ //decimal has been pressed
          zeroPressed++;
        }
      }
      else{
        if(dec == false){ //no decimal
          number2 = (number2*10);
        }
        else{ //decimal has been pressed
          zeroPressed++;
        }
      }
    }//zero
  }//end of column 0
  
  else if(Y<150 && Y>75){ //column 1
    if(X<250 && X>200){ //row 0
      //next
      calcMenu2 = true;
      calcMenu1 = false;
    }//next
    else if(X<200 && X>150){ //row 1
      //if result is being displayed, reset
      if(resultDisplay == true){
        resultDisplay = trigDisplay = op = dec = false;
        number1 = number2 = Result = 0.0;
        zeroPressed = 0;
        trigDisplay = false;
      }
      //continue
      //eight
      if(op == false || trigDisplay == true){   //no operation
        if(dec == false){ //no decimal
          number1 = (number1*10) + 8;
        }
        else{ //decimal has been pressed
          zeroPressed++;
          number1 = number1 + (8/float(pow(10,zeroPressed)));
        }
      }
      else{               //operation exists
        if(dec == false){ //no decimal
          number2 = (number2*10) + 8;
        }
        else{ //decimal has been pressed
            zeroPressed++;
            number2 = number2 + (8/float(pow(10,zeroPressed)));
        }
      }
    }//eight
    else if(X<150 && X>100){ //row 2
      //if result is being displayed, reset
      if(resultDisplay == true){
        resultDisplay = trigDisplay = op = dec = false;
        number1 = number2 = Result = 0.0;
        zeroPressed = 0;
        trigDisplay = false;
      }
      //continue
      //five
      if(op == false || trigDisplay == true){   //no operation
        if(dec == false){ //no decimal
          number1 = (number1*10) + 5;
        }
        else{ //decimal has been pressed
          zeroPressed++;
          number1 = number1 + (5/float(pow(10,zeroPressed)));
        }
      }
      else{               //operation exists
        if(dec == false){ //no decimal
          number2 = (number2*10) + 5;
        }
        else{ //decimal has been pressed
            zeroPressed++;
            number2 = number2 + (5/float(pow(10,zeroPressed)));
        }
      }
    }//five
    else if(X<100 && X>50){ //row 3
      //if result is being displayed, reset
      if(resultDisplay == true){
        resultDisplay = trigDisplay = op = dec = false;
        number1 = number2 = Result = 0.0;
        zeroPressed = 0;
        trigDisplay = false;
      }
      //continue
      //two
      if(op == false || trigDisplay == true){   //no operation
        if(dec == false){ //no decimal
          number1 = (number1*10) + 2;
        }
        else{ //decimal has been pressed
          zeroPressed++;
          number1 = number1 + (2/float(pow(10,zeroPressed)));
        }
      }
      else{               //operation exists
        if(dec == false){ //no decimal
          number2 = (number2*10) + 2;
        }
        else{ //decimal has been pressed
            zeroPressed++;
            number2 = number2 + (2/float(pow(10,zeroPressed)));
        }
      }
    }//two
    else if(X<50 && X>0){ //row 4
      //if result is being displayed, reset
      if(resultDisplay == true){
        resultDisplay = trigDisplay = op = dec = false;
        number1 = number2 = Result = 0.0;
        zeroPressed = 0;
        trigDisplay = false;
      }
      //continue
      //decimal
      if (dec == true){
        SendError();
      }
      else{
        dec = true;
        zeroPressed = 0;
      }
    }//decimal
  }//end of column1
  
  else if(Y<225 && Y>150){ //column 2
    if(X<250 && X>200){ //row 0
      //backspace
      if(op == false){ //zero out number1
        if(dec == false){
          number1 = 0.0;
        }
        else{ //remove decimal value from number1
          int c = number1/1; //floored division
          number1 = c*1.0; //float number1 = ##.0
        }
      }
      else{
        if(dec == false){ //zero out number2 or remove operator
          if (number2 == 0.0){
            op = false;
          }
          else{
            number2 = 0.0;
          }
        }
        else{ //remove decimal value from number2
          int c = number2/1; //floored division
          number2 = c*1.0; //float number2 = ##.0
        }
      }
    }//backspace
    else if(X<200 && X>150){ //row 1
      //if result is being displayed, reset
      if(resultDisplay == true){
        resultDisplay = trigDisplay = op = dec = false;
        number1 = number2 = Result = 0.0;
        zeroPressed = 0;
        trigDisplay = false;
      }
      //continue
      //nine
      if(op == false || trigDisplay == true){   //no operation
        if(dec == false){ //no decimal
          number1 = (number1*10) + 9;
        }
        else{ //decimal has been pressed
          zeroPressed++;
          number1 = number1 + (9/float(pow(10,zeroPressed)));
        }
      }
      else{               //operation exists
        if(dec == false){ //no decimal
          number2 = (number2*10) + 9;
        }
        else{ //decimal has been pressed
            zeroPressed++;
            number2 = number2 + (9/float(pow(10,zeroPressed)));
        }
      }
    }//nine
    else if(X<150 && X>100){ //row 2
      //if result is being displayed, reset
      if(resultDisplay == true){
        resultDisplay = trigDisplay = op = dec = false;
        number1 = number2 = Result = 0.0;
        zeroPressed = 0;
        trigDisplay = false;
      }
      //continue
      //six
      if(op == false || trigDisplay == true){   //no operation
        if(dec == false){ //no decimal
          number1 = (number1*10) + 6;
        }
        else{ //decimal has been pressed
          zeroPressed++;
          number1 = number1 + (6/float(pow(10,zeroPressed)));
        }
      }
      else{               //operation exists
        if(dec == false){ //no decimal
          number2 = (number2*10) + 6;
        }
        else{ //decimal has been pressed
            zeroPressed++;
            number2 = number2 + (6/float(pow(10,zeroPressed)));
        }
      }
    }//six
    else if(X<100 && X>50){ //row 3
      //if result is being displayed, reset
      if(resultDisplay == true){
        resultDisplay = trigDisplay = op = dec = false;
        number1 = number2 = Result = 0.0;
        zeroPressed = 0;
        trigDisplay = false;
      }
      //continue
      //three
      if(op == false || trigDisplay == true){   //no operation
        if(dec == false){ //no decimal
          number1 = (number1*10) + 3;
        }
        else{ //decimal has been pressed
          zeroPressed++;
          number1 = number1 + (3/float(pow(10,zeroPressed)));
        }
      }
      else{               //operation exists
        if(dec == false){ //no decimal
          number2 = (number2*10) + 3;
        }
        else{ //decimal has been pressed
            zeroPressed++;
            number2 = number2 + (3/float(pow(10,zeroPressed)));
        }
      }
    }//three
    else if(X<50 && X>0){ //row 4
      //equal
      resultDisplay = true;
      CalculateResult();
    }//equal
  }//end of column 2
  
  else if(Y<300 && Y>225){ //column 3
    if(X<250 && X>200){ //row 0
      //clear
      //erase data
      number1=number2=Result = 0.0;
      zeroPressed = 0;
      resultDisplay = false;
      trigDisplay = false;
      op = false;
      dec = false;
    }//clear
    else if(X<200 && X>150){ //row 1
      //divide or sin
      if(op == true){//operation already exists
        if (number2 != 0.0){
          CalculateResult();
          number1 = Result;
          number2 = 0.0;
          Result = 0.0;
          op = false;
          resultDisplay = false; //number1 is result
          trigDisplay = false;
        }
      }
      //set new operator;
      if(calcMenu1 == true){
        op = true;
        oper = '/';
        dec = false;
        zeroPressed = 0;
      }
      else if(calcMenu2 == true && number1 == 0.0){
        op = true;
        oper = 's';
        dec = false;
        zeroPressed = 0;
        trigDisplay = true;
        Trig = "sin ";
      }
      else{
        SendError();
      }
    }//divide or sin
    else if(X<150 && X>100){ //row 2
      //multiply or cos
      if(op == true){//operation already exists
        if (number2 != 0.0){
          CalculateResult();
          number1 = Result;
          number2 = 0.0;
          Result = 0.0;
          op = false;
          resultDisplay = false; //number1 is result
          trigDisplay = false;
        }
      }
      //set new operator;
      if(calcMenu1 == true){
        op = true;
        oper = '*';
        dec = false;
        zeroPressed = 0;
      }
      else if(calcMenu2 == true && number1 == 0.0){
        op = true;
        oper = 'c';
        dec = false;
        zeroPressed = 0;
        trigDisplay = true;
        Trig = "cos ";
      }
      else{
        SendError();
      }
    }//multiply or cos
    else if(X<100 && X>50){ //row 3
      //subtract or tan
      if(op == true){//operation already exists
        if (number2 != 0.0){
          CalculateResult();
          number1 = Result;
          number2 = 0.0;
          Result = 0.0;
          op = false;
          resultDisplay = false; //number1 is result
          trigDisplay = false;
        }
      }
      //set new operator;
      if(calcMenu1 == true){
        op = true;
        oper = '-';
        dec = false;
        zeroPressed = 0;
      }
      else if(calcMenu2 == true && number1 == 0.0){
        op = true;
        oper = 't';
        dec = false;
        zeroPressed = 0;
        trigDisplay = true;
        Trig = "tan ";
      }
      else{
        SendError();
      }
    }//subtract or tan
    else if(X<50 && X>0){ //row 4
      //addition or power
      if(op == true){//operation already exists
        if (number2 != 0.0){
          CalculateResult();
          number1 = Result;
          number2 = 0.0;
          Result = 0.0;
          op = false;
          resultDisplay = false; //number1 is result
          trigDisplay = false;
        }
      }
      //set new operator;
      if(calcMenu1 == true){
        op = true;
        oper = '+';
        dec = false;
        zeroPressed = 0;
      }
      else if(calcMenu2 == true){
        op = true;
        oper = '^';
        dec = false;
        zeroPressed = 0;
      }
      else{
        SendError();
      }
    }//addition or power
  }//end of column 3
}

/*
 * DetectButtons()
 * Uses boolean values mainMenu, graphMenu, and calcMenu
 * to determine which Menu to use for user touch.
 */
void DetectButtons(){
  if(mainMenu == true){
    DetectMain();
  }
  else if(calcMenu1 == true){
    DetectCalc();
  }
  else if(calcMenu2 == true){
    DetectCalc();
  }
  else if(graphMenu == true){
    DetectGraph();
  }
  else{ //error occured, reset
    SendError();
  }
}

/*
 * CalculateResult()
 * Interprets existing variables and sends both operation
 * and result to the LCD on Arduino Uno #2
 */
void CalculateResult(){
  if(op == true){
    if(oper == '+'){
      Result = number1 + number2;
    }
    else if(oper == '-'){
      Result = number1 - number2;
    }
    else if(oper == '*'){
      Result = number1 * number2;
    }
    else if(oper == '/'){
      if (number2 == 0.0){
        SendError();
      }
      else{
        Result = number1 / number2;
      }
    }
    else if(oper == 's'){
      Result = sin(number1);
    }
    else if(oper == 'c'){
      Result = cos(number1);
    }
    else if(oper == 't'){
      if(graphMenu == true){
        SendError();
      }
      else{
        Result = tan(number1);
      }
    }
    else if(oper == '^'){
      Result = pow(number1, number2);
    }
    else{
      SendError();
    }
  }
  else if(op == false){
    Result = number1;
  }
  else{
    SendError();
    return;
  }
  //send result if display is true
  if(resultDisplay == true){
    SendResult();
  }
}

/*
 * SendResult()
 * Sends Result to Arduino Uno #2
 * for 16x2 LCD.
 */
void SendResult(){
  if(graphMenu == true){
    SendGraph();
  }
  else{
    resultDisplay == true;
    //TODO: Send to Uno#1
    Serial.print("Result is: "); Serial.println(Result, 2);

  }
}

/*
 * SendString()
 * Sends operation string to Arduino Uno #2
 * for 16x2 LCD.
 */
void SendString(){
  String opString;
  /*debugging
  Serial.print("(X,Y): "); Serial.print("(");
  Serial.print(X); Serial.print(','); Serial.print(Y); Serial.println(")");
  Serial.println("GraphMenu: " + String(graphMenu));
  Serial.println("MainMenu: " + String(mainMenu));
  Serial.println("Calc1Menu: " + String(calcMenu1));
  Serial.println("Calc2Menu: " + String(calcMenu2));
  //end of debugging*/
  if(graphMenu != true){
    if(op == false && trigDisplay == false){
      opString = String(number1,2);
      //Serial.print("Number1: "); Serial.println(number1,2);
      Serial.println(opString);
    }
    else{
      if(op == true && trigDisplay == false){
        opString = String(number1, 2) + oper + String(number2,2);
        Serial.print(number1,2); Serial.print(" "); Serial.print(oper); Serial.print(" "); Serial.println(number2, 2);
      }
      else if(op == true && trigDisplay == true){
        if(oper == 's'){
          opString = "Sin " + String(number1,2);
        }
        else if(oper == 'c'){
          opString = "Cos " + String(number1,2);
        }
        else if(oper == 't'){
          opString = "Tan " + String(number1, 2);
        }
        else{
          SendError();
        }
        Serial.print(Trig); Serial.print(" "); Serial.println(number1, 2);
      }
      else{
        SendError();
      }
    }
  }
  else{
    if(op == false && trigDisplay == false){
      opString = String(number1,2);
      //Serial.print("Number1: "); Serial.println(number1,2);
      Serial.println(opString);      
    }
    else{
      if(op == true && trigDisplay == false){
        opString = "Y = " + String(number1, 2) + oper + String(number2,2);
        Serial.print(number1,2); Serial.print(" "); Serial.print(oper); Serial.print(" "); Serial.println(number2, 2);
      }
      else if(op == true && trigDisplay == true){
        if(oper == 's'){
          opString = String("Y = ") + String("Sin ") + String(number1,2);
        }
        else if(oper == 'c'){
          opString = String("Y = ") + String("Cos ") + String(number1,2);
        }
        else if(oper == 't'){
          opString = "Tan x not available. Resetting...";
        }
        else{
          SendError();
        }
      }
      else{
        SendError();
      }
    }
  }
  if(resultDisplay == true){
    opString = String(Result, 4);
  }
  Serial1.print(opString);
}

/*
 * SendGraph()
 * Sends graph equation to Arduino Mega #2
 * for display.
 */
void SendGraph(){
  //TODO
  //Serial2.write(
  Serial.println("Sent to graph");
  if(op == true && trigDisplay == true){
    if(oper == 's' && number1 == 1.0){
      Serial2.write("s1");
      delay(10);
    }
    else if(oper == 's' && number1 == 2.0){
      Serial2.write("s2");
      delay(10);
    }
    else if(oper == 's' && number1 == 3.0){
      Serial2.write("s3");
      delay(10);
    }
    else if(oper == 's' && number1 == 4.0){
      Serial2.write("s4");
      delay(10);
    }
    else if(oper == 's' && number1 == 5.0){
      Serial2.write("s5");
      delay(10);
    }
    else if(oper == 's' && number1 == 6.0){
      Serial2.write("s6");
      delay(10);
    }
    else if(oper == 's' && number1 == 7.0){
      Serial2.write("s7");
      delay(10);
    }
    else if(oper == 's' && number1 == 8.0){
      Serial2.write("s8");
      delay(10);
    }
    else if(oper == 's' && number1 == 9.0){
      Serial2.write("s9");
      delay(10);
    }
    else if(oper == 's' && number1 == -1.0){
      Serial2.write("s-1");
      delay(10);
    }
    else if(oper == 's' && number1 == -2.0){
      Serial2.write("s-2");
      delay(10);
    }
    else if(oper == 's' && number1 == -3.0){
      Serial2.write("s-3");
      delay(10);
    }
    else if(oper == 's' && number1 == -4.0){
      Serial2.write("s-4");
      delay(10);
    }
    else if(oper == 's' && number1 == -5.0){
      Serial2.write("s-5");
      delay(10);
    }
    else if(oper == 's' && number1 == -6.0){
      Serial2.write("s-6");
      delay(10);
    }
    else if(oper == 's' && number1 == -7.0){
      Serial2.write("s-7");
      delay(10);
    }
    else if(oper == 's' && number1 == -8.0){
      Serial2.write("s-8");
      delay(10);
    }
    else if(oper == 's' && number1 == -9.0){
      Serial2.write("s-9");
      delay(10);
    }
    else if(oper == 'c' && number1 == 1.0){
      Serial2.write("c1");
      delay(10);
    }
    else if(oper == 'c' && number1 == 2.0){
      Serial2.write("c2");
      delay(10);
    }
    else if(oper == 'c' && number1 == 3.0){
      Serial2.write("c3");
      delay(10);
    }
    else if(oper == 'c' && number1 == 4.0){
      Serial2.write("c4");
      delay(10);
    }
    else if(oper == 'c' && number1 == 5.0){
      Serial2.write("c5");
      delay(10);
    }
    else if(oper == 'c' && number1 == 6.0){
      Serial2.write("c6");
      delay(10);
    }
    else if(oper == 'c' && number1 == 7.0){
      Serial2.write("c7");
      delay(10);
    }
    else if(oper == 'c' && number1 == 8.0){
      Serial2.write("c8");
      delay(10);
    }
    else if(oper == 'c' && number1 == 9.0){
      Serial2.write("c9");
      delay(10);
    }
    else if(oper == 'c' && number1 == -1.0){
      Serial2.write("c-1");
      delay(10);
    }
    else if(oper == 'c' && number1 == -2.0){
      Serial2.write("c-2");
      delay(10);
    }
    else if(oper == 'c' && number1 == -3.0){
      Serial2.write("c-3");
      delay(10);
    }
    else if(oper == 'c' && number1 == -4.0){
      Serial2.write("c-4");
      delay(10);
    }
    else if(oper == 'c' && number1 == -5.0){
      Serial2.write("c-5");
      delay(10);
    }
    else if(oper == 'c' && number1 == -6.0){
      Serial2.write("c-6");
      delay(10);
    }
    else if(oper == 'c' && number1 == -7.0){
      Serial2.write("c-7");
      delay(10);
    }
    else if(oper == 'c' && number1 == -8.0){
      Serial2.write("c-8");
      delay(10);
    }
    else if(oper == 'c' && number1 == -9.0){
      Serial2.write("c-9");
      delay(10);
    }
  }
  else{
    SendError();
  }
}

/*
 * Volume()
 * 
 */
void Volume(){
//TODO
tone(buzzer,900);
delay(20);
noTone(buzzer);
delay(120);
  
  /*
    vol = val1 - 48;
  }
    else if(Serial1.available() == 2){
    val1 = Serial1.read();
    val2 = Serial1.read();
    delay(50);

    vol = ((val1-48) * 10) + (val2-48);
  }
    else if(Serial1.available() == 3){
    val1 = Serial1.read();
    val2 = Serial1.read();
    val3 = Serial1.read();
    delay(50);

    vol = ((val1-48)*100) + ((val2-48) * 10) + (val3-48);
  }
  else{
    Serial1.flush();
    Volume();
  }
  Serial.println(val1);
  Serial.println(val2);
  Serial.println(val3);
  Serial.println(vol);*/
}

/*
 * SendError()
 * Sends current boolean value for Error variable
 * to Arduino Uno #1.
 */
void SendError(){
  //reset variables
  Reset();

  //Light LED for 3 seconds
  //TODO
  
  Serial.println("Error occured. Resetting...");
  Serial1.print("Error occured. Resetting...");
  digitalWrite(led,HIGH);
  delay(3000);
  digitalWrite(led,LOW);
}

void Reset(){
  //reset variables
  mainMenu = true;
  graphMenu = false;
  calcMenu1 = false;
  calcMenu2 = false;
  number1 = number2 = Result = 0.0;
  op = false;
  dec = false;
  resultDisplay = false;
  trigDisplay = false;
  zeroPressed = 0;
}

/*
 * DrawButtons()
 * Draws appropriate buttons given status of menu variables
 * graphMenu, calcMenu1, mainMenu, calcMenu2, settingsMenu
 */
void DrawButtons(){
  if(mainMenu == true){
    DrawMain();
  }
  else if(calcMenu1 == true){
    DrawCalc();
  }
  else if(calcMenu2 == true){
    DrawCalc();
  }
  else if(graphMenu == true){
    DrawCalc();
  }
  /*
  else if(settingsMenu == true){
    DrawSettings();
  }*/
  else{ //error occured, reset
    mainMenu = true;
  }
}

/*
 * DrawMain()
 * Draws Main Menu
 */
void DrawMain(){

  //Draw Buttons
  tft.fillRect (0,0,240,160,BLUE);
  tft.fillRect (0,160,240,320,GREEN);

  //Draw Horizontal Lines
  for (int y = 160; y <= 320; y+=160){
    tft.drawFastHLine (0, y, 240, WHITE);
  }
  
  //Display buttons
  for (int i=0; i <= 1; i++){
    tft.setCursor(25, 80 + (160*i));
    tft.setTextSize(3);
    tft.setTextColor(WHITE);
    tft.println(mainSymbol[i][0]);
  }
}

/*
 * DrawCalc()
 * Draws Calculator 1 or 2
 */
void DrawCalc(){
  //Draw First Column
  tft.fillRect (0,0,60,64,BLUE);
  tft.fillRect (0,64,60,64,BLACK);
  tft.fillRect (0,128,60,64,BLACK);
  tft.fillRect (0,192,60,64,BLACK);
  tft.fillRect (0,256,60,64,BLACK);
  //Draw Second Column
  tft.fillRect (60,0,60,64,BLUE);
  tft.fillRect (60,64,60,64,BLACK);
  tft.fillRect (60,128,60,64,BLACK);
  tft.fillRect (60,192,60,64,BLACK);
  tft.fillRect (60,256,60,64,BLUE);
  //Draw Third Column
  tft.fillRect (120,0,60,64,BLUE);
  tft.fillRect (120,64,60,64,BLACK);
  tft.fillRect (120,128,60,64,BLACK);
  tft.fillRect (120,192,60,64,BLACK);
  tft.fillRect (120,256,60,64,GREEN);
  //Draw Fourth Column
  tft.fillRect (180,0,60,64,RED);
  tft.fillRect (180,64,60,64,BLUE);
  tft.fillRect (180,128,60,64,BLUE);
  tft.fillRect (180,192,60,64,BLUE);
  tft.fillRect (180,256,60,64,BLUE);

  //Draw Horizontal Lines
  for (int y = 64; y < 320; y+=64){
    tft.drawFastHLine (0, y, 240, WHITE);
  }

  //Draw Vertical Lines
  for (int x = 60; x < 240; x+=60){
    tft.drawFastVLine (x, 0, 320, WHITE);
  }

  //Display buttons
  for (int i=0; i < 4; i++){
    for (int j=0; j < 5; j++){
      if ((j==0) && (i==2)){ //backspace character
        tft.setCursor(11 + (60*i), 20 + (64*j));
      }
      else{
        tft.setCursor(22 + (60*i), 20 + (64*j));
      }
      tft.setTextSize(3);
      tft.setTextColor(WHITE);
      if(calcMenu1==true){
        tft.println(calcSymbol1[j][i]);
      }
      else{
        tft.println(calcSymbol2[j][i]);
      }
    }
  }
}
