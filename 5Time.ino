    #include <ESP8266WiFi.h>
    #include <WiFiClient.h>
    #include <ESPAsyncTCP.h>
    //#include <ESP8266WebServer.h>
    #include <ESPAsyncWebServer.h>
    #include <ArduinoJson.h>
    #include "FS.h"
    #include <RTClib.h>
    #include <Wire.h>
    #include <NTPClient.h>
    #include <WiFiUdp.h>
    #include "SoftwareSerial.h"
    #include "DFRobotDFPlayerMini.h"

    // Deklarasi DF Player
    SoftwareSerial mySoftwareSerial(D3, D4); // RX, TX
    DFRobotDFPlayerMini myDFPlayer;

    // Deklarasi GPIO
    const int ledPin = 14;
    const int resetPin = 12;

    // Variable Millis 
    unsigned long previousUpdate = 0;
    unsigned long previousJam = 0;
    unsigned long previousLed = 0;

    // Led Blink State
    bool ledState = false; 
    
    // AP-Mode Variable
    const char* SSID_AP = "ESP-ALARM CONFIG";
    const char* PASSWORD_AP = "88888888";

    AsyncWebServer server(80);

    // Deklarasi RTC dan NTP
    RTC_DS3231 rtc;
    WiFiUDP ntpUDP;

    // =============== NTP =============== // 
    // GMT+1 3600
    // GMT+7 25200
    NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 25200, 60000);
    int NTP_JAM;
    int NTP_MENIT;
    int NTP_DETIK;
    int RTC_JAM = 0;
    int RTC_MENIT = 0;
    int RTC_DETIK = 0;

    // =============== Parameter Input =============== // 
    const char* PARAM_SSID = "ssid";
    const char* PARAM_PSWD = "pswd";
    const char* PARAM_JAM_1 = "jam1";
    const char* PARAM_JAM_2 = "jam2";
    const char* PARAM_JAM_3 = "jam3";
    const char* PARAM_JAM_4 = "jam4";
    const char* PARAM_JAM_5 = "jam5";
    const char* PARAM_MENIT_1 = "menit1";
    const char* PARAM_MENIT_2 = "menit2";
    const char* PARAM_MENIT_3 = "menit3";
    const char* PARAM_MENIT_4 = "menit4";
    const char* PARAM_MENIT_5 = "menit5";
    const char* PARAM_RING_1 = "ring1";
    const char* PARAM_RING_2 = "ring2";
    const char* PARAM_RING_3 = "ring3";
    const char* PARAM_RING_4 = "ring4";
    const char* PARAM_RING_5 = "ring5";

    // =============== Input Value =============== //

    // Variable config file
    String VALUE_SSID;
    String VALUE_PSWD;
    String VALUE_JAM_1;
    String VALUE_JAM_2;
    String VALUE_JAM_3;
    String VALUE_JAM_4;
    String VALUE_JAM_5;
    String VALUE_MENIT_1;
    String VALUE_MENIT_2;
    String VALUE_MENIT_3;
    String VALUE_MENIT_4;
    String VALUE_MENIT_5;
    String VALUE_RING_1;
    String VALUE_RING_2;
    String VALUE_RING_3;
    String VALUE_RING_4;
    String VALUE_RING_5;

    const char* VALUE_SSID_CHAR;
    const char* VALUE_PSWD_CHAR;

    // Variable config file (Integer)
    int INT_JAM_1;
    int INT_JAM_2;
    int INT_JAM_3;
    int INT_JAM_4;
    int INT_JAM_5;
    int INT_MENIT_1;
    int INT_MENIT_2;
    int INT_MENIT_3;
    int INT_MENIT_4;
    int INT_MENIT_5;
    int INT_RING_1;
    int INT_RING_2;
    int INT_RING_3;
    int INT_RING_4;
    int INT_RING_5;


    // =============== LOAD & PARSE CONFIG FILE =============== //
    bool loadConfig() {
        File configFile = SPIFFS.open("/config.json", "r");

        if(!configFile) {
            Serial.println("Failed to open config file");
        }

        size_t size = configFile.size();
        if(size >1024) {
            Serial.println("Config file is too large");
            return false;
        }

        // Parsing config File
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);

        StaticJsonDocument<700> doc;

        auto error = deserializeJson(doc, buf.get());
        if(error) {
            Serial.println("Failed to parse config File");
            return false;
        }

        VALUE_SSID = String(doc["ssid"]);
        VALUE_PSWD = String(doc["pswd"]);
        VALUE_JAM_1 = String(doc["j1"]);
        VALUE_JAM_2 = String(doc["j2"]);
        VALUE_JAM_3 = String(doc["j3"]);
        VALUE_JAM_4 = String(doc["j4"]);
        VALUE_JAM_5 = String(doc["j5"]);
        VALUE_MENIT_1 = String(doc["m1"]);
        VALUE_MENIT_2 = String(doc["m2"]);
        VALUE_MENIT_3 = String(doc["m3"]);
        VALUE_MENIT_4 = String(doc["m4"]);
        VALUE_MENIT_5 = String(doc["m5"]);
        VALUE_RING_1 = String(doc["tone1"]);
        VALUE_RING_2 = String(doc["tone2"]);
        VALUE_RING_3 = String(doc["tone3"]);
        VALUE_RING_4 = String(doc["tone4"]);
        VALUE_RING_5 = String(doc["tone5"]);

        VALUE_SSID_CHAR = doc["ssid"];
        VALUE_PSWD_CHAR = doc["pswd"];

        configFile.close();

        return true;

    }

    //=============== HTML WEBPAGE (BACK PAGE) =============== //
    const char back_html[] PROGMEM = R"rawliteral(
        <center>
        <h2>Berhasil diupdate</h2>
        <h2>
            <a href="/">
                <button style="padding: 12px 32px; background-color: #FFB200; border: 0; border-radius: 5px 5px 5px 5px; cursor: pointer; color:#fff">
                    Kembali
                </button>
            </a>
        </h2>
        </center>
    )rawliteral";

    //=============== HTML WEBPAGE (INDEX PAGE) =============== //
    const char index_html[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>Control Panel</title>
    </head>
    <body>
    <center>
        <h2>
            Control Panel
        </h2>
        <h2>
            <a href="/jadwal">
                <button style="padding: 12px 32px; background-color: #277BC0; border: 0; border-radius: 5px 5px 5px 5px; cursor: pointer; color:#fff">
                    Atur Jadwal
                </button>
            </a>
        </h2>
        <h2>
            <a href="/config">
                <button style="padding: 12px 32px; background-color: #277BC0; border: 0; border-radius: 5px 5px 5px 5px; cursor: pointer; color:#fff">
                    WiFi Config
                </button>
            </a>
        </h2>
        <p>
            <a href="wa.me/621252616770" style="text-decoration:none;">Daffa</a>
        </p>
    </center>
        
    </body>
    </html>
    )rawliteral";

    //=============== HTML WEBPAGE (HANDLE SCHEDULE) =============== //
    const char jadwal_html[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>Control Panel</title>
    </head>
    <body>
        <center>
            <h2>Pengaturan Jadwal</h2>
            <p>Format Waktu 24 Jam</p>
            <br>
        <form action="/get" target="hidden-form>
            <div style="margin: 10px ;">
                <label><b>Jam ke 1</b></label>
                <input type="number" size="3" name="jam1" style="width: 30px;" value="%jam1%">
                <label> : </label>
                <input type="number" size="3" name="menit1" style="width: 30px;" value="%menit1%">
                <label> | Dering :</label>
                <input type="number" size="3" name="ring1" style="width: 30px;" value="%ring1%">
            </div>
            <div style="margin: 10px ;">
                <label><b>Jam ke 2</b></label>
                <input type="number" size="3" name="jam2" style="width: 30px;" value="%jam2%">
                <label> : </label>
                <input type="number" size="3" name="menit2" style="width: 30px;" value="%menit2%">
                <label> | Dering :</label>
                <input type="number" size="3" name="ring2" style="width: 30px;" value="%ring2%">
            </div>
            <div style="margin: 10px ;">
                <label><b>Jam ke 3</b></label>
                <input type="number" size="3" name="jam3" style="width: 30px;" value="%jam3%">
                <label> : </label>
                <input type="number" size="3" name="menit3" style="width: 30px;" value="%menit3%">
                <label> | Dering :</label>
                <input type="number" size="3" name="ring3" style="width: 30px;" value="%ring3%">
            </div>
            <div style="margin: 10px ;">
                <label><b>Jam ke 4</b></label>
                <input type="number" size="3" name="jam4" style="width: 30px;" value="%jam4%">
                <label> : </label>
                <input type="number" size="3" name="menit4" style="width: 30px;" value="%menit4%">
                <label> | Dering :</label>
                <input type="number" size="3" name="ring4" style="width: 30px;" value="%ring4%">
            </div>
            <div style="margin: 10px ;">
                <label><b>Jam ke 5</b></label>
                <input type="number" size="3" name="jam5" style="width: 30px;" value="%jam5%">
                <label> : </label>
                <input type="number" size="3" name="menit5" style="width: 30px;" value="%menit5%">
                <label> | Dering :</label>
                <input type="number" size="3" name="ring5" style="width: 30px;" value="%ring5%">
            </div>
            <div style="margin: 10px ;margin-top: 20px;">
                <input type="submit" value="Submit">
            </div>
        </form>
    </center>
        
    </body>
    </html>
    )rawliteral";

    //=============== HTML WEBPAGE (HANDLE CONFIG) =============== //
    const char wifi_html[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>Control Panel</title>
    </head>
    <body>
        <center>
        <form action="/get">
            <div style="margin: 10px ;">
                <label><b>SSID :</b></label>
                <input type="text" size="20" name="ssid" value="%ssid%">
            </div>
            <div style="margin: 10px ;">
                <label><b>Password :</b></label>
                <input type="password" size="20" name="pswd">
            </div>
            <div style="margin: 10px ;margin-top: 20px;">
                <input type="submit" value="Submit">
            </div>
        </form>
    </center>
        
    </body>
    </html>
    )rawliteral";

    void saveConfig() {
        StaticJsonDocument<700> doc;
        doc["ssid"] = VALUE_SSID;
        doc["pswd"] = VALUE_PSWD;
                
        doc["j1"] = VALUE_JAM_1;
        doc["j2"] = VALUE_JAM_2;
        doc["j3"] = VALUE_JAM_3;
        doc["j4"] = VALUE_JAM_4;
        doc["j5"] = VALUE_JAM_5;

        doc["m1"] = VALUE_MENIT_1;
        doc["m2"] = VALUE_MENIT_2;
        doc["m3"] = VALUE_MENIT_3;
        doc["m4"] = VALUE_MENIT_4;
        doc["m5"] = VALUE_MENIT_5;

        doc["tone1"] = VALUE_RING_1;
        doc["tone2"] = VALUE_RING_2;
        doc["tone3"] = VALUE_RING_3;
        doc["tone4"] = VALUE_RING_4;
        doc["tone5"] = VALUE_RING_5;

        File configFile = SPIFFS.open("/config.json", "w");
        serializeJson(doc, configFile);
        configFile.close();
    }

    //=============== PRINT CONFIG =============== //
    void printConfig() {
        Serial.println("---------------------------------------------");
        // Jam 1
        Serial.print("JAM 1 -> ");
        Serial.print(VALUE_JAM_1);
        Serial.print(":");
        Serial.print(VALUE_MENIT_1);
        Serial.print(" WIB | Ring :");
        Serial.println(VALUE_RING_1);
        // Jam 2
        Serial.print("JAM 2 -> ");
        Serial.print(VALUE_JAM_2);
        Serial.print(":");
        Serial.print(VALUE_MENIT_2);
        Serial.print(" WIB | Ring :");
        Serial.println(VALUE_RING_2);
        // Jam 3
        Serial.print("JAM 3 -> ");
        Serial.print(VALUE_JAM_3);
        Serial.print(":");
        Serial.print(VALUE_MENIT_3);
        Serial.print(" WIB | Ring :");
        Serial.println(VALUE_RING_3);
        // Jam 4
        Serial.print("JAM 4 -> ");
        Serial.print(VALUE_JAM_4);
        Serial.print(":");
        Serial.print(VALUE_MENIT_4);
        Serial.print(" WIB | Ring :");
        Serial.println(VALUE_RING_4);
        // Jam 5
        Serial.print("JAM 5 -> ");
        Serial.print(VALUE_JAM_5);
        Serial.print(":");
        Serial.print(VALUE_MENIT_5);
        Serial.print(" WIB | Ring :");
        Serial.println(VALUE_RING_5);
        Serial.println("---------------------------------------------");
    }

    //=============== String Processor =============== //
    String processor(const String& var) {
        if(var == "ssid") {
            return VALUE_SSID;
        } else if(var == "pswd") {
            return VALUE_PSWD;
        }

        else if(var == "jam1") {
            return VALUE_JAM_1;
        } else if(var == "jam2") {
            return VALUE_JAM_2;
        } else if(var == "jam3") {
            return VALUE_JAM_3;
        } else if(var == "jam4") {
            return VALUE_JAM_4;
        } else if(var == "jam5") {
            return VALUE_JAM_5;
        } 

        else if(var == "menit1") {
            return VALUE_MENIT_1;
        } else if(var == "menit2") {
            return VALUE_MENIT_2;
        } else if(var == "menit3") {
            return VALUE_MENIT_3;
        } else if(var == "menit4") {
            return VALUE_MENIT_4;
        } else if(var == "menit5") {
            return VALUE_MENIT_5;
        } 

        else if(var == "ring1") {
            return VALUE_RING_1;
        } else if(var == "ring2") {
            return VALUE_RING_2;
        } else if(var == "ring3") {
            return VALUE_RING_3;
        } else if(var == "ring4") {
            return VALUE_RING_4;
        } else if(var == "ring5") {
            return VALUE_RING_5;
        } 
    }

    //=============== KONEKSI WIFI =============== //
    void wifiConnect() {

        // Variable WiFi (STA-Mode)
        char OUTPUT_SSID[32];
        char OUTPUT_PSWD[32];

        strcpy(OUTPUT_SSID, VALUE_SSID_CHAR);
        strcpy(OUTPUT_PSWD, VALUE_PSWD_CHAR);

        // Memulai STA Mode 
        WiFi.mode(WIFI_STA);
        WiFi.begin(OUTPUT_SSID, OUTPUT_PSWD);

        unsigned long connectingTime = millis();
        bool failConnect = false;

        Serial.print("Connecting to ");
        Serial.print(VALUE_SSID);
        Serial.print(" ");

        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");

            if(millis() - connectingTime >= 12000) {
                Serial.println();
                failConnect = true;
                break;
            }
        }

        // Berhasil koneksi STA-Mode
        if(WiFi.status() == WL_CONNECTED) {
            Serial.println();
            Serial.println("Koneksi Berhasil !");
            Serial.print("IPAddress : ");
            Serial.println(WiFi.localIP());
        }
        // Jika Gagal koneksi STA-Mode jalankan AP-Mode standar
        else if(failConnect == true){
            failConnect = false;

            Serial.println("Proses koneksi gagal, Memulai AP-Mode.");
            WiFi.softAP(SSID_AP, PASSWORD_AP);

            IPAddress myIP = WiFi.softAPIP();
            
            Serial.print("SSID AP : ");
            Serial.println(SSID_AP);
            Serial.print("IP-Address : ");
            Serial.println(myIP);
        }
        Serial.println("=============================================");
    }

    void setup(){

        // GPIO Mode
        pinMode(ledPin, OUTPUT);
        pinMode(resetPin, INPUT);

        // Memulai SoftwareSerial
        mySoftwareSerial.begin(9600);
        Serial.begin(115200);

        // Penting, untuk komunikasi DFPlayer
        delay(2000);

        if (!myDFPlayer.begin(mySoftwareSerial)) {
            Serial.println(F("Unable to begin:"));
            Serial.println(F("1.Please recheck the connection!"));
            Serial.println(F("2.Please insert the SD card!"));
            while(true){
                delay(0); // Code to compatible with ESP8266 watch dog.
            }
        }

        Serial.println(F("DFPlayer Mini online."));

        // Set Parameter DFPlayer
        myDFPlayer.volume(10);

        // Memulai Komunikasi RTC
        Wire.begin(5, 4);
        rtc.begin();

        // Memulai Komunikasi NTP
        timeClient.begin();

        Serial.println("");

        Serial.println("=============================================");
        Serial.println("Mounting FS");

        // Jika SPIFFS gagal dimuat
        if(!SPIFFS.begin()) {
            Serial.println("Failed to mount file system");

            return;
        }

        // Jika file config gagal dibuka
        if(!loadConfig()) {
            Serial.println("Failed to open Config");
        } else {
            Serial.println("Berhasil lorr");
        }

        printConfig();

        delay(1000);

        // Memanggil fungsi WiFi
        wifiConnect();

        // Memulai Webserver
        server.begin();

        
        //=============== PRINT CONFIG =============== //
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send_P(200, "text/html", index_html, processor);
        });

        server.on("/jadwal", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send_P(200, "text/html", jadwal_html, processor);
        });

        server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send_P(200, "text/html", wifi_html, processor);
        });

        server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) {

            if (request->hasParam(PARAM_JAM_1)) {
                VALUE_JAM_1 = request->getParam(PARAM_JAM_1)->value();
                VALUE_JAM_2 = request->getParam(PARAM_JAM_2)->value();
                VALUE_JAM_3 = request->getParam(PARAM_JAM_3)->value();
                VALUE_JAM_4 = request->getParam(PARAM_JAM_4)->value();
                VALUE_JAM_5 = request->getParam(PARAM_JAM_5)->value();
                
                VALUE_MENIT_1 = request->getParam(PARAM_MENIT_1)->value();
                VALUE_MENIT_2 = request->getParam(PARAM_MENIT_2)->value();
                VALUE_MENIT_3 = request->getParam(PARAM_MENIT_3)->value();
                VALUE_MENIT_4 = request->getParam(PARAM_MENIT_4)->value();
                VALUE_MENIT_5 = request->getParam(PARAM_MENIT_5)->value();

                VALUE_RING_1 = request->getParam(PARAM_RING_1)->value();
                VALUE_RING_2 = request->getParam(PARAM_RING_2)->value();
                VALUE_RING_3 = request->getParam(PARAM_RING_3)->value();
                VALUE_RING_4 = request->getParam(PARAM_RING_4)->value();
                VALUE_RING_5 = request->getParam(PARAM_RING_5)->value();

                saveConfig();
                printConfig();
                request->send(200, "text/html", back_html);
            }
            // GET inputInt value on <ESP_IP>/get?inputInt=<inputMessage>
            else if (request->hasParam(PARAM_SSID)) {
                VALUE_SSID = request->getParam(PARAM_SSID)->value();
                VALUE_PSWD = request->getParam(PARAM_PSWD)->value();

                request->send(200, "text/html", back_html);
                saveConfig();
                printConfig();
                ESP.reset();
            }



        });

        //rtc.adjust(DateTime(2022,8,22,19,30,0))

    }

    void loop() {

        INT_JAM_1 = VALUE_JAM_1.toInt();
        INT_JAM_2 = VALUE_JAM_2.toInt();
        INT_JAM_3 = VALUE_JAM_3.toInt();
        INT_JAM_4 = VALUE_JAM_4.toInt();
        INT_JAM_5 = VALUE_JAM_5.toInt();
        INT_MENIT_1 = VALUE_MENIT_1.toInt();
        INT_MENIT_2 = VALUE_MENIT_2.toInt();
        INT_MENIT_3 = VALUE_MENIT_3.toInt();
        INT_MENIT_4 = VALUE_MENIT_4.toInt();
        INT_MENIT_5 = VALUE_MENIT_5.toInt();
        INT_RING_1 = VALUE_RING_1.toInt();
        INT_RING_2 = VALUE_RING_2.toInt();
        INT_RING_3 = VALUE_RING_3.toInt();
        INT_RING_4 = VALUE_RING_4.toInt();
        INT_RING_5 = VALUE_RING_5.toInt();

        if(digitalRead(resetPin) == HIGH) {
            VALUE_SSID = "";
            VALUE_PSWD = "";
            saveConfig();
            ESP.reset();
        }

        unsigned long currentMillis = millis();
        // --------------- RTC TIME --------------- // 
        if(currentMillis - previousJam >= 1000) {

            // Update waktu RTC
            DateTime now = rtc.now();

            // Set Value kedalam Variable RTC
            RTC_JAM = now.hour();
            RTC_MENIT = now.minute();
            RTC_DETIK = now.second();

            Serial.print("RTC -> ");
            Serial.print(now.year());
            Serial.print("/");
            Serial.print(now.month());
            Serial.print("/");
            Serial.print(now.day());
            Serial.print(" ");
            Serial.print(RTC_JAM);
            Serial.print(":");
            Serial.print(RTC_MENIT);
            Serial.print(":");
            Serial.println(RTC_DETIK);

            // Serial.print("JAM ALARM ~~~~~~~~~~~~~> ");
            // Serial.print(INT_JAM_1);
            // Serial.print(":");
            // Serial.println(INT_MENIT_1);

            if(RTC_JAM == INT_JAM_1 && RTC_MENIT == INT_MENIT_1 && RTC_DETIK == 1) {
                Serial.println("ALARM AKTIF");
                myDFPlayer.play(INT_RING_1);
            }else if(RTC_JAM == INT_JAM_2 && RTC_MENIT == INT_MENIT_2 && RTC_DETIK == 1) {
                Serial.println("ALARM AKTIF");
                myDFPlayer.play(INT_RING_2);
            }else if(RTC_JAM == INT_JAM_3 && RTC_MENIT == INT_MENIT_3 && RTC_DETIK == 1) {
                Serial.println("ALARM AKTIF");
                myDFPlayer.play(INT_RING_3);
            }else if(RTC_JAM == INT_JAM_4 && RTC_MENIT == INT_MENIT_4 && RTC_DETIK == 1) {
                Serial.println("ALARM AKTIF");
                myDFPlayer.play(INT_RING_4);
            }else if(RTC_JAM == INT_JAM_5 && RTC_MENIT == INT_MENIT_5 && RTC_DETIK == 1) {
                Serial.println("ALARM AKTIF");
                myDFPlayer.play(INT_RING_5);
            }

            previousJam = currentMillis;
        }

        // --------------- NTP TIME --------------- // 
        if(currentMillis - previousUpdate >= 1000) {
            if(WiFi.status() == WL_CONNECTED) {

                // Update waktu NTP
                timeClient.update();

                time_t epochTime = timeClient.getEpochTime();
                struct tm *ptm = gmtime((time_t *)&epochTime);

                // Variable Waktu NTP
                NTP_JAM = timeClient.getHours();
                NTP_MENIT = timeClient.getMinutes();
                NTP_DETIK = timeClient.getSeconds();
                int NTP_TANGGAL = ptm->tm_mday;
                int NTP_BULAN = ptm->tm_mon+1;
                int NTP_TAHUN = ptm->tm_year+1900;

                Serial.print("NTP -> ");
                Serial.print(NTP_TAHUN);
                Serial.print("/");
                Serial.print(NTP_BULAN);
                Serial.print("/");
                Serial.print(NTP_TANGGAL);
                Serial.print(" ");
                Serial.print(NTP_JAM);
                Serial.print(":");
                Serial.print(NTP_MENIT);
                Serial.print(":");
                Serial.println(NTP_DETIK);

                // Kalibrasi Otomatis
                if(RTC_JAM != NTP_JAM || RTC_MENIT != NTP_MENIT) {
                    rtc.adjust(DateTime(NTP_TAHUN, NTP_BULAN, NTP_TANGGAL, NTP_JAM, NTP_MENIT, NTP_DETIK));
                }

            } else {
                Serial.println("Tidak terkoneksi ke jaringan.");
            }

            previousUpdate = currentMillis;
        }

        // --------------- LED BLINK INDICATOR --------------- // 
        if(WiFi.status() == WL_CONNECTED) {
            if(ledState == true) {
                if(currentMillis - previousLed >= 200) {
                    ledState = false;

                    previousLed = currentMillis;
                }
            } else {
                if(currentMillis - previousLed >= 1500) {
                    ledState = true;

                    previousLed = currentMillis;
                }
            }
        } else {
            if(currentMillis - previousLed >= 300) {

                ledState = !ledState;
                previousLed = currentMillis;
            }
        }

        digitalWrite(ledPin, ledState);
        
    }
