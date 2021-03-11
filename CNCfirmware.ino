// KLAUS W - CNC Router February 2021

#define CLK 2                                                            // Rotary Encoder Pins used
#define DT 3
#define SW 4

int counter = 0;                                                         // Rotary Encoder variables
int currentStateCLK;
int lastStateCLK;
String currentDir = "";
unsigned long lastButtonPress = 0;

String MENUlbl[100];                                                     // Declaration Menu: Labels
bool MENUval[100];                                                       // Declaration Menu: Values
int MENUact = 11;                                                        // Declaration Menu: Current position

int Xpos = 0;                                                            // Global Position of Axes (0 = Home)
int Ypos = 0;
int Zpos = 0;
int Xoffs = 0;                                                           // Offset of three axes for relative positionig
int Yoffs = 0;
int Zoffs = 0;
double Xfact = 5.86;                                                     // Factor Steps per mm X (initial value before user calibration)
double Yfact = 5.88;                                                     // Factor Steps per mm Y (initial value before user calibration)
double Zfact = 24.88;                                                    // Factor Steps per mm Z (initial value before user calibration)


#define MOT 45                                                           // Pin for MOTOR Relais
#define PMP 43                                                           // Pin for PUMP Relais
bool MOTState = false;                                                   // Status of Motor Relais: false = off
bool PMPState = false;                                                   // Status of Pump Relais: false = off
bool firstRUN = true;                                                    // helps to declare initial values in LOOP

#include <LiquidCrystal.h>                                               // LCD Library
//LiquidCrystal lcd(RS, E, D4, D5, D6, D7);
LiquidCrystal lcd( 8, 9,  10, 11, 12, 13);                               // LCD Pins used

#include <SD.h>                                                          // SD Library
Sd2Card card;                                                            // SD Parameters
SdVolume volume;
File root;
const int chipSelect = 53;                                               // Pins are standard pins for the boards SPI/ICSP configuration plus pin 53 for CS (for ARDUINO MEGA)
bool sdin = false;
String fileList[20];
int filePos = 0;


#define X_STP  23                                                        // Stepper Driver and End Stop Pins for three axes
#define X_DIR  25
#define X_END  44
#define Y_STP  27
#define Y_DIR  29
#define Y_END  46
#define Z_STP  35
#define Z_DIR  37
#define Z_END  48

void setup()
{


  pinMode(CLK, INPUT);                                                   // Set encoder pins as inputs
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  lastStateCLK = digitalRead(CLK);                                       // Read the initial state of CLK

  pinMode(MOT, OUTPUT);                                                  // Pin for MOTOR Relais is OUTPUT
  pinMode(PMP, OUTPUT);                                                  // Pin for PUMP Relais is OUTPUT

  pinMode(X_STP, OUTPUT);                                                // define and initialize stepper and end stop pins X
  pinMode(X_DIR, OUTPUT);
  pinMode(X_END, INPUT_PULLUP);
  digitalWrite(X_STP, LOW);
  digitalWrite(X_DIR, LOW);
  pinMode(Y_STP, OUTPUT);                                                // define and initialize stepper and end stop pins Y
  pinMode(Y_DIR, OUTPUT);
  pinMode(Y_END, INPUT_PULLUP);
  digitalWrite(Y_STP, LOW);
  digitalWrite(Y_DIR, LOW);
  pinMode(Z_STP, OUTPUT);                                                // define and initialize stepper and end stop pins Z
  pinMode(Z_DIR, OUTPUT);
  pinMode(Z_END, INPUT_PULLUP);
  digitalWrite(Z_STP, LOW);
  digitalWrite(Z_DIR, LOW);

  lcd.begin(16, 2);                                                      // LCD parameters for 16x2 Display
  lcd.print("KLAUS W - CNC_1");                                          // Initial/Opening Screen
  lcd.setCursor(0, 1);
  lcd.print("Version 0.1");
  delay(2000);

  Serial.begin(9600);                                                     // Open serial communications and wait for port to open:
  pinMode(53, OUTPUT);                                                    // CS pin needs to be set as output and set to HIGH

}

void loop()
{

  if (firstRUN) {
    for (int i = 0; i < 100; i++) {
      MENUval[i] = false;
    }
  }
  firstRUN = false;

  MENUlbl[11] = "INFO";                                                   // Menu labels
  MENUlbl[12] = "MANUAL MODE";
  MENUlbl[13] = "AUTO MODE";
  MENUlbl[14] = "--------------------";
  MENUlbl[21] = "X";
  MENUlbl[22] = "Y";
  MENUlbl[23] = "Z";
  MENUlbl[24] = "RST";
  MENUlbl[25] = "MOT";
  MENUlbl[26] = "HOM";
  MENUlbl[27] = "<";
  MENUlbl[MENUact] = ">" + MENUlbl[MENUact];

  lcd.clear();

  if (MENUact > 10 && MENUact < 20)                                       // sub-menu "MAIN"
  {
    lcd.setCursor(0, 0);
    lcd.print(MENUlbl[MENUact]);
    lcd.setCursor(1, 1);
    lcd.print(MENUlbl[MENUact + 1]);
  }




  while (MENUact > 10 && MENUact < 20) {                                  // Routine for MAIN Sub-Menu
    currentStateCLK = digitalRead(CLK);                                   // START Rotary Encoder Routine returning counter position and action for pushed button
    if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {
      if (digitalRead(DT) != currentStateCLK) {
        counter --;
        currentDir = "CCW";
      } else {
        counter ++;
        currentDir = "CW";
      }

      if (currentDir == "CCW") {                                          // turned knob without selection --> toggle menu
        MENUact = MENUact + 1;
      }
      else {
        MENUact = MENUact - 1;
      }
      MENUact = constrain(MENUact, 11, 13);
      MENUlbl[11] = "INFO";                                               // Update screen with newly selected menu item
      MENUlbl[12] = "MANUAL MODE";
      MENUlbl[13] = "AUTO MODE";
      MENUlbl[MENUact] = ">" + MENUlbl[MENUact];
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(MENUlbl[MENUact]);
      lcd.setCursor(1, 1);
      lcd.print(MENUlbl[MENUact + 1]);
    }
    lastStateCLK = currentStateCLK;

    int btnState = digitalRead(SW);                                       // Action for pushed button
    if (btnState == LOW) {
      if (millis() - lastButtonPress > 50) {
        MENUval[MENUact] = !MENUval[MENUact];
        if (MENUact == 11) {          // INFO Routine
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("KLAUS W - CNC_1");
          lcd.setCursor(0, 1);
          lcd.print("Version 0.1");
          delay(2000);
          MENUval[MENUact] = !MENUval[MENUact];
        }

        if (MENUact == 12) {                                              // go to MANUAL Routine
          MENUval[MENUact] = !MENUval[MENUact];
          MENUact = 21;
        }

        if (MENUact == 13) {                                              // go to AUTO Routine
          MENUval[MENUact] = !MENUval[MENUact];
          MENUact = 31;
        }

      }
      lastButtonPress = millis();
    }
    delay(1);                                                             // END Rotary Encoder Routine
  }

  if (MENUact > 20 && MENUact < 30)                                       // sub-menu "MANUAL"
  {
    lcd.clear();
    lcd.setCursor(2, 0);                                                  // sub-menu "MANUAL" needs to start with homing to reach 0,0,0
    lcd.print("GOING HOME...");
    homing();
    Xpos = 0;
    Ypos = 0;
    Zpos = 0;
    Xoffs = 0;
    Yoffs = 0;
    Zoffs = 0;

    lcd.clear();                                                          // initial LCD screen for sub-menu "MANUAL"
    lcd.setCursor(0, 0);
    lcd.print(MENUlbl[21]);
    lcd.print(Xpos);
    lcd.setCursor(6, 0);
    lcd.print(MENUlbl[22]);
    lcd.print(Ypos);
    lcd.setCursor(12, 0);
    lcd.print(MENUlbl[23]);
    lcd.print(Zpos);
    lcd.setCursor(0, 1);
    lcd.print(MENUlbl[24]);
    lcd.setCursor(5, 1);
    lcd.print(MENUlbl[25]);
    lcd.setCursor(10, 1);
    lcd.print(MENUlbl[26]);
    lcd.setCursor(15, 1);
    lcd.print(MENUlbl[27]);
  }

  while (MENUact > 20 && MENUact < 30) {                                  // Routine for MANUAL Sub-Menu
    currentStateCLK = digitalRead(CLK);                                   // START Rotary Encoder Routine returning counter position and action for pushed button
    if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {
      if (digitalRead(DT) != currentStateCLK) {
        counter --;
        currentDir = "CCW";
      } else {
        counter ++;
        currentDir = "CW";
      }

      if (MENUval[MENUact])   {                                           // turned knob WITH selection --> move selected axis

        if (MENUact == 21) {                                              // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
          if (currentDir == "CW") {
            Xpos = Xpos - 1;
            if (Xpos < 0) {
              Xpos = 0;
            }
            else {
              digitalWrite(X_DIR, LOW);
              digitalWrite(X_STP, HIGH);
              delayMicroseconds(200);
              digitalWrite(X_STP, LOW);
              delayMicroseconds(1800);
            }
          }
          else {
            Xpos = Xpos + 1;
            if (Xpos > 1500) {
              Xpos = 1500;
            }
            else {
              digitalWrite(X_DIR, HIGH);
              digitalWrite(X_STP, HIGH);
              delayMicroseconds(200);
              digitalWrite(X_STP, LOW);
              delayMicroseconds(1800);
            }
          }
          lcd.setCursor(2, 0);
          lcd.print("    ");
          lcd.setCursor(2, 0);
          lcd.print(int((Xpos - Xoffs) / Xfact));
        }

        if (MENUact == 22) {                                              // YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
          if (currentDir == "CW") {
            Ypos = Ypos - 1;
            if (Ypos < 0) {
              Ypos = 0;
            }
            else {
              digitalWrite(Y_DIR, LOW);
              digitalWrite(Y_STP, HIGH);
              delayMicroseconds(200);
              digitalWrite(Y_STP, LOW);
              delayMicroseconds(1800);
            }
          }
          else {
            Ypos = Ypos + 1;
            if (Ypos > 925) {
              Ypos = 925;
            }
            else {
              digitalWrite(Y_DIR, HIGH);
              digitalWrite(Y_STP, HIGH);
              delayMicroseconds(200);
              digitalWrite(Y_STP, LOW);
              delayMicroseconds(1800);
            }
          }
          lcd.setCursor(8, 0);
          lcd.print("    ");
          lcd.setCursor(8, 0);
          lcd.print(int((Ypos - Yoffs) / Yfact));
        }

        if (MENUact == 23) {                                              // ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
          if (currentDir == "CW") {
            Zpos = Zpos - 5;
            if (Zpos < 0) {
              Zpos = 0;
            }
            else {
              digitalWrite(Z_DIR, LOW);
              for (int i = 1; i < 6; i++) {
                digitalWrite(Z_STP, HIGH);
                delayMicroseconds(200);
                digitalWrite(Z_STP, LOW);
                delayMicroseconds(1800);
              }
            }
          }
          else {
            Zpos = Zpos + 5;
            if (Zpos > 3175) {
              Zpos = 3175;
            }
            else {
              digitalWrite(Z_DIR, HIGH);
              for (int i = 1; i < 6; i++) {
                digitalWrite(Z_STP, HIGH);
                delayMicroseconds(200);
                digitalWrite(Z_STP, LOW);
                delayMicroseconds(1800);
              }
            }
          }
          lcd.setCursor(14, 0);
          lcd.print("  ");
          lcd.setCursor(14, 0);
          lcd.print(int((Zpos - Zoffs) / Zfact));
        }
      }

      else {
        if (currentDir == "CCW") {                                        // turned knob without selection --> toggle menu
          MENUact = MENUact + 1;
        }
        else {
          MENUact = MENUact - 1;
        }
        MENUact = constrain(MENUact, 21, 27);
        MENUlbl[11] = "INFO";                                             // Update screen with newly selected menu item
        MENUlbl[12] = "MANUAL MODE";
        MENUlbl[13] = "AUTO MODE";
        MENUlbl[21] = "X";
        MENUlbl[22] = "Y";
        MENUlbl[23] = "Z";
        MENUlbl[24] = "RST";
        MENUlbl[25] = "MOT";
        MENUlbl[26] = "HOM";
        MENUlbl[27] = "<";
        MENUlbl[MENUact] = ">" + MENUlbl[MENUact];
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(MENUlbl[21]);
        lcd.print(int((Xpos - Xoffs) / Xfact));
        lcd.setCursor(6, 0);
        lcd.print(MENUlbl[22]);
        lcd.print(int((Ypos - Yoffs) / Yfact));
        lcd.setCursor(12, 0);
        lcd.print(MENUlbl[23]);
        lcd.print(int((Zpos - Zoffs) / Zfact));
        lcd.setCursor(0, 1);
        lcd.print(MENUlbl[24]);
        lcd.setCursor(5, 1);
        lcd.print(MENUlbl[25]);
        lcd.setCursor(10, 1);
        lcd.print(MENUlbl[26]);
        lcd.setCursor(14, 1);
        lcd.print(MENUlbl[27]);
      }
    }
    lastStateCLK = currentStateCLK;

    int btnState = digitalRead(SW);                                       // Action for pushed button
    if (btnState == LOW) {
      if (millis() - lastButtonPress > 50) {
        MENUval[MENUact] = !MENUval[MENUact];
        if (MENUact == 24) {          // RESET Routine
          Xoffs = Xpos;
          Yoffs = Ypos;
          Zoffs = Zpos;
          MENUval[MENUact] = !MENUval[MENUact];
        }

        if (MENUact == 25) {                                              // MOTOR Routine
          MOTState = !MOTState;
          if (MOTState) {
            digitalWrite(MOT, HIGH);
          }
          else {
            digitalWrite(MOT, LOW);
          }
          MENUval[MENUact] = !MENUval[MENUact];
        }

        if (MENUact == 26) {                                              // HOME Routine
          homing();
          Xpos = 0;
          Ypos = 0;
          Zpos = 0;
          Xoffs = 0;
          Yoffs = 0;
          Zoffs = 0;
          MENUval[MENUact] = !MENUval[MENUact];
        }

        if (MENUact == 27) {                                              // back to main menu Routine
          MENUact = 11;
          MENUval[MENUact] = !MENUval[MENUact];
        }

      }
      lastButtonPress = millis();
    }
    delay(1);                                                             // END Rotary Encoder Routine
  }



  if (MENUact > 30 && MENUact < 40)                              // sub-menu "AUTO"
  {
    if (sdin) {
      root.close();
    }
    digitalWrite (53, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AUTO MODE");
    delay(1000);
    lcd.setCursor(0, 0);
    lcd.print("SD card...");
    if (!SD.begin(chipSelect)) {
      lcd.setCursor(0, 1);
      lcd.print("...failed.  ");
      MENUact = 11;
      delay(2000);
    }
    else {
      lcd.setCursor(0, 1);
      lcd.print("...ready.    ");
      sdin = true;
      delay(2000);
      root = SD.open("/");
      printDirectory(root);
      lcd.clear();
      if (filePos == 0) {
        lcd.setCursor(0, 1);
        sdin = false;
        lcd.print("no .cnc file....");
        MENUact = 11;
        delay(2000);
      }
      else {
        lcd.setCursor(0, 0);
        lcd.print(">" + fileList[0]);
        lcd.setCursor(1, 1);
        lcd.print(fileList[1]);
        if (filePos > 8) {
          filePos = 8;
        }
      }
    }
  }



  while (MENUact > 30 && MENUact < 40) {                                    // Routine for AUTO Sub-Menu
    currentStateCLK = digitalRead(CLK);                                     // START Rotary Encoder Routine returning counter position and action for pushed button
    if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {
      if (digitalRead(DT) != currentStateCLK) {
        counter --;
        currentDir = "CCW";
      } else {
        counter ++;
        currentDir = "CW";
      }

      if (currentDir == "CCW") {                                             // turned knob without selection --> toggle menu
        MENUact = MENUact + 1;
      }
      else {
        MENUact = MENUact - 1;
      }
      MENUact = constrain(MENUact, 31, 31 + filePos);
      for (int i = 0; i < (filePos + 1); i++) {                              // Update screen with newly selected menu item
        MENUlbl[31 + i] = fileList[i];
      }

      MENUlbl[31 + filePos] = "<";
      MENUlbl[MENUact] = ">" + MENUlbl[MENUact];
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(MENUlbl[MENUact]);
      lcd.setCursor(1, 1);
      lcd.print(MENUlbl[MENUact + 1]);
    }
    lastStateCLK = currentStateCLK;

    int btnState = digitalRead(SW);                                         // Action for pushed button
    if (btnState == LOW) {
      if (millis() - lastButtonPress > 50) {
        MENUval[MENUact] = !MENUval[MENUact];

        if (MENUact == 31 + filePos) {                                      // go to MAIN
          MENUval[MENUact] = !MENUval[MENUact];
          MENUact = 11;
        }

        else {                                                              // file reading routine
          root = SD.open(fileList[MENUact - 31]);
          String recStrg = readStrg();
          if (recStrg != "KWCNC") {                                         // further check that the file is a valid one --> must have specific string to start
            MENUval[MENUact] = !MENUval[MENUact];
            MENUact = 11;                                                   // if not go back to main menu
            root.close();
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("no valid file!!");
            delay(2000);
          }
          else {
            double totDur = readStrg().toDouble();                          // next entry after start string is the total program duration in ms
            double actDur = 0;                                              // start values for elapsed program duration (ms), pause time per step and steps per call
            int stepTime = 0;
            int steps = 1;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(fileList[MENUact - 31]);
            lcd.setCursor(4, 1);
            lcd.print("%");
            lcd.setCursor(0, 1);
            lcd.print("0");                                                // Screen for AUTO mode: filename and elapsed time in %
            while (recStrg != "E") {                                       // last command in each file needs to be "E" (end)...until then read other commands
              recStrg = readStrg();
              if (recStrg == "T") {                                        // "T" is followed by a number: new value for pause between steps (ms)
                int stepTime = readStrg().toInt();
              }
              if (recStrg == "X") {                                        // "X" is followed by the numberof steps: negative = towards HOME; positive = away from HOME
                int steps = readStrg().toInt();
                if (steps < 0) {
                  digitalWrite(X_DIR, LOW);
                  steps = -steps;
                }
                else {
                  digitalWrite(X_DIR, HIGH);
                }
                for (int i = 0; i < steps; i++) {                         // perform number of steps for X
                  digitalWrite(X_STP, HIGH);
                  delayMicroseconds(1000);
                  digitalWrite(X_STP, LOW);
                  delay(stepTime + 1);
                }
                actDur = actDur + ((stepTime + 2) * steps);
                lcd.setCursor(0, 1);
                lcd.print(int((actDur / totDur) * 100));
              }

              if (recStrg == "Y") {                                       // "Y" is followed by the numberof steps: negative = towards HOME; positive = away from HOME
                int steps = readStrg().toInt();
                if (steps < 0) {
                  digitalWrite(Y_DIR, LOW);
                  steps = -steps;
                }
                else {
                  digitalWrite(Y_DIR, HIGH);
                }
                for (int i = 0; i < steps; i++) {                        // perform number of steps for Y
                  digitalWrite(Y_STP, HIGH);
                  delayMicroseconds(1000);
                  digitalWrite(Y_STP, LOW);
                  delay(stepTime + 1);
                }
                actDur = actDur + ((stepTime + 2) * steps);
                lcd.setCursor(0, 1);
                lcd.print(int((actDur / totDur) * 100));
              }

              if (recStrg == "Z") {                                      // "Z" is followed by the numberof steps: negative = towards HOME; positive = away from HOME
                int steps = readStrg().toInt();
                if (steps < 0) {
                  digitalWrite(Z_DIR, LOW);
                  steps = -steps;
                }
                else {
                  digitalWrite(Z_DIR, HIGH);
                }
                for (int i = 0; i < steps; i++) {                       // perform number of steps for Z
                  digitalWrite(Z_STP, HIGH);
                  delayMicroseconds(1000);
                  digitalWrite(Z_STP, LOW);
                  delay(stepTime + 1);
                }
                actDur = actDur + ((stepTime + 2) * steps);
                lcd.setCursor(0, 1);
                lcd.print(int((actDur / totDur) * 100));
              }

              if (recStrg == "M") {                                    // "M" to switch motor on/off
                MOTState = !MOTState;
                if (MOTState) {
                  digitalWrite(MOT, HIGH);
                }
                else {
                  digitalWrite(MOT, LOW);
                }
              }
            }
          }
          // close the file:                                          // after "E" was arrived....
          root.close();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Finished!");
          delay(2000);
          MENUval[MENUact] = !MENUval[MENUact];
          MENUact = 11;
        }
      }
      lastButtonPress = millis();
    }
    delay(1);                                                         // END Rotary Encoder Routine
  }

}

void homing() {                                                       // Homing routine: one by one move each axis until end-stop switch triggered
  digitalWrite(X_DIR, LOW);
  digitalWrite(Y_DIR, LOW);
  digitalWrite(Z_DIR, LOW);
  while (digitalRead(X_END) == HIGH) {
    digitalWrite(X_STP, HIGH);
    delayMicroseconds(200);
    digitalWrite(X_STP, LOW);
    delayMicroseconds(1800);
  }
  while (digitalRead(Y_END) == HIGH) {
    digitalWrite(Y_STP, HIGH);
    delayMicroseconds(200);
    digitalWrite(Y_STP, LOW);
    delayMicroseconds(1800);
  }
  while (digitalRead(Z_END) == HIGH) {
    digitalWrite(Z_STP, HIGH);
    delayMicroseconds(200);
    digitalWrite(Z_STP, LOW);
    delayMicroseconds(1800);
  }
}


void printDirectory(File dir) {                                        // Read all .cnc file from the SD card
  filePos = 0;
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {                                                     // no more files
      break;
    }
    String fileNam = entry.name() ;

    if (fileNam.endsWith(".CNC")) {
      fileList[filePos] = fileNam;
      filePos = filePos + 1;
    }
    entry.close();
  }
}


String readStrg() {                                                    // read the opened file on the SD card one line at a time (function returns string after "cr" is reached)
  char inStrg = 0;
  String fullStrg = "";
  if (root) {                                                          // read from the file until there's nothing else in it:
    while (root.available()) {
      char inStrg = root.read();
      if (inStrg == 13) {
        root.read();
        break;
      }
      fullStrg = fullStrg + inStrg;
    }
    return fullStrg;
  }
  else {                                                                // if the file didn't open, print an error:
    return "ERROR";
  }

}
