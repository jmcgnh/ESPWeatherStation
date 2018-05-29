// derived from https://github.com/wero1414/ESPWeatherStation
// GPL2.0 License applies to this code.

#define DHT_DEBUG  1
#define DEBUG_ESP_WIFI
#define DEBUG_ESP_PORT Serial

#define PHANT01 1

#include "secret.h" // defines IDs and PASSWDs

// interfaces

// WiFi //
#include <ESP8266WiFi.h>
const char* ssid     = MYSSID;
const char* password = SSIDPASSWD;
const int sleepTimeS = 600; // in seconds; 18000 for Half hour, 300 for 5 minutes etc.
const char vfname[] =  __FILE__ ;
const char vtimestamp[] =  __DATE__ " " __TIME__;
const char versionstring[] = "20180529.0100.1";

#ifdef WUNDERGROUND
/////////////// Weather Underground ////////////////////////
char wu_host [] = "weatherstation.wunderground.com";
char wu_WEBPAGE [] = "/weatherstation/updateweatherstation.php";
char wu_ID [] = MYWUID;
char wu_PASSWORD [] = WUPASSWD;
char WU_cert_fingerprint[] = "12 DB BB 24 8E 0F 6F D4 63 EC 45 DD 5B ED 37 D7 6F B1 5F E5";
#endif

#ifdef PHANT01
/////////////// Phant ////////////////////////
char logHost [] = MYPHANTHOST;
char logWebPage [] = MYPHANTWEBPAGE;
char phantPubKey [] = MYPHANTPUBKEY;
char phantPrivKey [] = MYPHANTPRIVKEY;
char logCertFingerprint [] = PHANTSHA1FINGERPRINT;
#endif



/////////////IFTTT/////////////////////// not currently used
//const char* host = "maker.ifttt.com";//dont change
//const String IFTTT_Event = "YourEventName";
//const int puertoHost = 80;
//const String Maker_Key = "YourMakerKey";
//String conexionIF = "POST /trigger/"+IFTTT_Event+"/with/key/"+Maker_Key +" HTTP/1.1\r\n" +
//                  "Host: " + host + "\r\n" +
//                  "Content-Type: application/x-www-form-urlencoded\r\n\r\n";
//////////////////////////////////////////

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#undef BME280_ADDRESS
#define BME280_ADDRESS                (0x76)


Adafruit_BME280 bme; // I2C

unsigned long delayTime;

void setup()
{
  int wifiwaitcount = 0;
  int wifistatus = WL_IDLE_STATUS;

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  delay(1000);

  
  Serial.println();
  Serial.print("file: ");
  Serial.println(vfname);
  Serial.print("timestamp (local time): ");
  Serial.println(vtimestamp);
  Serial.println();


  // Connect D0 to RST to wake up
  pinMode(D0, WAKEUP_PULLUP);

  // most of the time we will have been reconnected by now, so check for connection before .begin()
  // this avoids a problem that crops up when calling .begin() while already connected
  if ( (wifistatus = WiFi.status()) == WL_CONNECTED) {
    Serial.print("Connected to ");   Serial.println(WiFi.SSID());
    Serial.println(WiFi.localIP());
  } else {
    Serial.print("Connecting to ");  Serial.println(ssid);
    Serial.println(password);
    WiFi.begin(ssid, password);
    while (((wifistatus = WiFi.status()) == WL_IDLE_STATUS) && (++wifiwaitcount < 60)) {
      Serial.print(".");
      delay(1000);
    }
    Serial.println();
  }
  Serial.print("wifi status= ");     Serial.println(wifistatus);
  Serial.println( "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");

    // from BME280 test code example
    bool status;

    status = bme.begin( BME280_ADDRESS );  
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
    
    Serial.println("-- BME280 Default Test --");
    delayTime = 1000;

    Serial.println();

   /* Display some basic information on this sensor */
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");

    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.println();
}

void loop() {
  //Check battery // currently not using this code
  //int level = analogRead(A0);
  //level = map(level, 0, 1024, 0, 100);
  //if(level<50)
  //{
  // mandarNot(); //Send IFTT
  // Serial.println("Low battery");
  // delay(500);
  //}

  float baromin = 100;
  float tempf = 1000;

  //Get sensor data
  float tempc = bme.readTemperature();
  tempf =  (tempc * 9.0) / 5.0 + 32.0;
  float humidity = bme.readHumidity();
  float dewptf = dewPoint(tempf, humidity);
  float baromhPa = bme.readPressure();
  baromin = baromhPa / 3386.389;

  //local sensor data report
  Serial.println("+++++++++++++++++++++++++");
  Serial.print("tempF=     ");  Serial.print(tempf);   Serial.println(" *F");
  Serial.print("tempC=     ");  Serial.print(tempc);    Serial.println(" *C");
  Serial.print("dew point= ");  Serial.print(dewptf);   Serial.println(" *F");
  Serial.print("humidity=  ");  Serial.print(humidity); Serial.println("%");
  Serial.print("barometer  ");  Serial.print(baromin);  Serial.println(" inches");
  Serial.println("vvvvvvvvvvvvvvvvvvvvvvvvvv");

  //Send data to logging site
  Serial.print("sending data to ");  Serial.println(logHost);
 
   // Using HTTPS protocol
   WiFiClientSecure client;
   if (!client.connect(logHost, 443)) {
     Serial.println("Conection Fail");
    return;
   }
   if (client.verify(logCertFingerprint, logHost)) {
     Serial.println("certificate matches");
   } else {
     Serial.println("certificate doesn't match");
   }

  String ReqData = "baromin=";    ReqData += baromin;
        ReqData += "&hum=";       ReqData += humidity;
        ReqData += "&tempf=";     ReqData += tempf;
        ReqData += "\r\n";
  Serial.println("ReqData= " + ReqData);

  String WebReq = "POST ";        WebReq += logWebPage; WebReq += " HTTP/1.1\r\n";
  WebReq += "Host: ";             WebReq += logHost;    WebReq += "\r\n";
  WebReq += "Phant-Private-Key: "; WebReq += phantPrivKey; WebReq += "\r\n";
  WebReq += "Connection: "        "close"        "\r\n";
  WebReq += "Content-Length: ";   WebReq += ReqData.length(); WebReq += "\r\n";
  WebReq += "Content-Type: application/x-www-form-urlencoded\r\n";
  WebReq += "\r\n"; // end of headers
  WebReq += ReqData; // POST data

  Serial.println("WebReq= " + WebReq);

  client.print(WebReq);
  
  Serial.println("-----Response-----");
  while (client.connected())
  {
    if (client.available())
    {
      String line = client.readStringUntil('\n');
      Serial.println(line);
    }
  }
  Serial.println("----------");
  delay(2000);
  sleepMode();
}


double dewPoint(double tempf, double humidity) //Calculate dew Point
{
  double A0 = 373.15 / (273.15 + tempf);
  double SUM = -7.90298 * (A0 - 1);
  SUM += 5.02808 * log10(A0);
  SUM += -1.3816e-7 * (pow(10, (11.344 * (1 - 1 / A0))) - 1) ;
  SUM += 8.1328e-3 * (pow(10, (-3.49149 * (A0 - 1))) - 1) ;
  SUM += log10(1013.246);
  double VP = pow(10, SUM - 3) * humidity;
  double T = log(VP / 0.61078);
  return (241.88 * T) / (17.558 - T);
}

//void mandarNot(){ // not using this yet
//  WiFiClient client;
//  if (!client.connect(host, puertoHost)) //Check connection
//  {
//    Serial.println("Failed connection");
//    return;
//  }
//  client.print(conexionIF);//Send information
//  delay(10);
//  while(client.available())
//  {
//    String line = client.readStringUntil('\r');
//    Serial.println(line);
//  }
//}

void sleepMode() {
  Serial.print("Going into deep sleep now...");
  ESP.deepSleep(sleepTimeS * 1000000);
}

/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
