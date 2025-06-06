#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include <time.h>

// RFID Module setup
#define RST_PIN 0  // RFID Reset pin
#define SS_PIN 5   // RFID SPI SS pin
#define mySerial Serial2 // use UART2 for AS608
#define RELAY_PIN 15 //Relay IN pin
MFRC522 rfid(SS_PIN, RST_PIN);

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);  

// Keypad Setup
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = 
{
    { '1', '2', '3' },
    { '4', '5', '6' },
    { '7', '8', '9' },
    { '*', '0', '#' }
};
byte rowPins[ROWS] = { 13, 12, 14, 27 };
byte colPins[COLS] = { 26, 25, 33 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Fingerprint Sensor Setup
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Authorized Credentials
const String authorizedRFID1 = "132C92D";  //UID1
const String correctPassword = "0605";    
String enteredPassword = "";

// Function Prototypes
bool verifyFingerprint(); 
void unlockAccess();

void setup() 
{
    Serial.begin(500000);
    mySerial.begin(57600, SERIAL_8N1, 16, 17);
    SPI.begin();
    rfid.PCD_Init();
    lcd.init();
    lcd.backlight();
    
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);  // Keep lock closed
    
    finger.begin(57600);
    if (finger.verifyPassword()) 
    {
        Serial.println("Fingerprint sensor found!");
    } 
    else 
    {
        Serial.println("Fingerprint sensor not detected.");
    }
    
    Serial.println("System Ready");
    lcd.setCursor(0, 0);
    lcd.print("System Ready");     
}

void loop() 
{
    // Check RFID
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) 
    {
        String uid = "";
        for (byte i = 0; i < rfid.uid.size; i++) 
        {
            uid += String(rfid.uid.uidByte[i], HEX);
        }
        uid.toUpperCase();
        Serial.println("RFID UID detected: " + uid);
        
        uid.trim();
        if (uid == authorizedRFID1 ) 
        {
            unlockAccess();
        } 
        else 
        {
            displayMessage("Unauthorized Access");
            Serial.println("Unauthorized RFID card");
            delay(700);
            displayMessage("System Ready");
        }
        rfid.PICC_HaltA();
    }

    // Check Keypad Input
    char key = keypad.getKey();
    if (key) 
    {
        if (key == '#') 
        { 
            if (enteredPassword == correctPassword) 
            {
                unlockAccess();
            } 
            else 
            {
                displayMessage("Incorrect Code");
                Serial.println("Incorrect password");
                delay(200);
                displayMessage("System Ready");           
            }
            enteredPassword = "";
        } 
        else if (key == '*') 
        { 
            enteredPassword = "";
            Serial.println("Password reset");
            displayMessage("Reset Code");
            delay(300);
            displayMessage("System Ready");
        } 
        else
            {
            enteredPassword += key;
            Serial.print("*");
            lcd.setCursor(0, 1);
            lcd.print(enteredPassword);
            }
    }

    // Check Fingerprint Authentication
    if (verifyFingerprint()==true) 
    {
        unlockAccess();
    }
    if (verifyFingerprint()==false) 
    {
        displayMessage("System Ready");
    }
}

// Function to Verify Fingerprint
bool verifyFingerprint() 
{
    int p = finger.getImage();
    if (p != FINGERPRINT_OK) return false;

    p = finger.image2Tz();
    if (p != FINGERPRINT_OK) return false;

    p = finger.fingerSearch();
    if (p == FINGERPRINT_OK) 
    {
        Serial.println("Fingerprint recognised");
        displayMessage("Fingerprint Passed ;)");
        return true;
    } 
    else
    {
        Serial.println("Fingerprint not recognized");
        displayMessage("Fingerprint Denied =))");
        return false;
        delay(400);
    }  
}


// Function to Unlock Access
void unlockAccess() 
{
    Serial.println("Access Granted!");
    displayMessage("Access Granted!");
    digitalWrite(RELAY_PIN, HIGH);
    delay(5000);
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Lock Relocked");
    displayMessage("System Ready");
}

// Function to Display LCD Messages
void displayMessage(String message) 
{
    lcd.clear();
    lcd.setCursor(0, 0);
    if (message.length() <= 16) 
    {
        lcd.print(message); // If the message fits, just display it
    } 
    else 
    {
        for (int i = 0; i <= message.length() - 16; i++) 
        {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(message.substring(i, i + 20)); // Display scrolling window
            delay(400); // Adjust delay for speed control
        }
    }
    delay(100);
}