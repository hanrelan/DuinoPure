#include <LiquidCrystal.h>

#define PIN_BTN_GOSTOP 2
#define PIN_BTN_TIMEUP 3
#define PIN_BTN_TIMEDOWN 4
#define PIN_ROCKER_SAFETY 5

#define PIN_UV_1 13

#define TIME_MAX 20*60
#define TIME_MIN 0

#define PRESSED LOW
#define UNPRESSED HIGH


long lastTime = millis();
int time = 0;
boolean isOn = false;

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
char lcd_buffer[20];
char time_string[16];

void lcd_print(char *string, int column, int line){
  lcd.clear();
  lcd.setCursor(column, line);
  lcd.print(string);
}

void set_pin_as_button_input(int pin){
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
}

int readButtonInput(int pin) {
  int val1 = digitalRead(pin);
  delay(10);
  int val2 = digitalRead(pin);
  if(val1 == val2){
    return val1;
  }
}

boolean isButtonPressedOnce(int pin, int *buttonState) {
  int button = readButtonInput(pin);
  boolean retValue = false;
  if((button != *buttonState) && (button == PRESSED)) {
    retValue =  true;
  }
  else {
    retValue = false;
  }
  *buttonState = button;
  return retValue;
}

void stop() {
  isOn = false;
  digitalWrite(PIN_UV_1, LOW);
  delay(1000);
  lcd_print("Stopped", 0, 0);
}

void start() {
  isOn = true;
  lastTime = millis();
  digitalWrite(PIN_UV_1, HIGH);
  lcd_print("Starting", 0, 0);
}

void readGoStop() {

  static int btnState = UNPRESSED;

  if(isButtonPressedOnce(PIN_BTN_GOSTOP, &btnState)){
    if(isOn == false){
      if(readButtonInput(PIN_ROCKER_SAFETY) == PRESSED){
        start();
      }
      else {
        lcd_print("Close door", 0, 0);
      }
    }
    else {
      stop();
    }
  }
}

void readSafety() {
  if(isOn) {
    if(readButtonInput(PIN_ROCKER_SAFETY) == UNPRESSED) {
      lcd_print("Door open!", 0, 1);
      stop();
    }
  }
}

void setTimeString() {
  int minutes = time/60;
  int seconds = time %60;
  sprintf(time_string, "%i:%02i", minutes, seconds);
}

void readTime() {

  static int downBtnState = UNPRESSED;
  static int upBtnState = PRESSED;
  
  boolean didChange = true;
  if(isButtonPressedOnce(PIN_BTN_TIMEDOWN, &downBtnState)){
    time = time + 60;
  }
  else if(isButtonPressedOnce(PIN_BTN_TIMEUP, &upBtnState)){
    time = time - 60
    ;
  }
  else {
    didChange = false;
  }
  
  if(time < TIME_MIN){
    time = TIME_MIN;
  }
  if(time > TIME_MAX){
    time = TIME_MAX;
  }
  
  if(didChange){
    setTimeString();
    sprintf(lcd_buffer, "New time: %s", time_string);
    lcd_print(lcd_buffer, 0, 1);
  }
  
}

void updateTime() {
  unsigned long currentTime = millis();
  if((currentTime - lastTime
  ) >= 1000){
    lastTime = currentTime;
  }
  else{
    return;
  }
  
  if(isOn) {
    lcd_print("Running...", 0, 0);
    time = time - 1;
    setTimeString();
    sprintf(lcd_buffer, "Time left %s", time_string);
    lcd_print(lcd_buffer, 0, 1);
    if(time <= 0) {
      time = 0;
      stop();
    }
  }
}

void setup() {
  Serial.begin(9600);
  set_pin_as_button_input(PIN_BTN_GOSTOP);
  set_pin_as_button_input(PIN_BTN_TIMEUP);
  set_pin_as_button_input(PIN_BTN_TIMEDOWN);
  set_pin_as_button_input(PIN_ROCKER_SAFETY);
  
  pinMode(PIN_UV_1, OUTPUT);
  digitalWrite(PIN_UV_1, LOW);
  
  lcd.begin(16, 2);
  lcd_print("DuinoPure!", 0, 0);
}

void loop() {
  readSafety();
  readGoStop();
  readTime();
  updateTime();
}
