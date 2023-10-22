/* MAE-211 Project */

#include <IRremote.h> // IR Recv. Library

// USS Pins and Vars
const int trigPinL = 8; // USS Trig Pin L
const int trigPinR = 3; // USS Trig Pin R
const int trigPinF = 12; // USS Trig Pin F
const int echoPinL = 7; // USS recvPin L
const int echoPinR = 2; // USS recvPin R
const int echoPinF = 11; // USS recvPin F
long int distance, duration; // Initialization of Distance and Duration Variables

// Restriction Variables
const int minDistFront = 24; // Minimum front distance before it will redirect
const int minDistSide = 4; // Minimum side sensor distance before it will redirect

// IR Receiver Pins and Vars
const int irRecvPin = 13; // IR Recieve Pin
int cmd; // Initialize cmd variables assigned to nothing

// Motor Pins and Vars
const int motorLPin = 9; // Left Motor Pin (IN1)
const int motorLRev = 5; // Left Motor Reverse Pin (IN2)
const int motorRPin = 10; // Right Motor Pin (IN3)
const int motorRRev = 6; // Right Motor Reverse Pin (IN4)

const int speed = 110; // speed value out of 255 which the motors will be ran at

void setup() { // Setting up output modes and initialization 
    Serial.begin(9600); // Serial Readout is 9600 Baud Rate
    pinMode(motorRRev,OUTPUT); // Pinmode output for motor variables
    pinMode(motorLRev,OUTPUT);
    pinMode(motorRPin,OUTPUT);
    pinMode(motorLPin,OUTPUT);

    pinMode(trigPinL, OUTPUT); // Sensor triggering pins
    pinMode(trigPinR, OUTPUT); 
    pinMode(trigPinF, OUTPUT); 
    pinMode(echoPinL, INPUT); // Reception pin from USS Sensors
    pinMode(echoPinR, INPUT); 
    pinMode(echoPinF, INPUT); 

    IrReceiver.begin(irRecvPin); // Receiver enables

    digitalWrite(motorLRev, LOW); // Ensuring Motor Movement is Forward
    digitalWrite(motorRRev, LOW);
}

void loop() {
    Serial.println("Pre-IR"); // Serial Indication that the program is running and is ready for remote start
    if (IrReceiver.decode()) { // On receiving an IR signal, decode it to a number
        IrReceiver.resume(); // resume reception that way it can receive another IR signal
        Serial.print(IrReceiver.decodedIRData.command); // Print the reception code from the last IR signal
        cmd = IrReceiver.decodedIRData.command; // Set the IR signal as the variable cmd
        if (cmd == 201) { // Check CMD for Mode Button on the remote used to start the entire system
        Serial.print("Received"); // Notification that the system has begun and received the correct code
            while (true) {  // Infinite loop to be broken if Power is pressed
                if (IrReceiver.decode()){ // IR Receiver for shutdown
                    IrReceiver.resume(); // resume reception that way it can receive another IR signal
                    Serial.print(IrReceiver.decodedIRData.command); // Print the reception code from the last IR signal
                    cmd = IrReceiver.decodedIRData.command;// Set the IR signal as the variable cmd
                    if (cmd == 224) { // Check CMD for Mode Button on the remote used to break the entire system
                        stop(); // Running the stop function to stop the robots movement through setting power output
                        break; // break the loop and send it back to "Pre-IR"
                    }
                }

                drive(); // Once the 201 command has been received, it will run the drive function, which is defined below
            }
        }
    }
}

void drive() { // defined drive function
    //Serial.println("Driving");
    unsigned long distanceF = sensDist(echoPinF,trigPinF); // Run the sensDist function with the variables for the front USS Sensor unsigned to remove negatives
    unsigned long distanceL = sensDist(echoPinL,trigPinL); // Run the sensDist function with the variables for the right USS Sensor unsigned to remove negatives
    unsigned long distanceR = sensDist(echoPinR,trigPinR); // Run the sensDist function with the variables for the left USS Sensor unsigned to remove negatives

    if (distanceF <= minDistFront || distanceL <= minDistSide || distanceR <= minDistSide) { /* Check to see if the sensors are detecting any objects 
    within the minimum distances set by the earlier parameters*/
        shift(); // run the shift function defined below 
    } 
    else if (distanceF <= minDistFront && distanceL <= minDistSide && distanceR <= minDistSide) { /* if it deteces that ALL of the sensors are within their min distances, 
    it will break to this tree */
        reverse(); // Run the reverse function
    } else {
        forward(); // Run the forward function 
    }
}

long sensDist(int echo, int trigger){ // Will take the variables which were input when the function is called and complete an action below
    int maxDist = 3000; // Setting the maximum distance which the sensors can be
    digitalWrite(trigger, LOW); // Ensuring the USS trigger pin is off and is not sending any information
    delayMicroseconds(2); // Delay for 2 microseconds to ensure waves have cleared
    digitalWrite(trigger, HIGH); // Writting to the trigger pin of the USS for 10 microseconds which is 8 cycle ultrasonic bursts
    delayMicroseconds(10); // Delay shown above
    digitalWrite(trigger, LOW); // Turn off the trigger then

    duration = pulseIn(echo, HIGH); // Read the pusle in and assign it to duration
    distance = duration * 0.034 / 2; // Using this ratio, the duration is proportional to centimeters when it is * by .034/2

    if (distance > maxDist) // If the distance is longer then the max dist it will assign it to the max distance
    {
        distance = maxDist; // assiging it to the max distance
    }
    return distance; // returning the distance value to the variable which the function was called as
}

void shift (){ // Shift Function
    long distanceF = sensDist(echoPinF,trigPinF); // Run the sensDist function with the variables for the front USS Sensor unsigned to remove negatives
    long distanceL = sensDist(echoPinL,trigPinL); // Run the sensDist function with the variables for the right USS Sensor unsigned to remove negatives
    long distanceR = sensDist(echoPinR,trigPinR); // Run the sensDist function with the variables for the left USS Sensor unsigned to remove negatives

    Serial.println(" "); // Serial Print Formatting
    Serial.println(distanceF); // Serial Print Front Distance
    Serial.println(distanceL); // Serial Print Right Distance
    Serial.println(distanceR); // Serial Print Left Distance
    Serial.println(" ");

/* The below code works undert he assumption that the angle of the USS ensures that if the L or R sensors trigger, 
if it is clear infront, that the robot and its body will clear thsoe objects being detected.*/
    if (distanceF > distanceL && distanceF > distanceR) { /* Check to see if there is room to its front and if that is larger then the L and R distances,
     if so it will keep going forward as it will miss these objects*/
        forward(); // Forward Function Called
    } else if (distanceF < distanceL && distanceR < distanceL) { /* IF it finds that the distance in front is LESS then distance to the L and that the
     distance to the R is less Distance left, it needs to go left to avoid the object*/
        turnLeft(); // Turn left function
    } else {
        turnRight(); // If it finds that it can't go forward or left, it must turn right
    }
    return; // Break the shift loop and return to the drive function
}


void forward() { // Forward driving function
    digitalWrite(motorRRev, LOW); // Ensure that the reverse motor pins are low to prevent braking
    digitalWrite(motorLRev, LOW);
    long distanceL = sensDist(echoPinL,trigPinL); // Check distances for the L and R Sensor
    long distanceR = sensDist(echoPinR,trigPinR);
    long centerDeviation = distanceL - distanceR; // Determine how much the distances differentiate from eachother
    if (distanceL >=20 || distanceR >=20) { // If distances are equal, keep both motors at same speed
        analogWrite(motorLPin, speed); // Setting them to the assigned speed variables
        analogWrite(motorRPin, speed);
    } else if (centerDeviation < 0) { // If it is found that the right distance is greater, it will turn Right to center the robot
        analogWrite(motorLPin, speed); // Setting L motor speed to speed
        analogWrite(motorRPin, (speed-centerDeviation*10)); // Decreasing R motor speed slower, so it will turn
    } else if (centerDeviation > 0) { // If it is found that the left distance is greater, it will turn left to center the robot
        analogWrite(motorRPin, speed); // Setting R motor speed to speed
        analogWrite(motorLPin, (speed-centerDeviation*10)); // Setting L motor speed slower, so it will turn
    } else {
        analogWrite(motorLPin, speed); // Keep the speeds constant to speed
        analogWrite(motorRPin, speed); // Keep the speeds constant to speed
    } 
    return; // returning to drive
}

void turnLeft() { // turn left function
    stop(); // Quickly stop the robot to ensure nothing brakes or reverses
    analogWrite(motorRPin, speed*2); // Double the speed of the right motor as the left motor is stopped so it turns left quickly
    return; // return to call spot
}

void turnRight() { // turn right function
    stop(); // Quickly stop the robot to ensure nothing brakes or reverses
    analogWrite(motorLPin, speed*2); // Double the speed of the Left motor as the right motor is stopped so it turns right quickly
    return; // return to call spot
}

void reverse() { // reverse function
    stop(); // Quickly stop the robot to ensure nothing brakes or goes forward
    analogWrite(motorRRev, speed); // Set the reverse pins to max speed
    analogWrite(motorLRev, speed); 
    delay(1000); // Set a delay to ensure that it backs a way for at least a bit
    shift(); // By clever configuration, due to the fact that when it reverses, if it wants to reverse to the left, and breaks into the turn left,  
    //when it activates the right motor, it will stop the left motor, and will reverse the right motor to reverse to the left
    delay(1000); // Set a delay to ensure that it backs a way for at least a bit once exiting shift
    digitalWrite(motorRRev, LOW); // End the reversing capabilities of the R Motor
    digitalWrite(motorLRev, LOW); // End the reversing capabilities of the L Motor
    return; // return to call spot
}

void stop(){ // Stop Function
    Serial.print('Stopped'); // Stating stop in Serial Monitor
    digitalWrite(motorLPin, LOW); // Setting ALL motor pins to low to stop everything.
    digitalWrite(motorRPin, LOW);
    digitalWrite(motorRRev, LOW);
    digitalWrite(motorLRev, LOW);
    return; // return to call spot
}
