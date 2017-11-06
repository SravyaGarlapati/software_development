//   ---------------------------------------------------------------------------- 
//                    Bluetooth Low Energy Peripheral 
   
//            Modified from code written by Mr Frank Duignan
//                by Sravya Garlapati and Neil Hamilton 
 
//  BLE specific includes for pre-defined standardized protocols and function calls
// Hosted on GITHub @ https://github.com/sandeepmistry/arduino-BLEPeripheral/tree/master/src
// ------------------------------------------------------------------------------------------
#include <BLEAttribute.h>
#include <BLECentral.h>
#include <BLECharacteristic.h>
#include <BLECommon.h>
#include <BLEDescriptor.h>
#include <BLEPeripheral.h>
#include <BLEService.h>
#include <BLETypedCharacteristic.h>
#include <BLETypedCharacteristics.h>
#include <BLEUuid.h>
#include <Servo.h>

// servo specific include
Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

// Global Variables and objects
BLEPeripheral blePeripheral;                           // Installation of a Bluetooth low Energy Peripheral Object
BLEService BLENH_SGSensorService("2345");               //Installation of sensor service object --passed to BLEAttribute.h


// UUID number and the properties Notify position of servo and magnet
// remote clients will be able to read the LED state & are notified when it changes
BLEIntCharacteristic BrightnessLevelChar("0x2A77", BLERead | BLENotify);    // read/notify BLE LED Characteristic
BLEIntCharacteristic MagnetChar("0x26FF", BLERead | BLEWrite);            // read/write to the MagnetChar characteristic
BLEIntCharacteristic MoveServo("0xFDB2", BLEWrite);                      // write to MoveServo characteristic

//Global variables declared and initialised pins
int Magnet=7;
int Magnet_val=0;
int pos=0;
float Brightness = 0;

//  Setup - configuration of Baud rates, BLE peripherals and Data Direction registers for I/O
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);    // set up for the baud rate for serial communication
  // DDR for digital output
  pinMode(13, OUTPUT);   // initialize the LED on pin 13 to
  // indicate when a central is connected
  pinMode(Magnet, OUTPUT);  // This is the Magnet that will be controlled
    // servo on pin 9 attached to servo object 
  myservo.attach(9);       // Attach the servo PWM control line to the PWM pin on the board
  myservo.write(0);

  // Name advertised by the BTLE protocol for clients to connect to 
  blePeripheral.setLocalName("NH&SG_IoT");
  blePeripheral.setAdvertisedServiceUuid(BLENH_SGSensorService.uuid());  // Add the service UUID
  blePeripheral.addAttribute(BLENH_SGSensorService);                     // Add the BLE service
  blePeripheral.addAttribute(BrightnessLevelChar);                        // Add the Brightness characteristic
  blePeripheral.addAttribute(MagnetChar);                               // Add the Magnet characteristic
  blePeripheral.addAttribute(MoveServo);                               // Add the Servo movement characteristic  
 
  BrightnessLevelChar.setValue(Brightness);        // Set initial value of brightness (0)   
  // Value which will be set by client later, different values will produce different outputs to the servo
  MagnetChar.setValue(0);               // Set initial value of magnet (0) 
  /* Now activate the BLE device.  It will start continuously transmitting BLE
     advertising packets and will be visible to remote BLE central devices
     until it receives a new connection */
  blePeripheral.begin();                // Peripheral will start to broadcast small data BTLE data packets avdvertising the name and services
  Serial.println("Bluetooth device active, waiting for connections...");

}
//Function that reads the current value of the Light Dependant Resistor and prints the result on the serial port
void LightSensor() {
  
  Brightness = analogRead(A0);                              // Read the LDR on analogue input pin 0
  Serial.print("\nBrightness Value: ");                     // print the text
  Serial.println(Brightness);                               // print value
                                                            // NOTE: The values sent over the link are LITTLE ENDIAN
  BrightnessLevelChar.setValue(Brightness);                 // Update the brightnesslevelchar characteristic by setting value to "Brightness"
  delay(5000);                                              //delay the function for 5s
}
//Function tha reads the current value of the Magnet and prints the result on the serial port
void MagnetRead() {
  
  Magnet_val=digitalRead(Magnet);                           // Update the characteristic Magnet_val by reading the value "Magnet"
  Serial.print("\tMagnet Value: ");                         // print text
  Serial.println(Magnet_val);                               // print value
                                                            // NOTE: The values sent over the link are LITTLE ENDIAN
  MagnetChar.setValue(Magnet_val);                          // Update the MagnetChar characteristic by setting it to value "Magnet_val
  
}
//Function that checks the current value of the Servo and prints the result on the serial port
void CheckServo(){

  if(MoveServo.written()){                                    // did User write to MoveServo characteristic?
    
    if (MoveServo.value()) {                                  // did User write a non-zero value?
      
      for(pos = 0; pos <= 180; pos += 1){                     // increments from 0 degrees to 180 degrees
                                                              // in steps of 1 degree
        myservo.write(pos);                                   // tell servo to go to position in variable 'pos'
        delay(15);                                            // waits 15ms for the servo to reach the position
      
      }
      Serial.print("\nServo position: ");                     // print text
      Serial.println(pos);                                    // print value
      Serial.print(" Status: OPEN ");                         // print text
    
    }  
    else if (MoveServo.value() == 0) {                        // did User write a non-zero value?
    
      for(pos = 180; pos>=0; pos-=1){                         // goes from 180 degrees to 0 degrees
 
        myservo.write(pos);                                   // tell servo to go to position in variable 'pos'
        delay(15);                                            // waits 15ms for the servo to reach the position
      
      }
      Serial.print("\nServo position: ");                     // print out text
      Serial.println(pos);                                    // print out value
      Serial.print(" Status: CLOSED ");                       // print out text
    }      
  }
  else {
  
    Serial.print("\nNo User Input on Servo ");                // print text
  
  }
}
void loop() {
  
  BLECentral central = blePeripheral.central();               // Bind the peripheral to the client
  
  if (central) {                                              // Only enter this path once we have established a connection
  
    Serial.print("Connected to central: ");                   // print to console connection success
    Serial.println(central.address());                        // print the console MAC address:
    digitalWrite(13, HIGH);                                   // turn on the LED to indicate the connection:
    
    while (central.connected()) {                             // while the device is connected via bluetooth, enter and remain in this statement/loop.
      /*LDR*/
      LightSensor();                                          // call the LightSensor function, return the current value to central
      MagnetRead();                                         // call the MagnetRead function, return the current value to central
       
      if (Brightness >= 200){                                 // If light is High
          
        //MagnetRead();                                         // call the MagnetRead function, return the current value to central
        if (MagnetChar.written()){                            // did central write to Magnet?
      
          if (MagnetChar.value()) {                           // did central write a non-zero value?
   
            digitalWrite(Magnet,HIGH);                        // if so, then turn on the Magnet
            Serial.print("\nMagnet Currently On. Value: ");   // print text
            Serial.print(MagnetChar.value());                 // print value
            Serial.print("\nServo will activate and close door"); // print text
            
            for(pos = 180; pos>=0; pos-=1){                   // goes from 180 degrees to 0 degrees
 
              myservo.write(pos);                             // tell servo to go to position in variable 'pos'
              delay(15);                                      // waits 15ms for the servo to reach the position
            
            }
            Serial.print("\nServo position: ");               // print text
            Serial.println(pos);                              // print value
            Serial.print(" Status: CLOSED ");                 // print text        
          }
        } 
        else {
        
            digitalWrite(Magnet,LOW);                         // turn off the Magnet(set it low)
            Serial.print("\nMagnet Currently Off. Value: ");  // print text
            Serial.print(MagnetChar.value());                 // print value
        
        }
        CheckServo();                                         // call CheckServo function, return the current value to central
      }
      else if(Brightness < 200){                              // If light is Low, enter this statement
        
        //MagnetRead();                                         // call the MagnetRead function, return the current value to central
        if (MagnetChar.written()){                            // did User write to Magnet Characteristic?
            
          if (MagnetChar.value() == 0) {                      // Checks the clients Data i.e. did User write a zero value?
            
            digitalWrite(Magnet,LOW);                         // turn off the Magnet
            Serial.print("\nMagnet Currently Off. Value: ");  // print text
            Serial.print(MagnetChar.value());                 // print value
          }
        }  
        else {                                              // enter this statement if no user input
          digitalWrite(Magnet,HIGH);                        // if so, then turn on the Magnet
          Serial.print("\nMagnet Currently On. Value: ");   // print text
          Serial.print(MagnetChar.value());                 // print vale
          Serial.print("\nServo will activate and close door"); // print text
          
          for(pos = 180; pos>=0; pos-=1){                   // goes from 180 degrees to 0 degrees
            myservo.write(pos);                             // tell servo to go to position in variable 'pos'
            delay(15);                                      // waits 15ms for the servo to reach the position
          }
          Serial.print("\nServo position: ");               // print text
          Serial.println(pos);                              // print value
          Serial.print(" Status: CLOSED ");                 // print text
        }
        CheckServo();                                       // call CheckServo function, return the current value to central
      }
      delay(2000);                                          // dalay for 2s 
    }
  }
  // when the central disconnects, turn off the LED:
  digitalWrite(13, LOW);                                   // when the central disconnects, turn off the LED on pin 13
  Serial.print("\nDisconnected from central: ");             // Print to console connection Disconnected 
  Serial.println(central.address());                        //Print to console connection Disconnected 


  delay(10000);                                             //delay for 10s 
  Serial.println("Waiting for a connection");               // Print to console Waiting for connection
}
