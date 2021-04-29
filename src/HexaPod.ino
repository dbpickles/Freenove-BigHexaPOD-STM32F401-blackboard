// Need to do, add comments and header

#include <Arduino.h>
#include <Wire.h>
#include <TinyMPU6050.h>
#include <Adafruit_PWMServoDriver.h>
#include <Adafruit_NeoPixel.h>
#include <SPIFlash.h>
#include "ansi.h"
#include "structure.h"
#include <bits/stdc++.h>

#define NEOLED PD6            // NEO Pixel LED pin
#define NUMPIXELS 7           // Number of NEO pixles
#define BUZZER PD7            // Buzzer pin
#define DELAYVAL 200          // Led delay
#define SERVO_FREQ 50         // Analog servos run at ~50 Hz updates

String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

Adafruit_PWMServoDriver pwm0 = Adafruit_PWMServoDriver(0x40, Wire);
Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver(0x41, Wire);
Adafruit_NeoPixel pixels(NUMPIXELS, NEOLED, NEO_GRB + NEO_KHZ800);
MPU6050 mpu (Wire);
SPIFlash flash(PIN_SPI_W25, 0xEF40);
ANSI ansi(&Serial);

void ledOff()
  {
    for(int i=0; i<NUMPIXELS; i++) 
    { // For each pixel...
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      pixels.show();   // Send the updated pixel colors to the hardware.
    }

  }

  void ledRotate(int r, int g, int b)
  {
    for(int i=0; i<NUMPIXELS; i++) 
    { // For each pixel...
      pixels.setPixelColor(i-1, pixels.Color(0,0,0));
      pixels.setPixelColor(i, pixels.Color(r,g,b));
      pixels.show();            // Send the updated pixel colors to the hardware.
      delay(142);
    }
    ledOff();
  }

void setup() {
  delay(2000);

  pinMode(BTN_K0,INPUT_PULLUP);        // Save config
  pinMode(BTN_K1,INPUT_PULLUP);        // Load config
  pinMode(LED_BUILTIN,OUTPUT);  
  pinMode(LED_GREEN,OUTPUT);
  pinMode(BUZZER,OUTPUT);              // HexaPod on board Buzzer

  Serial.begin(9600);

  tone(BUZZER,2000,500);

  ansi.clearScreen();
  ansi.println("Serial Begin....");

  ansi.print("Get Config from Flash: ");
    if (flash.initialize())
  {
    ansi.println("OK!");
    readConfig(0);
  }
  else {
    ansi.println("FAIL!");
  }

  ansi.println("NEO Pixle Begin");
  pixels.begin();                     // INITIALIZE NeoPixel strip object (REQUIRED)
  
  Wire.setSDA(PB7);
  Wire.setSCL(PB6);

  Wire.begin();
  ansi.println("Wire Begin");
  
  
  Serial.println("Checking I2C Devices");
  i2cCheck(64, "  PWM 0");
  i2cCheck(65, "  PWM 1");
  i2cCheck(72, "MPU6050");
  i2cCheck(104,"ADS7830");
 
  mpu.Initialize();

  pixels.clear(); // Set all pixel colors to 'off'
  ansi.println("MPU6050 Begin");
  ansi.println("Starting calibration...");

  ledRotate(255,0,0);
  mpu.Calibrate();
  ledRotate(0,255,0);
 
  ansi.println("Calibration complete!");
  ansi.print("Offsets");
  ansi.print(" - X = ");
  ansi.print(mpu.GetGyroXOffset());
  ansi.print(" - Y = ");
  ansi.print(mpu.GetGyroYOffset());
  ansi.print(" - Z = ");
  ansi.println(mpu.GetGyroZOffset());
  
  delay(100);
  
  pixels.clear(); // Set all pixel colors to 'off'
  
  pwm0.begin();
  pwm1.begin();
  
  pwm0.setOscillatorFrequency(27000000);
  pwm0.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates

  pwm1.setOscillatorFrequency(27000000);
  pwm1.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates

  delay(10);

  ansi.foreground(ansi.blue);
  ansi.print("CMD>");
  ansi.normal();
}


void i2cCheck(int id, String desc)
{
    Wire.beginTransmission(id);
    ansi.print(desc);

    if(Wire.endTransmission())
    {
      ansi.foreground(ansi.green);
      ansi.println(" : OK");
      ansi.normal();
    }
    else
    {
      ansi.foreground(ansi.red);
      ansi.println(" : FAIL");
      ansi.normal(); 
    }  
}

// Write hexaPod configuration to winbond flash
void writeConfig(int slot)
  {
    ansi.printf("\nWriting to slot %d ...",slot);
    flash.writeBytes(sizeof(Servo) * slot, Servo, sizeof(Servo)); 
    while(flash.busy());
    ansi.printf("Size [%d]\n",sizeof(Servo));
    tone(BUZZER,2000,500);
  }

// Read hexaPod configuration to winbond flash
void readConfig(int slot)
  {
      flash.readBytes(sizeof(Servo) * slot,Servo,sizeof(Servo));
      ansi.printf("READ CONFIG SLOT %d SIZE [%d]  \n",slot, sizeof(Servo));
      tone(BUZZER,2000,500);
  }

// wait for Serial Data data
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

void loop() {      
  getGyro();

  // Write confiuration
  if(digitalRead(BTN_K0) == LOW)   
  {
    ledRotate(248,148,6);   // Orange LEDS
    writeConfig(0);
  }

  // Read configuration
  if(digitalRead(BTN_K1) == LOW)
  {
    ledRotate(255,255,255);  // White LEDS
    readConfig(0);
  }

  if (stringComplete) 
  {    
      inputString.toUpperCase() ;
      ansi.println(inputString);

      String cmd  = getValue(inputString, SEP, 0);
      String par1 = getValue(inputString, SEP, 1);
      String par2 = getValue(inputString, SEP, 2);
      String par3 = getValue(inputString, SEP, 3);
      String par4 = getValue(inputString, SEP, 4);
      String par5 = getValue(inputString, SEP, 5);

      if (cmd == "ST") {
              Servo[par1.toInt()].servo_id = par1.toInt() ;
              int deg = par2.toInt() ;
              ansi.printf("SERVO[%d] DEGREE[%d] \n",Servo[par1.toInt()].servo_id,deg);
              servoAngle(Servo[par1.toInt()].servo_id, deg);
      }

      if (cmd == "SC") 
      {
              ansi.foreground(ansi.green);
              ansi.bold();
              ansi.printf("%s\n",Servo[par1.toInt()].desc);
              ansi.normal();
              ansi.printf("SERVO[%d]    PWM[%d] PULSE [%d/%d] \n",Servo[par1.toInt()].servo_id,Servo[par1.toInt()].pwm_id,Servo[par1.toInt()].servo_min,Servo[par1.toInt()].servo_max);
              ansi.printf("  POS[%d]  SLEEP[%d]   MOVE[%d/%d]\n",Servo[par1.toInt()].default_pos,Servo[par1.toInt()].default_sleep,Servo[par1.toInt()].min_movement,Servo[par1.toInt()].max_movement);                           
              ansi.printf(" SIDE[%d]    LEG[%d]  JOINT[%d] \n",Servo[par1.toInt()].side,Servo[par1.toInt()].leg,Servo[par1.toInt()].joint);
      }

      if (cmd == "SM") {
              int ServoNo;

              ServoNo   = par1.toInt();
              Servo[ServoNo].servo_id = ServoNo;
              Servo[ServoNo].servo_min      =   par2.toInt();
              Servo[ServoNo].servo_max      =   par3.toInt();
              Servo[ServoNo].min_movement   =   par4.toInt();
              Servo[ServoNo].max_movement   =   par5.toInt();
              ansi.printf("SERVO[%d] PULSE[%d/%d] \n",Servo[par1.toInt()].servo_id,Servo[par1.toInt()].servo_min,Servo[par1.toInt()].servo_max);
      }

      if (cmd == "LOAD") 
      {
          int slot   = par1.toInt(); // Slot location
          readConfig(slot);

      }  

      if (cmd == "SAVE") 
      {
          int slot   = par1.toInt(); // Slot location
          writeConfig(slot);
      } 

    if (cmd == "DEFAULTS") {
          Serial.print("Erasing Flash chip ... ");
          flash.chipErase();
          while(flash.busy());
          Serial.println("DONE");

          int slot   = par1.toInt(); // Slot location

          //Servos and there positions.

                           //  N = Knee  X = HipX   Y= HipY 
                           //  ******************************************************  
                           //        X        X        X        X        X         X
                           //     Y        Y        Y        Y        Y        Y 
                           //  N        N        N        N        N        N 
                           //    Front    Mid      Back     Front    Mid      Back
                           //  -------- -------- -------- -------- -------- --------
                           //  LLLLLLLLLLLLLLLLLLLLLLLLLL RRRRRRRRRRRRRRRRRRRRRRRRRR   Left / Right Leg
                           //  00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17
          int servoPins[18] = {18,17,16,21,20,21,27,23,22,13,14,15,10,11,12,31, 8, 9}; 

          for(int i;i < 32;i++)  
            {

            Servo[i].servo_id = i;

            if(i <=15) 
            {
              Servo[i].pwm_id = I2Cx40;
            }
            else
            {
              Servo[i].pwm_id = I2Cx41;
            }

            sprintf (Servo[i].desc, "UNUSED [%d]", i);

            Servo[i].leg            = UNUSED;
            Servo[i].joint          = UNUSED;
            Servo[i].side           = UNUSED;

            Servo[i].servo_min      = 80;
            Servo[i].servo_max      = 480;

            Servo[i].min_movement   = 0;
            Servo[i].max_movement   = 180;

            Servo[i].default_pos    = 45;
            Servo[i].default_sleep  = 180; 

            for(int l = 0; l < 18; l++)
              {
                if(servoPins[l] == i)
                {
                  // Left Or Right Leg
                  if(l <9 )
                    {
                      Servo[i].side = LEFT;
                    }
                  else
                    {
                      Servo[i].side = RIGHT;  
                    }

                 // type / Position
                 switch(l)
                 {
                    case 0:  Servo[i].leg = FRONT ;  Servo[i].joint = KNEE; break; 
                    case 3:  Servo[i].leg = MIDDLE;  Servo[i].joint = KNEE; break; 
                    case 6:  Servo[i].leg = BACK;    Servo[i].joint = KNEE; break; 
                    case 9:  Servo[i].leg = FRONT;   Servo[i].joint = KNEE; break; 
                    case 12: Servo[i].leg = MIDDLE;  Servo[i].joint = KNEE; break; 
                    case 15: Servo[i].leg = BACK;    Servo[i].joint = KNEE; break;

                    case 1:  Servo[i].leg = FRONT ;  Servo[i].joint = HIPY; break;
                    case 4:  Servo[i].leg = MIDDLE;  Servo[i].joint = HIPY; break;
                    case 7:  Servo[i].leg = BACK;    Servo[i].joint = HIPY; break;
                    case 10: Servo[i].leg = FRONT;   Servo[i].joint = HIPY; break;
                    case 13: Servo[i].leg = MIDDLE;  Servo[i].joint = HIPY; break;
                    case 16: Servo[i].leg = BACK;    Servo[i].joint = HIPY; break;

                    case 2:  Servo[i].leg = FRONT ;  Servo[i].joint = HIPX; break;
                    case 5:  Servo[i].leg = MIDDLE;  Servo[i].joint = HIPX; break;
                    case 8:  Servo[i].leg = BACK;    Servo[i].joint = HIPX; break;
                    case 11: Servo[i].leg = FRONT;   Servo[i].joint = HIPX; break;
                    case 14: Servo[i].leg = MIDDLE;  Servo[i].joint = HIPX; break;
                    case 17: Servo[i].leg = BACK;    Servo[i].joint = HIPX; break;
                 }
                sprintf (Servo[i].desc, "SERVO [%d]", i);
                break; 
                }
              }      
            }

          writeConfig(slot);                 // Uncomment for first time to erase flash and write default config to flash
    }


      inputString = "";
      stringComplete = false;
      tone(BUZZER,2000,100);
      ansi.foreground(ansi.blue);
      ansi.print("CMD>");
      ansi.normal();
  }
}

void servoAngle(int ServoNum, int degree)
{     
    int pulselen = 0 ;

    //Check if degree is valid range
    if(degree >= Servo[ServoNum].max_movement)
    {
        degree = Servo[ServoNum].max_movement ;
    }
    else if (degree <= Servo[ServoNum].min_movement)
    {
        degree = Servo[ServoNum].min_movement ;
    }

    pulselen = map(degree, 0, 180, Servo[ServoNum].servo_min, Servo[ServoNum].servo_max);

    Serial.printf("D %d  S %d  MI %d  MX %d ",degree, ServoNum,Servo[ServoNum].servo_min,Servo[ServoNum].servo_max);

    if (Servo[ServoNum].pwm_id == 0)      
    {
      pwm0.setPWM(Servo[ServoNum].servo_id, 0, pulselen);

    }
    else
    {
      pwm1.setPWM(Servo[ServoNum].servo_id, 0, pulselen);
    }
}

void getGyro()
{ 
  mpu.Execute();
}

// explode Serial data into a value specified by index
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}