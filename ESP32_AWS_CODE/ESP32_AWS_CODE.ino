#include "Certificates.h"
#include <SimpleDHT.h>
//librarires to get timestamp of sensor data
#include <NTPClient.h>
#include <WiFiUdp.h>

//libraries used for MQTT protocol
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"


//Define NTP client to request date and time from an NTP server
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedTime;
String dayStamp;
String timeStamp;


//TOPICS FOR MQTT  PROTOCOLS
#define AWS_IOT_PUBLISH_TOPIC "r4/PUB"


// This are example values we used in testing
int hum = 0;
float temper = 0;
float tds = 0;
int moisture = 0;
int ph = 0;



//MQTT Protocols
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);



void connectAWS() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);


  Serial.println("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  Serial.println("AWS IoT Connected!");
}




// Function to publish to topic stated
void publishMessage() {
  StaticJsonDocument<200> doc;
  doc["Timestamp"] = timeStamp;
  doc["Temperature"] = temper;
  doc["Humidity"] = hum;
  doc["TDS"] = tds;
  doc["Moisture"] = moisture;
  doc["PH"] = ph;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);  // print to client
  Serial.println(jsonBuffer);

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}






void setup() {
  Serial.begin(115200);
  connectAWS();

  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(3600);  // Adjust value according to your timezone


  //Ensure we get a valid date and time
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
}




void loop() {

  client.loop();
  publishMessage();
  //To get time
  formattedTime = timeClient.getFormattedTime();
  timeStamp = formattedTime;

}

