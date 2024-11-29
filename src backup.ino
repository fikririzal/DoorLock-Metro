#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include "EEPROM.h"

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

#define RST_PIN 17                // Configurable, see typical pin layout above
#define SS_PIN 5                  // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

#define NUM_ROWS 4 // four rows
#define NUM_COLS 4 // three columns
char keys[NUM_ROWS][NUM_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[NUM_ROWS] = {13, 12, 14, 27}; // connect to the row pinouts of the keypad
byte colPins[NUM_COLS] = {26, 25, 33, 32}; // connect to the column pinouts of the keypad
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, NUM_ROWS, NUM_COLS);

#define Enter '#'
#define Back '*'

// EEPROM Addres
EEPROMClass storedPassword_eeprom("eeprom0");
EEPROMClass num_storedCard_eeprom("eeprom1");
EEPROMClass storedCard_eeprom("eeprom2");
EEPROMClass idle_time_eeprom("eeprom3");

// Password Variable
#define MAX_PASSWORD 17
char masterPassword[MAX_PASSWORD] = "14122004";
char storedPassword[MAX_PASSWORD] = "";
char typePassword[MAX_PASSWORD] = "";
const char blank[MAX_PASSWORD] = "";
static uint8_t typeIndex = 0;

char prev_typePassword[MAX_PASSWORD] = "";
char verif_typePassword[MAX_PASSWORD] = "";
char prev_storedPassword[MAX_PASSWORD] = "";

// RFID Variable
#define MAX_RFID 50
#define MAX_RFID_BYTE 4
#define storedCardSize (MAX_RFID * MAX_RFID_BYTE)
uint8_t num_storedCard = 0;
uint8_t MasterCard[MAX_RFID_BYTE] = {0x0D, 0xCD, 0xE5, 0x36};
uint8_t storedCard[MAX_RFID][MAX_RFID_BYTE] = {};
uint8_t scannedCard[MAX_RFID_BYTE] = {};

uint8_t prev_num_storedCard = 0;
uint8_t prev_storedCard[MAX_RFID][MAX_RFID_BYTE] = {};

const uint8_t r[MAX_RFID_BYTE] = {};

// Menu Variable
const uint8_t MAX_ITEM_LENGTH = 16;

const uint8_t num_prog_Home_screen = 7;
#define HOME 0
#define SETTING 1

#define HOME_OPEN 01
#define HOME_CLOSE 02

#define HOME_OPEN_SETT 011
#define HOME_OPEN_NORM 012

const uint8_t num_setting_screen = 6;
#define SETT_HOME 10

const uint8_t num_CHG_PSWD_screen = 7;
#define SETT_CHG_PSWD 12

const uint8_t num_ADD_RFID_screen = 9;
#define SETT_ADD_RFID 13
#define SETT_DEL_RFID 14

const uint8_t num_DELALL_RFID_items = 4;
#define SETT_DELALL_RFID 15

#define SETT_CHG_PSWD_ADD 110
#define SETT_CHG_PSWD_VER 111
uint8_t prog_sett_chg_pswdState = SETT_CHG_PSWD_ADD;

char setting_menu_items[num_setting_screen][MAX_ITEM_LENGTH] = {
    {" -- "},
    {"CHG Passcode"},
    {"ADD a Card"},
    {"DEL a Card"},
    {"DEL ALL Card"},
    {"IDLE Timer"},
};

char ADD_RFID_items[num_ADD_RFID_screen][MAX_ITEM_LENGTH] = {
    {" -- "},
    {"Scan New RFID"},
    {"Err N Support"},
    {"Card id "},
    {"add in num "},
    {"Card alrdy In"},
    {"Storge Num "},
    {"Cant ADD this"},
    {"Storage Full"},
};

char DELALL_RFID_items[num_DELALL_RFID_items][MAX_ITEM_LENGTH] = {
    {" -- "},
    {"Enter Passcode"},
    {"Passcode Inval"},
    {"All RFID Deltd"},
};

char CHG_PSWD_items[num_CHG_PSWD_screen][MAX_ITEM_LENGTH] = {
    {" -- "},
    {"Ent New Pass"},
    {"Ver New Pass"},
    {"Pass Invalid"},
    {"New Pass Valid"},
    {"Passcode Set"},
    {"Pass is Null"},
};

uint8_t programState = HOME;
uint8_t programSubHOMEState = HOME_OPEN;
uint8_t programSubHOME_OPEN_State = HOME_OPEN_NORM;
uint8_t programSubSETTINGState = SETT_HOME;

int8_t current_pos = 0;
int8_t previous_item = num_setting_screen - 1;
int8_t previous_screen = 2;
int8_t selected_item = 1;
int8_t next_item = selected_item + 1;
int8_t current_screen = 1;
uint16_t delay_screen_display = 2000;

// UID2Display  Variable
String prev_UID = "";
String current_UID = "";
String next_UID = "";

// Idle Variable
uint32_t prev_change_home = 0;
uint16_t idle_time = 0;
uint16_t last_idle_time = 0;
bool change = true;

// Doorlock Pin And Config
#define Servo_PIN 16
#define Servo_Ch 0
#define Servo_Freq 50
#define Servo_Res 12

#define unlockPB_pin 4
#define doorSens_pin 35

#define Open false
#define Close true

const uint16_t servo_uS_LOCK = 500;
const uint16_t servo_uS_UNLOCK = 2000;

// Buzzer Pin And Config
#define Buzzer_PIN 15
#define Buzzer_Ch 1

#define melodyLenght 12

#define NT_C 0
#define NT_Cs 1
#define NT_D 2
#define NT_Eb 3
#define NT_E 4
#define NT_F 5
#define NT_Fs 6
#define NT_G 7
#define NT_Gs 8
#define NT_A 9
#define NT_Bb 10
#define NT_B 11
#define OFF 12

// delay Melody
#define short1 3
#define short2 5

#define middle1 10
#define middle2 15

#define long1 20
#define long2 25

// Melody if delay == 0, melody stop
uint8_t melodyA[3][melodyLenght] = {
    {NT_C, NT_Cs, NT_D, NT_Eb, NT_E, NT_F, NT_Fs, NT_G, NT_Gs, NT_A, NT_Bb, NT_B}, // Note
    {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},                                          // Octave
    {200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200},                  // delay 5 == 50ms
};

uint8_t kuruKuru[3][melodyLenght] = {
    {NT_Fs, NT_G, OFF, NT_Fs, NT_G},                      // Note
    {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},                 // Octave
    {10, 10, 2, 10, 15, 0, 200, 200, 200, 200, 200, 200}, // delay 5 == 50ms
};

uint8_t m_Open[3][melodyLenght] = {
    {NT_A, OFF, NT_Eb, OFF, NT_Eb},                 // Note
    {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},           // Octave
    {middle1, short1, middle2, short1, middle2, 0}, // delay 5 == 50ms
};

uint8_t m_Close[3][melodyLenght] = {
    {NT_Eb, OFF, NT_A, OFF, NT_A},                  // Note
    {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},           // Octave
    {middle1, short1, middle2, short1, middle2, 0}, // delay 5 == 50ms
};

uint8_t m_doorOPEN[3][melodyLenght] = {
    {NT_A, OFF, NT_A},                    // Note
    {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}, // Octave
    {middle1, short1, middle1, 0},        // delay 5 == 50ms
};

uint8_t m_keypadClick[3][melodyLenght] = {
    {NT_E},                               // Note
    {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}, // Octave
    {middle1, 0},                         // delay 5 == 50ms
};

uint8_t m_FunckeypadClick[3][melodyLenght] = {
    {NT_B},                               // Note
    {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}, // Octave
    {middle1, 0},                         // delay 5 == 50ms
};

uint8_t m_IncreasekeypadClick[3][melodyLenght] = {
    {NT_C},                               // Note
    {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}, // Octave
    {middle1, 0},                         // delay 5 == 50ms
};

uint8_t m_DecreasekeypadClick[3][melodyLenght] = {
    {NT_A},                               // Note
    {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}, // Octave
    {middle1, 0},                         // delay 5 == 50ms
};

void playMelody(uint8_t (*arr1)[melodyLenght]);

void setup()
{
  Serial.begin(115200);

  pinMode(unlockPB_pin, INPUT_PULLUP);
  pinMode(doorSens_pin, INPUT);
  pinMode(Buzzer_PIN, OUTPUT);

  setDoorStatus(!digitalRead(doorSens_pin));
  if (getDoorStatus() == Close)
  {
    servoLock(servo_uS_LOCK);
    programSubHOMEState = HOME_CLOSE;
  }
  else
  {
    servoUnlock(servo_uS_UNLOCK);
    programSubHOMEState = HOME_OPEN;
  }

  storedPassword_eeprom.begin(MAX_PASSWORD + 1);
  num_storedCard_eeprom.begin(sizeof(uint8_t) + 1);
  storedCard_eeprom.begin(storedCardSize + 1);
  idle_time_eeprom.begin(sizeof(uint16_t) + 1);
  delay(1000);

  uint16_t i;
  idle_time_eeprom.get(0, i);
  if (i != 10)
  {
    idle_time_eeprom.put(0, 10);
    delay(1000);
    idle_time_eeprom.commit();
  }

  /*
  storedPassword_eeprom.put(0, storedPassword);
  num_storedCard_eeprom.put(0, num_storedCard);
  storedCard_eeprom.put(0, storedCard);
  delay(1000);

  storedPassword_eeprom.commit();
  num_storedCard_eeprom.commit();
  storedCard_eeprom.commit();

  memset(storedPassword, '\0', MAX_PASSWORD);
  memset(storedCard, 0, MAX_RFID * MAX_RFID_BYTE);
  num_storedCard = 0;
  */

  storedPassword_eeprom.get(0, storedPassword);
  num_storedCard_eeprom.get(0, num_storedCard);
  storedCard_eeprom.get(0, storedCard);
  idle_time_eeprom.get(0, idle_time);

  memcpy(prev_storedPassword, storedPassword, MAX_PASSWORD);
  last_idle_time = idle_time;
  prev_num_storedCard = num_storedCard;
  memcpy(prev_storedCard, storedCard, storedCardSize);

  Serial.println();
  Serial.println(storedPassword);
  Serial.println(num_storedCard);
  for (int i = 0; i < MAX_RFID; i++)
  {
    for (int j = 0; j < MAX_RFID_BYTE; j++)
    {
      Serial.print(storedCard[i][j]);
      Serial.print(' ');
    }
    Serial.print(' ');
    Serial.print(i);
    Serial.println();
  }
  prev_change_home = millis();

  kpd.setDebounceTime(10); // setDebounceTime(mS)
  SPI.begin();             // Init SPI bus
  mfrc522.PCD_Init();      // Init MFRC522
  delay(4);                // Optional delay. Some board do need more time after init to be ready, see Readme
  lcd.init();              // initialize the lcd
  lcd.backlight();
  lcd.clear();
  // playMelody(kuruKuru);
}

void loop()
{
  // playMelody(kururu);
  buzzMelody_runtime();

  EEprom_Update();
  switch (programState)
  {
  case HOME:
    prog_Home();
    break;

  case SETTING:
    prog_setting();
    break;
  }
  doorLock_handler();
}

// -----------------------------------------------------------------------------------
// ------------------------------------ Home PROGRAMS --------------------------------
// -----------------------------------------------------------------------------------

void prog_Home()
{
  switch (programSubHOMEState)
  {
  case HOME_CLOSE:
    process_HOME_CLOSE();
    break;

  case HOME_OPEN:
    process_HOME_OPEN();
    break;
  }
}

void process_HOME_CLOSE()
{
  lcd.setCursor(0, 0);
  lcd.print("      LOCK      ");

  uint32_t now = millis();
  if (now - prev_change_home > (idle_time * 1000))
  {
    if (change)
    {
      lcd.noBacklight();
      lcd.clear();
      clearTypeArray();
      change = false;
    }
  }
  else
  {
    change = true;
    lcd.backlight();
  }

  char key = kpd.getKey();
  switch (kpd.getState())
  {
  case PRESSED:
    switch (key)
    {
    case Back:
    {
      playMelody(m_FunckeypadClick);
      clearTypeArray();
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }
    break;
    case Enter:
      playMelody(m_FunckeypadClick);
      switch (evaluateTypeArray())
      {
      case 1:
        Serial.println("VALID");
        lcd.clear();
        if (getDoorStatus() == Close)
        {
          Unlock();
          prev_change_home = millis();
          programSubHOMEState = HOME_OPEN;
          programSubHOME_OPEN_State = HOME_OPEN_NORM;
        }

        break;
      case 100:
        Serial.println("MASTER");
        programState = SETTING;
        break;
      default:
        Serial.println("INVALID");
        lcd.backlight();
        lcd.setCursor(0, 0);
        lcd.print("Passcode INVALID");
        lcd.setCursor(0, 1);
        lcd.print("                ");
        delay(delay_screen_display);
        break;
      }
      break;

    default:
      if (key != '\0')
      {
        prev_change_home = millis();
        playMelody(m_keypadClick);
        lcd.setCursor(typeIndex, 1);
        lcd.print("*");

        Serial.println(key);
        addChar2TypeArray(key);
      }
      break;
    }
    break;
  }

  if (!mfrc522.PICC_IsNewCardPresent())
    return;

  if (!mfrc522.PICC_ReadCardSerial())
    return;

  lcd.backlight();

  mfrc522.PICC_HaltA();

  if (mfrc522.uid.size > 4)
  {
    Serial.println("card not supported");
    return;
  }
  current_UID = "";

  for (uint8_t i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(i == 0 ? (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") : (mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    Serial.print(mfrc522.uid.uidByte[i], HEX);

    current_UID += mfrc522.uid.uidByte[i] < 0x10 ? "0" : "";
    current_UID += String(mfrc522.uid.uidByte[i], HEX);
    current_UID.toUpperCase();

    scannedCard[i] = mfrc522.uid.uidByte[i];
    if (i == mfrc522.uid.size - 1)
      Serial.println();
  }

  uint8_t card = checkCard();
  switch (card)
  {
  case 100:
    Serial.println("MASTER");
    prev_change_home = millis();
    programState = SETTING;
    break;
  case 200:
    Serial.println("Unregistered Card");

    lcd.setCursor(0, 0);
    lcd.print("    ");
    lcd.print(current_UID);
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print("Unregistered UID");
    prev_change_home = millis();
    delay(delay_screen_display);
    break;
  default:
    Serial.print("card registered at number ");
    Serial.println(card);

    lcd.setCursor(0, 1);
    lcd.print(" Registered UID ");
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.print(current_UID);
    lcd.print(" Num ");
    lcd.print(card);
    prev_change_home = millis();
    if (getDoorStatus() == Close)
    {
      Unlock();
      programSubHOMEState = HOME_OPEN;
      programSubHOME_OPEN_State = HOME_OPEN_NORM;
    }
    break;
  }
}

bool idle_status = false;

void process_HOME_OPEN()
{
  uint32_t now = millis();
  if (now - prev_change_home > (idle_time * 1000))
  {
    bool i = ((millis() % 2000) > 1000) > 0 ? 1 : 0;
    if (i && idle_status)
    {
      playMelody(m_doorOPEN);
      idle_status = false;
    }

    if (!i)
      idle_status = true;

    prev_change_home = now;
  }

  switch (programSubHOME_OPEN_State)
  {
  case HOME_OPEN_NORM:
    process_HOME_OPEN_NORM();
    break;
  case HOME_OPEN_SETT:
    process_HOME_OPEN_SETT();
    break;
  }
}

void process_HOME_OPEN_NORM()
{
  lcd.setCursor(0, 0);
  lcd.print("     UNLOCK     ");

  char key = kpd.getKey();
  if (kpd.getState() == PRESSED)
  {
    switch (key)
    {
    case 'C':
      playMelody(m_FunckeypadClick);
      if (getDoorStatus() == Close)
      {
        Lock();
        prev_change_home = millis();
        programSubHOMEState = HOME_CLOSE;
        programSubHOME_OPEN_State = HOME_OPEN_NORM;
      }
      break;

    case 'A':
      playMelody(m_FunckeypadClick);
      lcd.clear();
      prev_change_home = millis();
      programSubHOME_OPEN_State = HOME_OPEN_SETT;
      break;
    }
  }
}

void process_HOME_OPEN_SETT()
{
  lcd.setCursor(0, 0); ////////
  lcd.print(" INPUT PASSCODE ");

  char key = kpd.getKey();
  switch (kpd.getState())
  {
  case PRESSED:
    switch (key)
    {
    case Back:
      playMelody(m_FunckeypadClick);
      if (memcmp(typePassword, blank, MAX_PASSWORD) == 0)
      {
        prev_change_home = millis();
        programSubHOME_OPEN_State = HOME_OPEN_NORM;
        return;
      }
      clearTypeArray();
      lcd.clear();
      break;
    case Enter:
      playMelody(m_FunckeypadClick);
      switch (evaluateTypeArray())
      {
      case 1:
        Serial.println("VALID");
        lcd.clear();
        prev_change_home = millis();
        programState = SETTING;
        break;
      default:
        Serial.println("INVALID");
        lcd.setCursor(0, 0);
        lcd.print("Passcode INVALID");
        lcd.setCursor(0, 1);
        lcd.print("                ");
        delay(delay_screen_display);
        prev_change_home = millis();
        break;
      }
      break;

    default:
      if (key != '\0')
      {
        prev_change_home = millis();
        playMelody(m_keypadClick);
        lcd.setCursor(typeIndex, 1);
        lcd.print("*");

        Serial.println(key);
        addChar2TypeArray(key);
      }
      break;
    }
    break;
  }

  if (!mfrc522.PICC_IsNewCardPresent())
    return;

  if (!mfrc522.PICC_ReadCardSerial())
    return;

  mfrc522.PICC_HaltA();

  if (mfrc522.uid.size > 4)
  {
    Serial.println("card not supported");
    return;
  }

  current_UID = "";
  for (uint8_t i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(i == 0 ? (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") : (mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    Serial.print(mfrc522.uid.uidByte[i], HEX);

    current_UID += mfrc522.uid.uidByte[i] < 0x10 ? "0" : "";
    current_UID += String(mfrc522.uid.uidByte[i], HEX);
    current_UID.toUpperCase();

    scannedCard[i] = mfrc522.uid.uidByte[i];
    if (i == mfrc522.uid.size - 1)
      Serial.println();
  }

  uint8_t card = checkCard();
  switch (card)
  {
  case 100:
    Serial.println("MASTER");
    programState = SETTING;
    break;
  }
}

// -----------------------------------------------------------------------------------
// --------------------------------- Setting Menu PROGRAMS ---------------------------
// -----------------------------------------------------------------------------------

void prog_setting()
{
  switch (programSubSETTINGState)
  {
  case SETT_HOME:
    prog_sett_home();
    break;
  case SETT_CHG_PSWD:
    prog_sett_chg_pswd();
    break;
  case SETT_ADD_RFID:
    prog_sett_add_rfid();
    break;
  case SETT_DEL_RFID:
    prog_sett_del_rfid();
    break;
  case SETT_DELALL_RFID:
    prog_sett_delALL_rfid();
    break;
  }
}

void prog_sett_home()
{
  if (selected_item != previous_screen)
  {
    Serial.println(setting_menu_items[selected_item]);

    if (selected_item < previous_screen)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(">");
      lcd.print(setting_menu_items[selected_item]);
      if (selected_item == 5)
      {
        lcd.setCursor(idle_time < 10 ? 14 : 13, 0);
        lcd.print(String(idle_time));
        lcd.print("s");
      }
      lcd.setCursor(0, 1);
      lcd.print(" ");
      lcd.print(setting_menu_items[next_item]);
      if (next_item == 5)
      {
        lcd.setCursor(idle_time < 10 ? 14 : 13, 1);
        lcd.print(String(idle_time));
        lcd.print("s");
      }
    }
    else if (selected_item > previous_screen)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" ");
      lcd.print(setting_menu_items[previous_item]);
      if (previous_item == 5)
      {
        lcd.setCursor(idle_time < 10 ? 14 : 13, 0);
        lcd.print(String(idle_time));
        lcd.print("s");
      }
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.print(setting_menu_items[selected_item]);
      if (selected_item == 5)
      {
        lcd.setCursor(idle_time < 10 ? 14 : 13, 1);
        lcd.print(String(idle_time));
        lcd.print("s");
      }
    }
    previous_screen = selected_item;
  } // END if (selected_item != previous_screen)

  if (selected_item == 5)
  {
  }

  char key = kpd.getKey();
  switch (kpd.getState())
  {
  case PRESSED:
    switch (key)
    {
    case 'A':
      playMelody(m_IncreasekeypadClick);
      selected_item--;
      if (selected_item <= 0)
      {
        previous_item = num_setting_screen - 2;
        selected_item = num_setting_screen - 1;
        next_item = 0;
      }
      else
      {
        previous_item = selected_item - 1;
        next_item = selected_item + 1;
      }

      break;
    case 'B':
      playMelody(m_DecreasekeypadClick);
      selected_item++;
      if (selected_item > num_setting_screen - 1)
      {
        previous_item = num_setting_screen - 1;
        selected_item = 1;
        next_item = selected_item + 1;
      }
      else
      {
        previous_item = selected_item - 1;
        next_item = selected_item + 1;
      }

      break;
    case 'C':

      if (selected_item == 5)
      {
        playMelody(m_IncreasekeypadClick);
        idle_time++;
        previous_screen = 0;
      }
      break;
    case 'D':

      if (selected_item == 5)
      {
        if (idle_time > 2)
        {
          playMelody(m_DecreasekeypadClick);
          idle_time--;
          previous_screen = 0;
        }
      }
      break;
    case Enter:
      playMelody(m_FunckeypadClick);
      switch (selected_item)
      {
      case 1:
        reset_setting_screen();
        programSubSETTINGState = SETT_CHG_PSWD;
        break;

      case 2:
        reset_setting_screen();
        programSubSETTINGState = SETT_ADD_RFID;
        break;

      case 3:
        reset_setting_screen();
        programSubSETTINGState = SETT_DEL_RFID;
        break;

      case 4:
        reset_setting_screen();
        programSubSETTINGState = SETT_DELALL_RFID;
        break;
      }
      break;
    case Back:
      playMelody(m_FunckeypadClick);
      reset_setting_screen();
      reset_typePasscode();
      prev_change_home = millis();
      programSubSETTINGState = SETT_HOME;
      programState = HOME;
      programSubHOME_OPEN_State = HOME_OPEN_NORM;
      break;
    } // END switch (key)
    break;
  } // END switch (kpd.getState())
}

void prog_sett_chg_pswd()
{
  if (current_screen != previous_screen)
  {
    Serial.println(CHG_PSWD_items[current_screen]);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(CHG_PSWD_items[current_screen]);

    previous_screen = current_screen;
    if (current_screen == 3)
    {
      current_screen = 2;
      delay(delay_screen_display);
    }
  } // END if (selected_item != previous_screen)

  if (memcmp(typePassword, prev_typePassword, MAX_PASSWORD) != 0)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(CHG_PSWD_items[current_screen]);
    lcd.setCursor(0, 1);
    lcd.print(typePassword);

    memset(prev_typePassword, '\0', MAX_PASSWORD);
    memcpy(prev_typePassword, typePassword, MAX_PASSWORD);
  }

  switch (prog_sett_chg_pswdState)
  {
  case SETT_CHG_PSWD_ADD:
  {
    process_SETT_CHG_PSWD_ADD();
  }
  break;

  case SETT_CHG_PSWD_VER:
  {
    process_SETT_CHG_PSWD_VER();
  }
  break;
  }
}

void prog_sett_add_rfid()
{

  if (current_screen != previous_screen)
  {
    Serial.println(ADD_RFID_items[current_screen]);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(ADD_RFID_items[current_screen]);

    previous_screen = current_screen;
    if (current_screen > 6)
    {
      delay(delay_screen_display);
      reset_setting_screen();
    }

  } // END if (selected_item != previous_screen)

  char key = kpd.getKey();
  if (kpd.getState() == PRESSED && key == Back)
  {
    Serial.println("BACK");
    programSubSETTINGState = SETT_HOME;
    reset_setting_screen();
  }

  if (!mfrc522.PICC_IsNewCardPresent())
    return;

  if (!mfrc522.PICC_ReadCardSerial())
    return;

  mfrc522.PICC_HaltA();

  if (mfrc522.uid.size > 4)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(ADD_RFID_items[2]);
    Serial.println("card not supported");
    delay(delay_screen_display);
    reset_setting_screen();
    return;
  }

  String uid = "";
  for (uint8_t i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(i == 0 ? (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") : (mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    uid += mfrc522.uid.uidByte[i] < 0x10 ? "0" : "";
    uid += String(mfrc522.uid.uidByte[i], HEX);
    uid.toUpperCase();
    scannedCard[i] = mfrc522.uid.uidByte[i];
    if (i == mfrc522.uid.size - 1)
    {
      Serial.println();
    }
  }

  uint8_t config = addScannedCard2StoredCard();
  switch (config)
  {
  case 100:
    current_screen = 7;
    break;

  case 200:
    current_screen = 8;
    break;

  default:
    if (config > 200)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(ADD_RFID_items[5]);
      lcd.setCursor(0, 1);
      lcd.print(ADD_RFID_items[6]);
      lcd.print(config - 201);
      delay(delay_screen_display);
      Serial.println(ADD_RFID_items[5]);
      Serial.print(ADD_RFID_items[6]);
      Serial.println(config - 201);
      programSubSETTINGState = SETT_HOME;
      reset_setting_screen();
    }
    else
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(ADD_RFID_items[3]);
      lcd.print(uid);
      lcd.setCursor(0, 1);
      lcd.print(ADD_RFID_items[4]);
      lcd.print(config);
      delay(delay_screen_display);
      Serial.print(ADD_RFID_items[3]);
      Serial.println(uid);
      Serial.print(ADD_RFID_items[4]);
      Serial.println(config);
      programSubSETTINGState = SETT_HOME;
      reset_setting_screen();
    }
    break;
  }
}

void prog_sett_del_rfid()
{
  if (selected_item != previous_screen)
  {
    prev_UID = "";
    current_UID = "";
    next_UID = "";

    for (uint8_t i = 0; i < MAX_RFID_BYTE; i++)
    {
      Serial.print(i == 0 ? (storedCard[selected_item - 1][i] < 0x10 ? "0" : "") : (storedCard[selected_item - 1][i] < 0x10 ? " 0" : " "));
      Serial.print(storedCard[selected_item - 1][i], HEX);

      if (i == MAX_RFID_BYTE - 1)
      {
        Serial.print("   ");
        Serial.print(selected_item - 1);
        Serial.println();
      }
    }

    if (selected_item < previous_screen)
    {
      for (uint8_t i = 0; i < MAX_RFID_BYTE; i++)
      {
        current_UID += storedCard[selected_item - 1][i] < 0x10 ? "0" : "";
        current_UID += String(storedCard[selected_item - 1][i], HEX);
        current_UID.toUpperCase();

        next_UID += storedCard[next_item - 1][i] < 0x10 ? "0" : "";
        next_UID += String(storedCard[next_item - 1][i], HEX);
        next_UID.toUpperCase();
      }

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(">");
      lcd.print(current_UID);
      lcd.setCursor(0, 1);
      lcd.print(" ");
      lcd.print(next_UID);

      lcd.setCursor(selected_item - 1 < 10 ? 15 : 14, 0);
      lcd.print(selected_item - 1);
      lcd.setCursor(next_item - 1 < 10 ? 15 : 14, 1);
      lcd.print(next_item - 1);
    }
    else if (selected_item > previous_screen)
    {
      for (uint8_t i = 0; i < MAX_RFID_BYTE; i++)
      {
        prev_UID += storedCard[previous_item - 1][i] < 0x10 ? "0" : "";
        prev_UID += String(storedCard[previous_item - 1][i], HEX);
        prev_UID.toUpperCase();

        current_UID += storedCard[selected_item - 1][i] < 0x10 ? "0" : "";
        current_UID += String(storedCard[selected_item - 1][i], HEX);
        current_UID.toUpperCase();
      }

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" ");
      lcd.print(prev_UID);
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.print(current_UID);

      lcd.setCursor(previous_item - 1 < 10 ? 15 : 14, 0);
      lcd.print(previous_item - 1);
      lcd.setCursor(selected_item - 1 < 10 ? 15 : 14, 1);
      lcd.print(selected_item - 1);
    }
    previous_screen = selected_item;
  } // END if (selected_item != previous_screen)

  char key = kpd.getKey();
  if (kpd.getState() == PRESSED)
  {
    switch (key)
    {
    case 'A':
      selected_item--;
      playMelody(m_IncreasekeypadClick);
      if (selected_item <= 0)
      {
        previous_item = MAX_RFID - 1;
        selected_item = MAX_RFID;
        next_item = 0;
      }
      else
      {
        previous_item = selected_item - 1;
        next_item = selected_item + 1;
      }

      break;
    case 'B':
      selected_item++;
      playMelody(m_DecreasekeypadClick);
      if (selected_item > MAX_RFID)
      {
        previous_item = MAX_RFID;
        selected_item = 1;
        next_item = selected_item + 1;
      }
      else
      {
        previous_item = selected_item - 1;
        next_item = selected_item + 1;
      }

      break;
    case Enter:
      playMelody(m_FunckeypadClick);
      if (!deleteA_RFID(selected_item - 1))
      {
        Serial.println("Selected UID is blank");
        reset_setting_screen();
        return;
      }

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Card In Num ");
      lcd.print(selected_item - 1);
      lcd.setCursor(0, 1);
      lcd.print("is Deleted");
      delay(delay_screen_display);

      Serial.println("Selected UID is deleted");

      reset_setting_screen();
      programSubSETTINGState = SETT_HOME;
      break;
    case Back:
      playMelody(m_FunckeypadClick);
      reset_setting_screen();
      programSubSETTINGState = SETT_HOME;
      break;
    } // END switch (key)
  }
}

void prog_sett_delALL_rfid()
{
  if (current_screen != previous_screen)
  {
    Serial.println(DELALL_RFID_items[current_screen]);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(DELALL_RFID_items[current_screen]);

    previous_screen = current_screen;
    if (current_screen == 2)
    {
      current_screen = 1;
      delay(delay_screen_display);
    }
  } // END if (selected_item != previous_screen)

  if (memcmp(typePassword, prev_typePassword, MAX_PASSWORD) != 0)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(DELALL_RFID_items[current_screen]);
    lcd.setCursor(0, 1);
    lcd.print(typePassword);

    memset(prev_typePassword, '\0', MAX_PASSWORD);
    memcpy(prev_typePassword, typePassword, MAX_PASSWORD);
  }

  char key = kpd.getKey();
  switch (kpd.getState())
  {
  case PRESSED:
    switch (key)
    {
    case Back:
      playMelody(m_FunckeypadClick);
      if (memcmp(typePassword, blank, MAX_PASSWORD) == 0)
      {
        programSubSETTINGState = SETT_HOME;
        reset_setting_screen();
        return;
      }
      clearTypeArray();
      break;
    case Enter:
      playMelody(m_FunckeypadClick);
      switch (evaluateTypeArray())
      {
      case 1:
        sub_prog_sett_delALL_rfid();
        break;

      case 100:
        sub_prog_sett_delALL_rfid();
        break;

      default:
        Serial.println("INVALID");
        Serial.println("try again");
        current_screen = 2;
        break;
      }

      break;

    default:
      if (key != '\0')
      {
        playMelody(m_keypadClick);
        Serial.println(key);
        addChar2TypeArray(key);
      }
      break;
    }
    break;
  }

  if (!mfrc522.PICC_IsNewCardPresent())
    return;

  if (!mfrc522.PICC_ReadCardSerial())
    return;

  mfrc522.PICC_HaltA();

  if (mfrc522.uid.size > 4)
  {
    Serial.println("card not supported");
    return;
  }

  for (uint8_t i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(i == 0 ? (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "") : (mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    scannedCard[i] = mfrc522.uid.uidByte[i];
    if (i == mfrc522.uid.size - 1)
      Serial.println();
  }
  uint8_t card = checkCard();
  switch (card)
  {
  case 100:
    Serial.println("MASTER");
    sub_prog_sett_delALL_rfid();
    break;
  }
}

// -----------------------------------------------------------------------------------
// --------------------------------------- Fungtions ---------------------------------
// -----------------------------------------------------------------------------------

void process_SETT_CHG_PSWD_VER()
{
  char key = kpd.getKey();
  if (kpd.getState() == PRESSED)
  {
    switch (key)
    {
    case Back:
      playMelody(m_FunckeypadClick);
      clearTypeArray();

      break;
    case Enter:
      playMelody(m_FunckeypadClick);
      switch (evaluateVerifTypeArray())
      {
      case 1:
        Serial.println("VALID");
        Serial.println("passcode set");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(CHG_PSWD_items[4]);
        lcd.setCursor(0, 1);
        lcd.print(CHG_PSWD_items[5]);
        delay(delay_screen_display);
        changePSWDfromVerif();
        prog_sett_chg_pswdState = SETT_CHG_PSWD_ADD;
        programSubSETTINGState = SETT_HOME;
        reset_setting_screen();
        break;
      default:
        current_screen = 3;
        Serial.println("INVALID");
        Serial.println("try again");
        break;
      }

      break;

    default:
      if (key != '\0')
      {
        playMelody(m_keypadClick);
        Serial.println(key);
        addChar2TypeArray(key);
      }
      break;
    }
  }
}

void process_SETT_CHG_PSWD_ADD()
{
  char key = kpd.getKey();
  if (kpd.getState() == PRESSED)
  {
    switch (key)
    {
    case Back:
      playMelody(m_FunckeypadClick);
      if (memcmp(typePassword, blank, MAX_PASSWORD) == 0)
      {
        programSubSETTINGState = SETT_HOME;
        reset_setting_screen();
        return;
      }

      clearTypeArray();
      break;

    case Enter:
      playMelody(m_FunckeypadClick);
      if (memcmp(typePassword, blank, MAX_PASSWORD) == 0)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(CHG_PSWD_items[6]);
        delay(delay_screen_display);
        reset_setting_screen();
        return;
      }

      current_screen = 2;
      changeVariffromType();
      Serial.println("need verif passcode");
      prog_sett_chg_pswdState = SETT_CHG_PSWD_VER;
      break;

    default:
      if (key != '\0')
      {
        playMelody(m_keypadClick);
        Serial.println(key);
        addChar2TypeArray(key);
      }
      break;
    }
  }
}

static uint8_t (*_melody)[melodyLenght];
bool buzzStatus = false;
static bool _playMelody = false;
static bool _startNotecounting = false;
static bool _buzzAttach = false;
static uint32_t lastMelody_time = 0;
static uint16_t delayMelody_time = 0;
static uint8_t noteNumber = 0;

void playMelody(uint8_t (*arr1)[melodyLenght])
{
  buzzMelody_done();
  if (buzzStatus)
  {
    return;
  }

  _melody = arr1;
  buzzStatus = true;
  _playMelody = true;
}

void buzzMelody_runtime()
{
  if (_playMelody)
  {
    uint32_t i = millis();
    // Serial.println(i - lastMelody_time + String(" i - lastMelody_time"));
    if (i - lastMelody_time > delayMelody_time)
    {
      if (noteNumber > melodyLenght - 1 || _melody[2][noteNumber] == 0)
      {
        buzzMelody_done();
      }
      else
      {
        lastMelody_time = i;
        delayMelody_time = _melody[2][noteNumber] * 10;
        if (_melody[0][noteNumber] != 12)
        {
          buzz_writeNote(_melody[0][noteNumber], _melody[1][noteNumber]);
        }
        else
        {
          buzz_off();
        }
        noteNumber++;
      }
    }
  }
}

void buzzMelody_done()
{
  buzz_off();

  lastMelody_time = 0;
  noteNumber = 0;
  _melody = 0;
  delayMelody_time = 0;
  buzzStatus = false;
  _playMelody = false;
}

static uint8_t last_note = 20;
void buzz_writeNote(uint8_t note, uint8_t octave)
{
  if (octave > 8)
  {
    return;
  }
  const uint16_t noteFrequencyBase[12] = {
      //   C        C#       D        Eb       E        F       F#        G       G#        A       Bb        B
      4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902};

  uint32_t noteFreq = (uint32_t)noteFrequencyBase[note] / (uint32_t)(1 << (8 - octave));

  digitalWrite(Buzzer_PIN, HIGH);
  return;
  ledcSetup(Buzzer_Ch, noteFreq, 10);

  buzz_detach();
  buzz_attach();

  ledcWrite(Buzzer_Ch, 0x1FF);
  // last_note = note;
}

void buzz_off()
{
  digitalWrite(Buzzer_PIN, LOW);

  return;
  ledcSetup(Buzzer_Ch, 10000, 10);
  ledcWrite(Buzzer_Ch, 0);
  buzz_detach();
}

void buzz_attach()
{
  if (!_buzzAttach)
  {
    ledcAttachPin(Buzzer_PIN, Buzzer_Ch);
    _buzzAttach = true;
  }
}

void buzz_detach()
{
  if (_buzzAttach)
  {
    ledcDetachPin(Buzzer_PIN);
    _buzzAttach = false;
  }
}

static bool _doorStatus = false;
static bool _unlock = false;
static bool _lock = false;
static int8_t _isServoStatus = 0;
static int8_t l_isServoStatus = 0;
static uint16_t _lastDutyCycle = 0;
static bool servoChange = false;
static uint32_t last_servoChange = 0;
#define _Lock 1
#define _Unlock -1

void Unlock()
{
  _unlock = true;
}

void Lock()
{
  _lock = true;
}

void setDoorStatus(bool i)
{
  _doorStatus = i;
}

bool getDoorStatus()
{
  return bool(_doorStatus);
}

bool _doMelody = false;
void doorLock_handler()
{
  bool _unlockPB = !digitalRead(unlockPB_pin);
  _doorStatus = !digitalRead(doorSens_pin);

  if (millis() - last_servoChange > 2000 && servoChange)
  {
    uint16_t i = ledcRead(Servo_Ch) - _lastDutyCycle;
    Serial.println("ledcRead(Servo_Ch) - _lastDutyCycle = " + String(i));
    if (i >= -5 && i <= 5)
    {
      Serial.println("servoChange = false;");
      servoChange = false;
      _isServoStatus = l_isServoStatus;
    }
  }

  if (!servoChange && (_unlock || _lock || _unlockPB) && _doorStatus == Close)
  {

    if (_unlock && _isServoStatus == _Lock)
    {
      if (_doMelody)
      {
        playMelody(m_Open);
      }

      servoUnlock(servo_uS_UNLOCK);
      prev_change_home = millis();
      programSubHOMEState = HOME_OPEN;
      programSubHOME_OPEN_State = HOME_OPEN_NORM;
    }

    if (_lock && _isServoStatus == _Unlock)
    {
      if (_doMelody)
      {
        playMelody(m_Close);
      }

      servoLock(servo_uS_LOCK);
      prev_change_home = millis();
      programSubHOMEState = HOME_CLOSE;
      programSubHOME_OPEN_State = HOME_OPEN_NORM;
    }

    _unlock = false;
    _lock = false;
  }

  if (!servoChange && _unlockPB && _doorStatus == Close)
  {

    if (_isServoStatus == _Lock)
    {
      if (_doMelody)
      {
        playMelody(m_Open);
      }
      lcd.backlight();

      servoUnlock(servo_uS_UNLOCK);
      prev_change_home = millis();
      programSubHOMEState = HOME_OPEN;
      programSubHOME_OPEN_State = HOME_OPEN_NORM;
    }

    if (_isServoStatus == _Unlock)
    {
      if (_doMelody)
      {
        playMelody(m_Close);
      }
      lcd.backlight();

      servoLock(servo_uS_LOCK);
      prev_change_home = millis();
      programSubHOMEState = HOME_CLOSE;
      programSubHOME_OPEN_State = HOME_OPEN_NORM;
    }
  }
}

void servoUnlock(uint16_t i)
{
  ServoWrite_uS(i, Servo_Freq, Servo_Res);
  Serial.println("servoUnlock in (uS):" + String(i));
  last_servoChange = millis();
  l_isServoStatus = _Unlock;
  servoChange = true;
  _doMelody = true;
}
void servoLock(uint16_t i)
{
  ServoWrite_uS(i, Servo_Freq, Servo_Res);
  Serial.println("servoLock in (uS):" + String(i));
  last_servoChange = millis();
  l_isServoStatus = _Lock;
  servoChange = true;
  _doMelody = true;
}

void ServoWrite_uS(uint16_t uS, uint16_t freq, uint8_t res)
{
  static uint16_t prev_Freq = 0;
  static uint16_t prev_uS = 0;
  static uint16_t prev_Res = 0;
  static uint32_t res_width = 0;
  static double aPeriodeWidth = 0;
  static uint16_t dutyCycle = 0;
  static bool attach = false;

  aPeriodeWidth = hzToMicroseconds(freq);
  res_width = pow(2, res);

  Serial.println("aPeriodeWidth = " + String(aPeriodeWidth));
  Serial.println("res_width = " + String(res_width));

  Serial.println("ledcSetup = " + String(ledcSetup(Servo_Ch, freq, res)));
  // ledcSetup(Servo_Ch, freq, res);

  ledcDetachPin(Servo_PIN);
  ledcAttachPin(Servo_PIN, Servo_Ch);

  dutyCycle = uS / (aPeriodeWidth / res_width);
  Serial.println("dutyCylce = " + String(dutyCycle));
  if (prev_uS != uS)
  {
    _lastDutyCycle = dutyCycle;
    Serial.println("ledcWrite uS = " + String(uS));
    ledcWrite(Servo_Ch, dutyCycle);
    prev_uS = uS;
  }
}

double hzToMicroseconds(double frequencyHz)
{
  return double(1.0 / frequencyHz * 1000000.0);
}

void EEprom_Update()
{
  if (memcmp(prev_storedCard, storedCard, storedCardSize) != 0)
  {
    storedCard_eeprom.put(0, storedCard);
    storedCard_eeprom.commit();
    memcpy(prev_storedCard, storedCard, storedCardSize);
    Serial.println("storedCard Saved in EEPROM");
  }

  if (memcmp(prev_storedPassword, storedPassword, MAX_PASSWORD) != 0)
  {
    storedPassword_eeprom.put(0, storedPassword);
    storedPassword_eeprom.commit();
    memcpy(prev_storedPassword, storedPassword, MAX_PASSWORD);
    Serial.println("storedPassword Saved in EEPROM");
  }

  if (prev_num_storedCard != num_storedCard)
  {
    num_storedCard_eeprom.put(0, num_storedCard);
    num_storedCard_eeprom.commit();
    prev_num_storedCard = num_storedCard;
    Serial.println("num_storedCard Saved in EEPROM");
  }

  if (last_idle_time != idle_time)
  {
    idle_time_eeprom.put(0, idle_time);
    idle_time_eeprom.commit();
    last_idle_time = idle_time;
    Serial.println("idle_time Saved in EEPROM");
  }
}

void reset_typePasscode()
{
  memset(prev_typePassword, '\0', MAX_PASSWORD);
  memcpy(prev_typePassword, typePassword, MAX_PASSWORD);
}

void sub_prog_sett_delALL_rfid()
{
  Serial.println("VALID");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(DELALL_RFID_items[3]);
  delay(delay_screen_display);
  Serial.println(DELALL_RFID_items[3]);
  deleteAllRFID();
  programSubSETTINGState = SETT_HOME;
  reset_setting_screen();
}

void changeVariffromType()
{
  memset(verif_typePassword, '\0', MAX_PASSWORD);
  memcpy(verif_typePassword, typePassword, MAX_PASSWORD);
  clearTypeArray();
}

void changePSWDfromVerif()
{
  memset(storedPassword, '\0', MAX_PASSWORD);
  memcpy(storedPassword, verif_typePassword, MAX_PASSWORD);
  clearVerif_typePassword();
}

void deleteAllRFID()
{
  memset(storedCard, 0, storedCardSize);
  num_storedCard = 0;
}

bool deleteA_RFID(uint8_t i)
{
  const uint8_t r[MAX_RFID_BYTE] = {};
  if (memcmp(storedCard[i], r, MAX_RFID_BYTE) == 0)
  {
    return false;
  }
  memset(storedCard[i], 0, sizeof(storedCard[i]));
  num_storedCard--;
  return true;
}

void reset_setting_screen()
{
  lcd.clear();
  current_screen = 1;
  previous_screen = 2;
  selected_item = 1;
  previous_item = num_setting_screen - 1;
  next_item = selected_item + 1;
}

uint8_t checkCard()
{
  if (memcmp(scannedCard, MasterCard, MAX_RFID_BYTE) == 0)
  {
    clearScaned_card();
    return 100;
  }

  for (uint8_t i = 0; i < MAX_RFID; i++)
  {
    if (memcmp(scannedCard, storedCard[i], MAX_RFID_BYTE) == 0)
    {
      clearScaned_card();
      return i;
    }
  }
  clearScaned_card();
  return 200;
}

uint8_t addScannedCard2StoredCard()
{
  if (memcmp(scannedCard, MasterCard, MAX_RFID_BYTE) == 0)
  {
    clearScaned_card();
    Serial.println("This is matercard, cant add");
    return 100;
  }

  for (uint8_t i = 0; i < MAX_RFID; i++)
  {
    if (memcmp(scannedCard, storedCard[i], MAX_RFID_BYTE) == 0)
    {
      Serial.print("Card Already Seved In Number ");
      Serial.println(i);
      clearScaned_card();
      return 201 + i;
      break;
    }
  }

  for (uint8_t i = 0; i < MAX_RFID; i++)
  {
    if (memcmp(storedCard[i], r, MAX_RFID_BYTE) == 0)
    {
      memcpy(storedCard[i], scannedCard, MAX_RFID_BYTE);
      Serial.print("Card Added in number ");
      Serial.println(i);
      num_storedCard++;
      Serial.print("Number Stored Card is ");
      Serial.println(num_storedCard);
      clearScaned_card();
      return i;
      break;
    }
  }

  Serial.print("Card Sorage Is FULL at ");
  Serial.println(num_storedCard);
  return 200;
}

void clearScaned_card()
{
  memset(scannedCard, 0, MAX_RFID_BYTE);
}

void addChar2TypeArray(char key)
{
  if (typeIndex == MAX_PASSWORD - 1)
    return;
  typePassword[typeIndex] = key;
  typeIndex++;
}

void clearTypeArray()
{
  typeIndex = 0;
  Serial.println("clearTypeArray DEBUG");
  Serial.print("BEFORE = '");
  Serial.print(typePassword);
  Serial.println("'");
  memset(typePassword, '\0', MAX_PASSWORD);
  Serial.print("AFTER  = '");
  Serial.print(typePassword);
  Serial.println("'");
}

void clearVerif_typePassword()
{
  Serial.println("verif_typePassword DEBUG");
  Serial.print("BEFORE = '");
  Serial.print(verif_typePassword);
  Serial.println("'");
  memset(verif_typePassword, '\0', MAX_PASSWORD);
  Serial.print("AFTER  = '");
  Serial.print(verif_typePassword);
  Serial.println("'");
}

uint8_t evaluateTypeArray()
{
  int8_t check = strcmp(typePassword, storedPassword);
  int8_t master = strcmp(typePassword, masterPassword);
  prev_change_home = millis();
  Serial.print("evaluateTypeArray DEBUG = '");
  Serial.print(typePassword);
  Serial.println("'");
  clearTypeArray();
  return master == 0 ? 100 : (check == 0 ? 1 : 0);
}

uint8_t evaluateVerifTypeArray()
{
  int8_t check = strcmp(typePassword, verif_typePassword);
  Serial.print("verif_typePassword DEBUG = '");
  Serial.print(typePassword);
  Serial.println("'");
  clearTypeArray();
  return check == 0 ? 1 : 0;
}

String uint32binToString(uint8_t lenght, uint32_t aUint32)
{
  String i = "";
  for (int8_t aBit = lenght * 8 - 1; aBit >= 0; aBit--)
  {
    i += (((aUint32 >> aBit) & 1) ? "1" : "0");
  }
  return i;
}
