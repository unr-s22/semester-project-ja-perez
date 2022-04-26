#include <Stepper.h>

const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution

// initialize the stepper library on pins 10 through 13:
Stepper myStepper(stepsPerRevolution, 10, 11, 12, 13);

const int switchCW = 2; //the number of the clockwise pushbutton pin

int buttonState = 0;

void setup() {
  // set the speed at 60 rpm:
  myStepper.setSpeed(60);
  // initialize the serial port:
  Serial.begin(9600);
  //initialize the CW pushbutton pin as an input:
  pinMode(switchCW, INPUT);
  //initialize the Stop pushbutton pin as an input:
}

void loop() {
  //read the state of the cw pushbutton value:
  buttonState = digitalRead(switchCW);
  //read the state of the stop pushbutton value:
  // step one revolution  in one direction:
  if (buttonState == HIGH) {
    Serial.println(buttonState);
    myStepper.step(stepsPerRevolution);
    delay(1);
  }else if (buttonState == LOW){
    
    }
}
