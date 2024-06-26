#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
float latitude, longitude;
SoftwareSerial gpsSerial(8, 9); // TX, RX
SoftwareSerial gsmSerial(2, 3); // TX, RX

const int soundSensorPin = A0;     // Connect the sound sensor's analog output to A0
const int heartbeatSensorPin = A1; // Connect the heartbeat sensor's analog output to A1

// Adjust these threshold values according to your sensor sensitivities
const int soundThreshold = 500;
const int heartbeatThreshold = 600;

const int readingsToAverage = 10; // Number of readings to average for panic situation

TinyGPSPlus gps;

void setup()
{
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    gpsSerial.begin(9600);
    gpsSerial.listen();
    textgps();

    gsmSerial.begin(9600);
    gsmSerial.print("\r");
    delay(1000);
    gsmSerial.print("AT+CMGF=1\r");
    delay(1000);
}

void loop()
{
    int soundTotal = 0;
    int heartbeatTotal = 0;

    for (int i = 0; i < readingsToAverage; i++)
    {
        soundTotal += analogRead(soundSensorPin);
        heartbeatTotal += analogRead(heartbeatSensorPin);
        delay(10);
    }

    int averageSoundValue = soundTotal / readingsToAverage;
    int averageHeartbeatValue = heartbeatTotal / readingsToAverage;

    if (averageSoundValue >= soundThreshold || averageHeartbeatValue >= heartbeatThreshold)
    {
        sendMessage();
    }
}

void textgps()
{
    while (1)
    {
        while (gpsSerial.available() > 0)
        {
            gps.encode(gpsSerial.read());
        }

        if (gps.location.isUpdated())
        {
            Serial.print("LAT=");
            Serial.println(gps.location.lat(), 6);
            Serial.print("LONG=");
            Serial.println(gps.location.lng(), 6);
            latitude = gps.location.lat();
            longitude = gps.location.lng();
            break;
        }
    }

    Serial.print("LATITUDE=");
    Serial.println(latitude, 6);
    Serial.print("LONGITUDE=");
    Serial.println(longitude, 6);
    lcd.print("LAT ");
    lcd.print(latitude, 6);
    lcd.setCursor(0, 1);
    lcd.print("LONG ");
    lcd.print(longitude, 6);
    delay(1000);
}

void sendMessage()
{
    gsmSerial.print("AT+CMGS=\"+917702143694\"\r"); // Replace with recipient's phone number
    delay(1000);

    String googleMapsLink = "https://www.google.com/maps/place/" + String(latitude, 6) + "," + String(longitude, 6);

    gsmSerial.print("Check this location: ");
    gsmSerial.println(googleMapsLink);
    delay(1000);
    gsmSerial.write(0x1A);
    delay(1000);
}
