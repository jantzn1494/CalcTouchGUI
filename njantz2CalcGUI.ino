
/* Nathan Jantz
 * njantz2
 * Calculator GUI for touch screen
 * 
 * 
 * Setup:
 * Setup is for 2.4" TFT touch screen with an Arduino Mega 2560.
 * 
 * 
 * References Used:
 * https://circuitdigest.com/microcontroller-projects/arduino-touch-screen-calculator-tft-lcd-project-code
 * https://www.youtube.com/watch?v=9Ms59ofSJIY&t=300s
 * https://github.com/JoaoLopesF/SPFD5408
 */

#include <pin_magic.h>
#include <SPFD5408_Adafruit_GFX.h>
#include <SPFD5408_TouchScreen.h>
#include <SPFD5408_Adafruit_TFTLCD.h>
#include <registers.h>
#include <SPFD5408_Util.h>

//Screen Calibration
#define TS_MINX 125
#define TS_MINY 85
#define TS_MAXX 965
#define TS_MAXY 905

//Define Colors
#define WHITE   0x0000 //Black->White
#define YELLOW    0x001F //Blue->Yellow
#define CYAN     0xF800 //Red->Cyan
#define PINK   0x07E0 //Green-> Pink
#define RED    0x07FF //Cyan -> Red
#define GREEN 0xF81F //Pink -> Green
#define BLUE  0xFFE0 //Yellow->Blue
#define BLACK   0xFFFF //White-> Black

//Assign Pressure Settings
#define MINPRESSURE 10
#define MAXPRESSURE 1000
#define Sens 300

//Define LCD Pins
#define YP A1
#define XM A2
#define YM 7
#define XP 6
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

//Touchscreen and Communication Setup
TouchScreen ts = TouchScreen(XP, YP, XM, YM, Sens);
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

//variables
String symbol[5][4] = {
  {"<", ">", "<<", "C"},
  {"7", "8", "9",  "/"},
  {"4", "5", "6",  "*"},
  {"1", "2", "3",  "-"},
  {"0", ".", "=",  "+"},
};
int X, Y;
int Num1, Num1dec, Num2, Num2dec, 
float Result = 0.0;
char oper;
boolean resultDisplay, Dec = false;
boolean graphMenu, calcMenu = false;
boolean mainMenu = true;
boolean errorFlag = false;

void setup(){
  Serial.begin(9600);//debugging purposes
  tft.reset();       //Reset at start
  tft.begin(0x9341); //LCD used interface
  tft.fillScreen(WHITE);

  draw_BoxNButtons();
}

void loop() {
  //Reads user's touch and changes X and Y accordingly
  TSPoint p = waitTouch();
  X = p.y;
  Y = p.x;

  DetectButtons();
  if(result==true){
    CalculateResult();
    DisplayResult();
    delay(300);
  }
}

/* Define all functions
 * 
 * waitTouch()
 * Finds and returns point user has touched on screen.
 * (0 <= x <= 320)
 * (0 <= y <= 240)
 */
TSPoint waitTouch(){
  TSPoint pTemp; //point to be returned
  
  //find point while checking for valid pressure
  do{
    p = ts.getPoint();
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
  }while ((p.z < MINPRESSURE) || (p.z > MAXPRESSURE));

  //set (p.x, p.y) coordinates
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, 240);

  return p;
}

/*
 * DetectMainButtons()
 * Detects which button is pressed by user in main menu.
 */
void DetectMainButtons(){
  //TODO
}

/*
 * DetectGraphButtons()
 * Detects which button is pressed by user in graph menu.
 */
void DetectGraphButtons(){
  //TODO
}

/*
 * DetectCalcButtons()
 * Detects which button is pressed by user in calculator.
 */
void DetectCalcButtons(){
  if(X<60 && X>0){ //column 0
    if(Y<64 && Y>0){ //row 0
      //Cancel button
      Num1= Num1dec=Num2=Num2dec=0;
      Result = 0.0;
      resultDisplay = false;
    }
    else if(Y<128 && Y>64){ //row 1
      if(Result == 0.0){
        Result = 1.0
      }
      else{
        if(Dec==true){
          Result = 
        }
        Result = (Result*10) + 1; //pressed more than once
      }
    }
    else if(Y<192 && Y>128){ //row 2
      
    }
    else if(Y<256 && Y>192){ //row 3
      
    }
    else if(Y<320 && Y>256){ //row 4
      
    }
  }
  else if(X<120 && X>60){ //column 1
    if(Y<64 && Y>0){ //row 0
      
    }
    else if(Y<128 && Y>64){ //row 1
      
    }
    else if(Y<192 && Y>128){ //row 2
      
    }
    else if(Y<256 && Y>192){ //row 3
      
    }
    else if(Y<320 && Y>256){ //row 4
      
    }
  }
  else if(X<180 && X>120){ //column 2
    if(Y<64 && Y>0){ //row 0
      
    }
    else if(Y<128 && Y>64){ //row 1
      
    }
    else if(Y<192 && Y>128){ //row 2
      
    }
    else if(Y<256 && Y>192){ //row 3
      
    }
    else if(Y<320 && Y>256){ //row 4
      
    }
  }
  else if(X<240 && X>180){ //column 3
    if(Y<64 && Y>0){ //row 0
      
    }
    else if(Y<128 && Y>64){ //row 1
      
    }
    else if(Y<192 && Y>128){ //row 2
      
    }
    else if(Y<256 && Y>192){ //row 3
      
    }
    else if(Y<320 && Y>256){ //row 4
      
    }
  }
}

/*
 * DetectButtons()
 * Uses boolean values mainMenu, graphMenu, and calcMenu
 * to determine which Menu to use for user touch.
 */
void DetectButtons(){
  //TODO
}

/*
 * CalculateResult()
 * Interprets existing variables and sends both operation
 * and result to the LCD on Arduino Uno #2
 */
void CalculateResult(){
  //TODO
}

/*
 * SendResult()
 * Sends Result to Arduino Uno #2
 * for 16x2 LCD.
 */
void SendResult(){
  //TODO
}

/*
 * SendString()
 * Sends operation string to Arduino Uno #2
 * for 16x2 LCD.
 */
void SendString(){
  //TODO
}

/*
 * SendGraph()
 * Sends graph equation to Arduino Mega #2
 * for display.
 */
void SendGraph(){
  //TODO
}

/*
 * SendError()
 * Sends current boolean value for Error variable
 * to Arduino Uno #1.
 */
void SendError(){
  //TODO
}

/*
 * IntroScreen()
 * Shows main menu for GUI.
 */
void IntroScreen(){
  //TODO
}

/*
 * DrawButtons()
 * Draws appropriate buttons given status of menu variables
 */
void DrawButtons(){
  //TODO
}
