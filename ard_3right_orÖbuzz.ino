#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9 // Configurable, see typical pin layout above
#define SS_PIN 10 // Configurable, see typical pin layout above

#define BUTTON_PIN 4
#define GreenPin 2
#define RedPin 3

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

int mode = 0; // 0 read , 1 write right , 2 wright wrong,

byte rightText[5] = {0x72, 0x69, 0x67, 0x68, 0x74}; // right

byte wrongText[5] = {0x77, 0x72, 0x6f, 0x6e, 0x67}; // wrong

int count = 0;
unsigned long uid_one;
unsigned long uid_two;
unsigned long uid_three;

void setup()
{
    pinMode(GreenPin, OUTPUT);
    pinMode(RedPin, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);

    Serial.begin(9600);                                        // Initialize serial communications with the PC
    SPI.begin();                                               // Init SPI bus
    mfrc522.PCD_Init();                                        // Init MFRC522 card
    Serial.println(F("Read personal data on a MIFARE PICC:")); // shows in serial that it is ready to read
}

void loop()
{

    if (digitalRead(BUTTON_PIN) == HIGH)
    {
        mode++;

        if (mode > 2)
        {
            mode = 0;
        }

        int pinNumber;

        if (mode == 0)
        {
            digitalWrite(GreenPin, HIGH);
            digitalWrite(RedPin, HIGH);
            delay(1000);
            digitalWrite(GreenPin, LOW);
            digitalWrite(RedPin, LOW);
        }
        else
        {
            if (mode == 1)
            {
                pinNumber = GreenPin;
            }
            else
            {
                pinNumber = RedPin;
            }

            digitalWrite(pinNumber, HIGH);
            delay(1000);
            digitalWrite(pinNumber, LOW);
        }
    }

    // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++)
        key.keyByte[i] = 0xFF;

    // some variables we need
    byte block;
    byte len;
    MFRC522::StatusCode status;

    if (!mfrc522.PICC_IsNewCardPresent())
    { // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
        return;
    }

    if (!mfrc522.PICC_ReadCardSerial())
    { // Select one of the cards
        return;
    }

    unsigned long uid = getID();

    Serial.println(F("**Card Detected:**"));

    // Read Mode
    if (mode == 0)
    {

        Serial.println(F("Start Prepare"));
        byte readbuffer[18];

        block = 1;
        len = 18;

        bool answered_right = true;

        status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)); // line 834 of MFRC522.cpp file
        if (status != MFRC522::STATUS_OK)
        {
            Serial.print(F("Authentication failed: "));
            Serial.println(mfrc522.GetStatusCodeName(status));
            return;
        }

        Serial.println(F("Start Read"));
        status = mfrc522.MIFARE_Read(block, readbuffer, &len);
        if (status != MFRC522::STATUS_OK)
        {
            Serial.print(F("Reading failed: "));
            Serial.println(mfrc522.GetStatusCodeName(status));
            return;
        }

        Serial.println(F("Start Check Answer"));
        for (uint8_t i = 0; i < 5; i++)
        {
            if (readbuffer[i] != rightText[i])
            {
                answered_right = false;
                break;
            }
        }

        Serial.println(F("Start What Answer"));
        if (answered_right)
        {

            if (uid != 0)
            {

                Serial.print("Card detected, UID: ");
                Serial.println(uid);

                Serial.println(F("check old ids"));
                if (uid != uid_one && uid != uid_two && uid != uid_three)
                {
                    count++;
                    if (count == 3)
                    {
                        // you win
                        uid_three = uid;
                        Serial.println(F("you win"));
                        game_reset();
                    }
                    if (count == 2)
                    {
                        // one more
                        uid_two = uid;
                        Serial.println(F("one more"));
                    }
                    if (count == 1)
                    {
                        // two more
                        uid_one = uid;
                        Serial.println(F("two more"));
                    }
                }
            }

            Serial.println(F("Right Answer"));

            digitalWrite(GreenPin, HIGH);
            delay(2000);
            digitalWrite(GreenPin, LOW);
        }
        else
        {
            Serial.println(F("wrong Answer"));
            game_reset();
            digitalWrite(RedPin, HIGH);
            delay(2000);
            digitalWrite(RedPin, LOW);
        }
        delay(2000);
    }
    else
    {
        // Dono
        MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak); // Dump PICC type
        Serial.println(mfrc522.PICC_GetTypeName(piccType));

        Serial.println(F("Start Prepare"));
        byte buffer[34];

        for (byte i = 0; i < 30; i++)
        {
            buffer[i] = ' '; // pad with spaces
        }
        for (byte i = 0; i < 5; i++)
        {
            if (mode == 1)
            {
                buffer[i] = rightText[i];
            }
            if (mode == 2)
            {
                buffer[i] = wrongText[i];
            }
        }

        Serial.println(F("Start Block 1"));
        block = 1;

        Serial.println(F("Start Authenticate"));
        // Serial.println(F("Authenticating using key A..."));
        status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
        if (status != MFRC522::STATUS_OK)
        {
            Serial.print(F("PCD_Authenticate() failed: "));
            Serial.println(mfrc522.GetStatusCodeName(status));
            return;
        }

        // Write block
        Serial.println(F("Start Write"));
        status = mfrc522.MIFARE_Write(block, buffer, 16);
        if (status != MFRC522::STATUS_OK)
        {
            Serial.print(F("MIFARE_Write() failed: "));
            Serial.println(mfrc522.GetStatusCodeName(status));
            return;
        }

        Serial.println(F("Start Block 2"));
        block = 2;
        // Serial.println(F("Authenticating using key A..."));
        Serial.println(F("Start Authenticate"));
        status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
        if (status != MFRC522::STATUS_OK)
        {
            Serial.print(F("PCD_Authenticate() failed: "));
            Serial.println(mfrc522.GetStatusCodeName(status));
            return;
        }

        Serial.println(F("Start Write"));
        // Write block
        status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
        if (status != MFRC522::STATUS_OK)
        {
            Serial.print(F("MIFARE_Write() failed: "));
            Serial.println(mfrc522.GetStatusCodeName(status));
            return;
        }

        digitalWrite(GreenPin, HIGH);
        digitalWrite(RedPin, HIGH);
        delay(2000);
        digitalWrite(GreenPin, LOW);
        digitalWrite(RedPin, LOW);
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}

void game_reset()
{
    Serial.println(F("Game Reset"));
    count = 0;
    uid_one = 0;
    uid_two = 0;
    uid_three = 0;
}

unsigned long getID()
{
    unsigned long hex_num;
    hex_num = mfrc522.uid.uidByte[0] << 24;
    hex_num += mfrc522.uid.uidByte[1] << 16;
    hex_num += mfrc522.uid.uidByte[2] << 8;
    hex_num += mfrc522.uid.uidByte[3];
    return hex_num;
}

//*****************************************************************************************//
