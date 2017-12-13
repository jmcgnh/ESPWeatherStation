// derived from https://github.com/wero1414/ESPWeatherStation

#define DHT_DEBUG  1
#define DEBUG_ESP_WIFI
#define DEBUG_ESP_PORT Serial

#include <ESP8266WiFi.h>

#include "secret.h" // defines IDs and PASSWDs

#define DHTPIN          2   //Pin to attach the DHT - on D1 mini, what's labeled as D4 is GPIO2
#define DHTTYPE DHT22       //type of DTH  
#include "DHT.h"

const char* ssid     = MYSSID;
const char* password = SSIDPASSWD;
const int sleepTimeS = 900; //18000 for Half hour, 300 for 5 minutes etc.
const char vfname[] =  __FILE__ ;
const char vtimestamp[] =  __DATE__ " " __TIME__;
///////////////Weather////////////////////////
char server [] = "weatherstation.wunderground.com";
char WEBPAGE [] = "GET /weatherstation/updateweatherstation.php?";
char ID [] = MYWUID;
char PASSWORD [] = WUPASSWD;


/////////////IFTTT///////////////////////
//const char* host = "maker.ifttt.com";//dont change
//const String IFTTT_Event = "YourEventName";
//const int puertoHost = 80;
//const String Maker_Key = "YourMakerKey";
//String conexionIF = "POST /trigger/"+IFTTT_Event+"/with/key/"+Maker_Key +" HTTP/1.1\r\n" +
//                  "Host: " + host + "\r\n" +
//                  "Content-Type: application/x-www-form-urlencoded\r\n\r\n";
//////////////////////////////////////////
DHT dht(DHTPIN, DHTTYPE);



void setup()
{
  int waitcount = 0;
  int wifistatus = 0;
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  dht.begin();
  delay(1000);
  Serial.println();
  Serial.print("file: ");
  Serial.println(vfname);
  Serial.print("timestamp (local time): ");
  Serial.println(vtimestamp);
  Serial.println();

  // Connect D0 to RST to wake up
  // pinMode(D0, WAKEUP_PULLUP);

  // most of the time we will have been reconnected by now, so check for connection before running .begin
  if ( (wifistatus = WiFi.status()) == WL_CONNECTED) {
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.println(WiFi.localIP());
  } else {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    Serial.println(password);
    WiFi.begin(ssid, password);
    while (((wifistatus = WiFi.status()) == WL_IDLE_STATUS) && (++waitcount < 60)) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println();
  }
  Serial.print("wifi status= ");
  Serial.println(wifistatus);
}

void loop() {
  //Check battery
  //int level = analogRead(A0);
  //level = map(level, 0, 1024, 0, 100);
  //if(level<50)
  //{
  // mandarNot(); //Send IFTT
  // Serial.println("Low battery");
  // delay(500);
  //}
  //Get sensor data
  float tempc = dht.readTemperature();
  float tempf =  (tempc * 9.0) / 5.0 + 32.0;
  float humidity = dht.readHumidity();
  float dewptf = (dewPoint(tempf, dht.readHumidity()));
  //check sensor data
  Serial.println("+++++++++++++++++++++++++");
  Serial.print("tempF= ");
  Serial.print(tempf);
  Serial.println(" *F");
  Serial.print("tempC= ");
  Serial.print(tempc);
  Serial.println(" *C");
  Serial.print("dew point= ");
  Serial.println(dewptf);
  Serial.print("humidity= ");
  Serial.println(humidity);

  //Send data to Weather Underground
  Serial.print("sending data to ");
  Serial.println(server);
  WiFiClient client;
  if (!client.connect(server, 80)) {
    Serial.println("Conection Fail");
    return;
  }
  client.print(WEBPAGE);
  client.print("ID=");
  client.print(ID);
  client.print("&PASSWORD=");
  client.print(PASSWORD);
  client.print("&dateutc=");
  client.print("now");
  client.print("&tempf=");
  client.print(tempf);
  client.print("&dewptf=");
  client.print(dewptf);
  client.print("&humidity=");
  client.print(humidity);
  client.print("&softwaretype=ESP%208266O%20version1&action=updateraw&realtime=1&rtfreq=2.5");
  client.println();
  delay(2500);
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

//void mandarNot(){
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
//    Serial.print(line);
//  }
//}

void sleepMode() {
  Serial.print(F("Going into deep sleep now..."));
  ESP.deepSleep(sleepTimeS * 1000000);
}

