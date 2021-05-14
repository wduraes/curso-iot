#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>

#define DHTPIN 10     
#define DHTTYPE DHT11 
DHT_Unified dht(DHTPIN, DHTTYPE);

#define WLAN_SSID "WI-FI"
#define WLAN_PASS "SENHA"
#define AIO_SERVER "IP"
#define AIO_USERNAME ""
#define AIO_KEY ""
#define AIO_SERVERPORT  1883

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish TEMPO = Adafruit_MQTT_Publish(&mqtt,"TEMPO");

int ChipID = ESP.getChipId(); 

//variables to hold data 
char hum[5];
char temp[5];
char payload[51];

void setup()
{
  Serial.begin(115200);
  delay(10);

  //pinMode(16,OUTPUT);
  
  // Connect to WiFi access point.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  dht.begin();
}

void loop()
{
  MQTT_connect();
  sensors_event_t event;

  //Cria o Payload combinando os dados de telemetria
  String Payload = "ID=";

  dht.temperature().getEvent(&event);
  float nn = event.temperature;
  dtostrf(nn, 2, 2, temp);

    Payload = Payload + ChipID + ";LOC=4;TEMP=" + temp + ";HUM=";

    dht.humidity().getEvent(&event);
    float no = event.relative_humidity;
    dtostrf(no, 2, 2, hum);

    Payload = Payload + hum;
    
    //convert String Payload to a char array
    int str_len = Payload.length() + 1;
    char char_array[str_len];
    Payload.toCharArray(char_array, str_len);

    Serial.println("");
    Serial.print("Payload: ");
    Serial.println(char_array);

    //Publish all value into a single Topic and wait 10 secondes 

    Serial.println("Sending Payload data... ");
    TEMPO.publish(char_array);       

    delay(30000);
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect()
{
  int8_t ret;
  if (mqtt.connected()) // Stop if already connected
  {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
  {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000); // wait 5 seconds
    retries--;
    if (retries == 0)
    {
      while (1)
        ;
    }
  }
  Serial.println("MQTT Connected!");
}