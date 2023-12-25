#include <Adafruit_NeoPixel.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <string>

#define PIN 15
#define NUMPIXELS 10
#define CARDKB_ADDR \
    0x5F



WiFiClient client;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB);
int current = 0;
int period = 400;
int kb_char = 0;
bool PIR_trigger = false;
bool SUCCESS = false;
int numAttemps = 0;
String lastAnswear = "";
String kbInput = "";
const String LOGIN_REQUEST = "LOGIN_REQUEST\a\b";



/* Put your SSID & Password */
const char* ssid = "my_wifi";  // Enter SSID here
const char* password = "pivkopivko";  //Enter Password here

const char* serverIP = "192.168.126.175";
const int serverPort = 8080;




void setup() {
    M5.begin();
    Serial.begin(115200);

    delay(1000);

    Serial.println("Connecting to WiFi");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting...");
    }
    Serial.println("Connected to WiFi");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    M5.lcd.setTextSize(2);
    M5.Lcd.print("WiFi connected, ");
    M5.Lcd.println(WiFi.localIP());
    delay(1000);
    strip.fill(BLACK, 0, NUMPIXELS);
    strip.show();

    M5.Power.begin();
    M5.Speaker.begin();
    pinMode(36, INPUT);
}


String readData()
{
    String response;
    delay(150);
    while(response.isEmpty()) {
        response = client.readStringUntil('\a');
        Serial.print("Response: ");
        Serial.println(response);
    }

    return response;
}

String readFromKB()
{
    kbInput = "";
    kb_char = 0;
    while(kb_char != 13) {
        Wire.requestFrom(CARDKB_ADDR, 1);
        while (Wire.available()) {
            kb_char = Wire.read();
            if (kb_char != -1 && kb_char != 0) {
                Serial.print("Got char: ");
                Serial.println(kb_char);
                if (kb_char == 8) {
                    if (kbInput.length() != 0) {
                        kbInput.remove(kbInput.length() - 1);
                        M5.Lcd.clear();
                        M5.Lcd.setCursor(0,0);
                        M5.Lcd.println("Enter password:");
                        M5.Lcd.print(kbInput);
                    }
                } else if (kb_char != 13) {
                    kbInput += (char) kb_char;
                    M5.Lcd.printf("%c", kb_char);
                } else break;
                // ENTER is 13 BACKSPACE is 8, rest is usual ASCII
            }
        }
    }
    kbInput += "\a\b";
    return kbInput;
}

void serverLoop()
{
    lastAnswear = "";
    M5.Lcd.clear();
    M5.Lcd.setCursor(0,0);
    M5.Lcd.print("Connecting.");
    while (!client.connected()) {
        Serial.println("Connecting to server...");
        M5.Lcd.print(".");
        if (client.connect(serverIP, serverPort)) {
            Serial.println("Connected to server");
            M5.Lcd.println();
            M5.Lcd.println("Connected to server!");
            break;
        } else {
            Serial.println("Connection failed");
        }
        delay(5000); // Wait before trying to reconnect
    }

    client.print("LOGIN_REQUEST");
    M5.Lcd.println("SENT LOGIN_REQUEST");
    lastAnswear = readData();

    if(lastAnswear == "LOGIN_FORBIDDEN")
    {
        Serial.println("LOGIN FORBIDDEN, ATTEMPT FAIL");
        strip.fill(PURPLE, 0, NUMPIXELS);
        strip.show();
        //M5.Speaker.beep();
        delay(200);
        client.stop();
        numAttemps++;
        return;
    }
    else if (lastAnswear == "BAD_REQUEST")
    {
        Serial.println("BAD REQUEST, debug");
        strip.fill(BLUE, 0, NUMPIXELS);
        strip.show();
        M5.Speaker.beep();
        client.stop();
        numAttemps++;
        return;
    }
    else if (lastAnswear == "LOGIN_CONTINUE")
    {
        M5.Lcd.clearDisplay();
        M5.Lcd.setCursor(0,0);
        M5.Lcd.println("Enter password:");

        readFromKB();
        M5.Lcd.println();
        M5.Lcd.print("Sending: ");
        M5.Lcd.println(kbInput);

        client.print(kbInput);
        lastAnswear = readData();

        if(lastAnswear == "LOGIN_SUCCESS")
        {
            Serial.println("SUCCESS");
            M5.Lcd.println("SUCCESS");
            SUCCESS = true;
            strip.fill(RED, 0, NUMPIXELS);
            strip.show();
            client.stop();
            return;
        }
        else if (lastAnswear == "LOGIN_FAIL")
        {
            Serial.println("FAIL");
            M5.Lcd.println("FAIL");
            strip.fill(BLUE, 0, NUMPIXELS);
            strip.show();
            //M5.Speaker.beep();
            delay(100);
            client.stop();
            numAttemps++;
            return;
        }

    }


}



void loop() {
    M5.update();
    if(M5.BtnC.wasPressed())
    {
        numAttemps = 0;
        SUCCESS = false;
        PIR_trigger = false;
        M5.Lcd.clear();
        strip.fill(BLACK, 0, NUMPIXELS);
        strip.show();
    }


    if(digitalRead(36) == 1 && !PIR_trigger && !SUCCESS)
    {
        PIR_trigger = true;
        numAttemps = 0;
        Serial.println("PIR SENSING");
        M5.Lcd.println("Detected PIR");

        while(numAttemps != 3 && !SUCCESS)
        {
            serverLoop();
        }

        M5.Lcd.clear();
        M5.Lcd.setCursor(0,0);
        M5.Lcd.setTextSize(2);
        if(SUCCESS)
        {
            M5.Lcd.println("LOGIN SUCCESSFUL :3333");
            PIR_trigger = false;
        } else
        {
            M5.Lcd.println("3 ATTEMPTS FAILED");
        }

    }



}