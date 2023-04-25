//#include <LiquidCrystal_I2C.h> // LCD
//#include // Motors
#include <IRremote.h> // IR Recv.

//LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD Initialization adn Setup on 0x27 default 
// We may want to switch this to a 7 segment display, much cheaper

// USS Pins and Vars
const int trigPinL = 8; // USS Trig Pin L
const int trigPinR = 3; // USS Trig Pin R
const int trigPinF = 12; // USS Trig Pin F
const int echoPinL = 7; // USS recvPin L
const int echoPinR = 2; // USS recvPin R
const int echoPinF = 11; // USS recvPin F
long int distance, duration;
int k = 100;


// Restriction Variables
const int minDistFront = 24; // Minimum distance before it will redirect
const int minDistSide = 4; // Minimum distance before it will redirect
const int  distParm = 20;

// IR Receiver Pins and Vars
const int irRecvPin = 13; // IR Recieve Pin
int cmd;

// Motor Pins and Vars
const int motorLPin = 9; // Left Motor Pin (IN1)
const int motorLRev = 5; // Left Motor Reverse Pin (IN2)
const int motorRPin = 10; // Right Motor Pin (IN3)
const int motorRRev = 6; // Right Motor Reverse Pin (IN4)

const int speed = 110;
void setup() {
    Serial.begin(9600); // Serial Readout is 9600
    pinMode(motorRRev,OUTPUT);
    pinMode(motorLRev,OUTPUT);
    pinMode(motorRPin,OUTPUT);
    pinMode(motorLPin,OUTPUT);

    pinMode(trigPinL, OUTPUT); // Sensor Reception
    pinMode(trigPinR, OUTPUT); // Sensor Reception
    pinMode(trigPinF, OUTPUT); // Sensor Reception
    pinMode(echoPinL, INPUT); // Reception pin from Sens L
    pinMode(echoPinR, INPUT); // Reception pin from Sens R
    pinMode(echoPinF, INPUT); // Reception pin from Sens F

    //lcd.init(); // LCD Initialization 
    //lcd.backlight(); // Turning on Backlight
    //lcd.clear(); // Clearing LCD

    IrReceiver.begin(irRecvPin); // Receiver enables

    digitalWrite(motorLRev, LOW); // Ensuring Motor Movement is Forward
    digitalWrite(motorRRev, LOW);

    //lcd.print('Strt Cmplt'); // May switch to 7 segment
}

void loop() {
    Serial.println("Pre-IR");
    if (IrReceiver.decode()) { // On IR Receive, decode
        IrReceiver.resume(); // resume reception
        Serial.print(IrReceiver.decodedIRData.command); // Print Reception
        cmd = IrReceiver.decodedIRData.command; // Set as CMD
        if (cmd == 201) { // Check CMD for Mode Button
        Serial.print("Received");
            while (true) {  // Infinite loop to be broken if Power is pressed
                if (IrReceiver.decode()){ // IR Receiver for shutdown
                    IrReceiver.resume();
                    Serial.print(IrReceiver.decodedIRData.command);
                    cmd = IrReceiver.decodedIRData.command;
                    if (cmd == 224) {
                        stop();
                        break;
                    }
                }

                drive(); // Drive Function
            }
        }
    }
}

void drive() {

    //Serial.println("Driving");
    long distanceF = sensDist(echoPinF,trigPinF);
    long distanceL = sensDist(echoPinL,trigPinL);
    unsigned long distanceR = sensDist(echoPinR,trigPinR);

    if (distanceF <= minDistFront || distanceL <= minDistSide || distanceR <= minDistSide) {
        shift();
    } 
    else if (distanceF <= minDistFront && distanceL <= minDistSide && distanceR <= minDistSide) {
        reverse();
    } else {
        forward();
    }
}

long sensDist(int echo, int trigger){ // Sensor reading
    int maxDist = 5000;
    digitalWrite(trigger, LOW);
    delayMicroseconds(2);
    digitalWrite(trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger, LOW);

    duration = pulseIn(echo, HIGH);
    distance = duration * 0.034 / 2;


    if (distance > maxDist)
    {
        distance = maxDist;
    }
    return distance;
}

void shift (){
    //lcd.print("Shifting Path");
    delay(100);
    long distanceF = sensDist(echoPinF,trigPinF);
    long distanceL = sensDist(echoPinL,trigPinL);
    long distanceR = sensDist(echoPinR,trigPinR); 

    Serial.println(" ");
    Serial.println(distanceF);
    Serial.println(distanceL);
    Serial.println(distanceR);
    Serial.println(" ");

    /*if (distanceF <= minDistFront && distanceL <= minDistSide && distanceR <= minDistSide) {
        reverse();
    } */if (distanceF > distanceL && distanceF > distanceR) {
        forward();
    } else if (distanceF < distanceL && distanceR < distanceL) {
        turnLeft();
    } else {
        turnRight();
    }
    return;
}


void forward() {
    digitalWrite(motorRRev, LOW);
    digitalWrite(motorLRev, LOW);
    long distanceL = sensDist(echoPinL,trigPinL);
    long distanceR = sensDist(echoPinR,trigPinR);
    long centerDeviation = distanceL - distanceR;
    if (distanceL >=20 || distanceR >=20) {
        analogWrite(motorLPin, speed);
        analogWrite(motorRPin, speed);
    } else if (centerDeviation < 0) {
        analogWrite(motorLPin, speed*1.4);
        analogWrite(motorRPin, (speed-centerDeviation*10));
    } else if (centerDeviation > 0) {
        analogWrite(motorRPin, speed);
        analogWrite(motorLPin, (speed-centerDeviation*10));
    } else {
        analogWrite(motorLPin, speed);
        analogWrite(motorRPin, speed);
    } 
    return;
   /*
    digitalWrite(motorRRev, LOW);
    digitalWrite(motorLRev, LOW);
    analogWrite(motorLPin, speed);
    analogWrite(motorRPin, speed);
    return;
    */
}

void turnLeft() {
    stop();
    //analogWrite(motorLRev, (speed*1.4));
    analogWrite(motorRPin, speed*2);
    return;
}

void turnRight() {
    stop();
    //analogWrite(motorRRev, (speed*1.4));
    analogWrite(motorLPin, speed*2);
    return;
}

void reverse() {
    stop();
    analogWrite(motorRRev, speed);
    analogWrite(motorLRev, speed);
    delay(1000);
    shift();
    delay(1000);
    digitalWrite(motorRRev, LOW);
    digitalWrite(motorLRev, LOW);
    return;
}

void stop(){
    Serial.print('Stopped');
    digitalWrite(motorLPin, LOW);
    digitalWrite(motorRPin, LOW);
    digitalWrite(motorRRev, LOW);
    digitalWrite(motorLRev, LOW);
    return;
}
