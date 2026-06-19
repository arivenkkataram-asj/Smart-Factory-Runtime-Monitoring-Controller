#include <WiFi.h>
#include <HardwareSerial.h>

#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID       "Theghost"
#define WIFI_PASSWORD   "45674567"

#define API_KEY         "API Key from Firebase"

#define DATABASE_URL    "https://solar-pump-monitoring-control-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

HardwareSerial STMSerial(2);

// Received data

float solarVoltage = 0;
float batteryVoltage = 0;
float pumpCurrent = 0;
float tankLevel = 0;

int pumpFeedback = 0;
int dryRun = 0;
int pumpStatus = 0;
int chargingStatus = 0;

bool signupOK = false;

// ESP32 Outputs

#define PUMP_LED      2
#define CHARGE_LED    4
#define ALARM_LED     5
#define BUZZER_LED    18

void parseData(String data)
{
    int result = sscanf(
        data.c_str(),
        "SV=%f,BV=%f,PC=%f,TL=%f,PF=%d,DR=%d,PS=%d,CS=%d",
        &solarVoltage,
        &batteryVoltage,
        &pumpCurrent,
        &tankLevel,
        &pumpFeedback,
        &dryRun,
        &pumpStatus,
        &chargingStatus
    );

    if(result == 8)
    {
        Serial.println("STM32 Data Received");

        digitalWrite(PUMP_LED,pumpStatus);
        digitalWrite(CHARGE_LED,chargingStatus);

        if(dryRun)
        {
            digitalWrite(ALARM_LED,HIGH);
            digitalWrite(BUZZER_LED,HIGH);
        }
        else
        {
            digitalWrite(ALARM_LED,LOW);
            digitalWrite(BUZZER_LED,LOW);
        }
    }
}

void uploadFirebase()
{
    Firebase.RTDB.setFloat(
        &fbdo,
        "/solarVoltage",
        solarVoltage);

    Firebase.RTDB.setFloat(
        &fbdo,
        "/batteryVoltage",
        batteryVoltage);

    Firebase.RTDB.setFloat(
        &fbdo,
        "/pumpCurrent",
        pumpCurrent);

    Firebase.RTDB.setFloat(
        &fbdo,
        "/tankLevel",
        tankLevel);

    Firebase.RTDB.setInt(
        &fbdo,
        "/pumpFeedback",
        pumpFeedback);

    Firebase.RTDB.setInt(
        &fbdo,
        "/dryRunStatus",
        dryRun);

    Firebase.RTDB.setInt(
        &fbdo,
        "/pumpStatus",
        pumpStatus);

    Firebase.RTDB.setInt(
        &fbdo,
        "/chargingStatus",
        chargingStatus);

    Serial.println("Firebase Updated");
}

void setup()
{
    Serial.begin(115200);

    STMSerial.begin(
        115200,
        SERIAL_8N1,
        16,
        17);

    pinMode(PUMP_LED,OUTPUT);
    pinMode(CHARGE_LED,OUTPUT);
    pinMode(ALARM_LED,OUTPUT);
    pinMode(BUZZER_LED,OUTPUT);

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD);

    Serial.print("Connecting WiFi");

    while(WiFi.status()!=WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    Serial.println();
    Serial.println("WiFi Connected");

    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    if(Firebase.signUp(
        &config,
        &auth,
        "",
        ""))
    {
        signupOK = true;
        Serial.println("Firebase SignUp OK");
    }
    else
    {
        Serial.printf(
            "%s\n",
            config.signer.signupError.message.c_str());
    }

    Firebase.begin(
        &config,
        &auth);

    Firebase.reconnectWiFi(true);
}

unsigned long lastUpload = 0;

void loop()
{
    if(STMSerial.available())
    {
        String data =
            STMSerial.readStringUntil('\n');

        data.trim();

        parseData(data);
    }

    if(Firebase.ready()
       && signupOK
       && millis()-lastUpload > 3000)
    {
        lastUpload = millis();

        uploadFirebase();
    }
}