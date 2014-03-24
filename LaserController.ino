/*Laser Controller for Arduino. This is a small CLI based arduino program that will allow simple safe use of a TTL capable laser. See home page for schematics and other details.

  Laser adapation and feature addition by James Kinney.

  v.5a
  
    On and off.  
    PWM Average power setting.
    Pulse in Hz setting, up to maximum resolution of the arduino loop.
    
  v1.0(TODO)
  
    1. Ability to set timed one shot with timed delay. 
    2. Ability to have rising and falling power over time. Ramp up and Ramp down mode.
    3. Ability to have external trigger, TTL or analog, maybe a table of power values over time?
    4. Need to integrate control of MMD, aka DLP Chip, for use in vortex experiments. LCD window for controlling laser profile?
    5. Calibrate average power with actual setting.
    
    
*/





/*Credits...*/

/* simplecli version 001
 * -----------------
 * C. Cosentino 2009 - http://digital-salvage.net/?p=36
 *  This software is licensed under the CC-GNU GPL version 2.0
 *     See: http://creativecommons.org/licenses/GPL/2.0/
 *    
 *  Simple CLI example that demonstrates how to use serial to perform
 *  interactive commands (turning a pin on or off) via a terminal application.
 */
 
 

#define VER "5a"

#define laserTTLPin 9        //Laser Output Pin
#define MCHARS 20            //Max Characters for CLI input command line
#define ENTER 13             //Define of Enter Key value, EOL on input
int laserMIN = 16;           //Minimum PWM setting for Laser Diode, laser diodes have a threshold to lase, minimum on time, so set this to the characteristics of your diode.
int laserMAX = 255;          //Maximum PWM setting for Laser Diode, however when not full power the laser will pulse, albiet quickly to achieve limited average power
char inStr[MCHARS];          //Input String Variable
int inByte = 0;              //Input Byte Variable
int strCount = 0;            //String Length Counter
int pulse = 0;               //Pulse Variable, 0 = off, 1 = on
int laserOn = LOW;           //Laser status variable, HIGH = laser on, LOW = Laser off. 
int laserLevel = 16;         //The laserLevel variable determines PWM level, 0 is power not limited. This variable should only ever be set as low as laserMIN or the special 0 for no limit.
int laserPercent = 5;
int rate = 5;                //Rate of the Pulse, this is in Hz.
long previousMillis = 0;     //Variable to help pulse state machine determine time.
float interval = 200;        //Time Interval 

void setup() {
        pinMode(laserTTLPin, OUTPUT);  // sets ledPin as an OUTPUT pin
	Serial.begin(9600);	  // opens serial port, sets data rate to 9600 bps
        Serial.print("LaserController>");        // print  prompt when serial is ready
}

void loop() {
  if(Serial.available()) {                 // if bytes are available to be read
    inByte = Serial.read();                // read the byte
    Serial.print(char(inByte));            // echo byte entered to terminal as a char
    if(inByte != ENTER) {                  // if the byte is not a NEWLINE
      inStr[strCount] = char(inByte);      // add the byte to the "String" array
      strCount++;                          // increase the item string count
    }
  }
  
  if(inByte == ENTER || strCount >= MCHARS) {  // if enter was pressed or max chars reached
    Serial.flush();                         // flush the serial data (overkill?)
    Serial.println("");                    // print a newline
    if (strncmp(inStr, "power", 5) == 0){
      if(strlen(inStr) > 6 ) { //Includes value?
        laserPercent = atoi(inStr+6);
        if(laserPercent == 0 ) { //Was 0 percent?
          analogWrite(laserTTLPin, 0);            // if so, then turn off ledPin
          Serial.println("Laser Off");            // print to the terminal
          pulse = 0;
        } else {  //Need to figure Laser Diode PWM setting
          laserLevel = laserMIN + ((laserMAX-laserMIN)*laserPercent/100); //Convert percentage to a PWM lavel between laserMIN and laserMAX
          analogWrite(laserTTLPin, laserLevel);
          Serial.print("Power set to ");
          Serial.print(laserPercent);        // print to the terminal
          Serial.print("% laserLevel ");
          Serial.println(laserLevel);
        } 
      } else { //No Value
        Serial.print("Must enter power in percent, e.g. >power 30");
      } //Includes value
    } else if (strncmp(inStr, "pulse", 5) == 0){ //Pulse Laser?
      if(strlen(inStr) > 6 ) { //Has value?
        rate = atoi(inStr+6);
        interval = (1.0/rate); //In Hz to fraction of second.
        interval = interval*1000; //Now we need milliseconds delay
      }
      if (pulse == 1 && strlen(inStr) < 6) { //Already blinking, need to turn off Laser Diode
        analogWrite(laserTTLPin, 0); //Set Laser Diode State to off
        Serial.println("Laser Off"); //Print out message indicating it is done
        pulse = 0; //Reset Pulse Flag
      } else { //Not already pulsing
        Serial.print("Laser pulsing at "); //Print out pulse settings.
        Serial.print(rate);   
        Serial.print("Hz ");
        Serial.print(interval);
        Serial.println("ms");
        pulse = 1;//Indicate to outter loop that pulse is activated.
      }
    } else if (strcmp(inStr, "on") == 0){ //Laser Diode On?
      if( laserLevel != 0 ) {
        analogWrite(laserTTLPin, laserLevel); //Set Laser Diode PWM level
        Serial.print("Laser on, set to ");
        Serial.print(laserPercent);
        Serial.print("% laserLevel ");
        Serial.println(laserLevel);        
      } else {
        analogWrite(laserTTLPin, 255); //Full on
        Serial.println("Laser On");
      }
                    // print to the terminal
      pulse = 0;
    } else if (strcmp(inStr, "off") == 0){  // string compare, does entered text == off?
      analogWrite(laserTTLPin, 0);            // if so, then turn off ledPin
      Serial.println("Laser Off");            // print to the terminal
      pulse = 0;
    } else if (strcmp(inStr, "low") == 0){
      laserPercent = 10;
      laserLevel = 30;
      analogWrite(laserTTLPin, laserLevel);
      Serial.print("Laser power set to ");
      Serial.print(laserPercent);
      Serial.println("%");
    } else if (strcmp(inStr, "med") == 0){
      laserPercent = 50;
      laserLevel = 135;
      analogWrite(laserTTLPin, laserLevel);
      Serial.print("Laser power set to ");
      Serial.print(laserPercent);
      Serial.println("%");
    } else if (strcmp(inStr, "high") == 0) {
      laserPercent = 100;
      laserLevel = 255;
      analogWrite(laserTTLPin, laserLevel);
      Serial.print("Laser power set to ");
      Serial.print(laserPercent);
      Serial.println("%");
    } else {                                // if the input text doesn't match any defined above
      Serial.println("Invalid.");           // echo back to the terminal
    }
    strCount = 0;                           // get ready for new input... reset strCount
    inByte = 0;                             // reset the inByte variable
    for(int i = 0; inStr[i] != '\0'; i++) { // while the string does not have null
      inStr[i] = '\0';                      // fill it with null to erase it
    }
    Serial.println("");                     // print a newline
    Serial.print("LaserController>");                      // print the prompt
  }
  
  
  //Pulse Mode
  if(pulse == 1) {
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis > interval) {
      //Set last time you pulsed the Laser Diode
      previousMillis = currentMillis;  
      //Set the state of the Laser Diode, simple Flip-Flop concept
      if (laserOn == LOW) {
        analogWrite(laserTTLPin, laserLevel);
        laserOn = HIGH;
      } else {
        analogWrite(laserTTLPin, 0);
        laserOn = LOW;
      }
    }
  }//Pulse Mode
}

 
