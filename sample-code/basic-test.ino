  //Incluindo todas as bibliotecas necessárias
  
  #include <ESP8266WiFi.h>
  #include "Adafruit_MQTT.h"
  #include "Adafruit_MQTT_Client.h"
  #include <Adafruit_Sensor.h>
  #include <DHT.h>
  #include <DHT_U.h>
  #include "SPI.h"
  #define DHTPIN 10       // define o pino do ESP8266 onde o sensor de temperatura está conectado
  #define DHTTYPE DHT11   // define o tipo do sensor que pode ser DHT11 ou DHT22
  DHT_Unified dht(DHTPIN, DHTTYPE);
  
  // Definições de Wi-Fi e do MQTT Broker.
  // Note que podemos conectar este mesmo código a qualquer outro servidor MQTT, este código nao é exclusivo para o IO.Adafruit!  
  
  #define WLAN_SSID       "Your_Wifi_name"
  #define WLAN_PASS       "Your_wifi_password"
  #define AIO_SERVER      "io.adafruit.com"
  #define AIO_USERNAME    "Your_Adafruit_account_name"
  #define AIO_KEY         "Your_Adafruit_key"
  #define AIO_SERVERPORT  1883                   
  WiFiClient client;
  //#define AIO_SERVERPORT  8883     // A porta 8883 do MQTT broker é usada para conexões usando SSL (conexão criptografdada)
  //WiFiClientSecure client;         // Para conexão segura esta biblioteca deve ser usada 

  Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

  //Definindo quais tópicos (tanto para publicar quanto para subscrever) serão utilizados por este device
  Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
  Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
  Adafruit_MQTT_Subscribe led = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/led");
  
  void MQTT_connect();
   
  void setup() //Código executado apenas quando o device é ligado ou resetado
  {       
    Serial.begin(115200);
    pinMode(16,OUTPUT);
  
    // Conectando ao Wi-Fi
    Serial.println(); Serial.println();
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
    Serial.println("IP address: "); Serial.println(WiFi.localIP());
    
    // Setup das MQTT subscriptions para todos os tópicos necessários, neste caso apenas um.
    mqtt.subscribe(&led);

    //inicializa o sensor DHT        
    dht.begin();
  }
     
  void loop() 
  {
    //reconexão automática - verifica a cada iteração do loop se a conexão está funcionando, reconectando caso nao esteja.
    MQTT_connect();
    sensors_event_t event;  
   
    Adafruit_MQTT_Subscribe *subscription;
    // Verifica se chegaram dados para qualquer dos tópicos que o device subscreve. 
    // Fica neste modo por 5 segundos e tem timeout se nao chegar nada.
    while ((subscription = mqtt.readSubscription(5000))) 
    {
     // Se entrou neste loop, é porque chegaram dados, mas nós ainda nao sabemos de qual tópico (mesmo que só tenha um)
     // Essa biblioteca limita a 5 tópicos que podem ter subscription 

     // verifica se os dadso que chegaram foram do Topico LED
      if (subscription == &led) 
      {
        Serial.print(F("Got: "));
        Serial.println((char *)led.lastread);
      }
      if (strcmp((char *)led.lastread, "ON") == 0) //Verifica se o comando recebido foi "ON"
      {
        digitalWrite(16, 1);  //Define o pino 16 como HIGH (ou 1) o que faz com que o LED receba 5 volts e acenda
      } 
      if (strcmp((char *)led.lastread, "OFF") == 0) //Verifica se o comando recebido foi "OFF"
      {
        digitalWrite(16, 0); //Define o pino 16 como LOW (ou o) o que faz com que a alimentação do LED seja interrompida, apagando-o.
      }        
    }           
   
    dht.temperature().getEvent(&event); 
    //Sempre vamos checar se os dados de temperatura que vieram do DHT11 são válidos ANTES de publicar 
    if(isnan(event.temperature))
    {
      //Se os dados recebidos do DHT11 não forem um número nao publicamos
      Serial.print("Last temperature value is not a number and it won't be published.");
    }
    else
    {
      //do contrário podemos publicar.
      if (temperature.publish(event.temperature)) 
      {
        Serial.print("Sending temperature val ");
        Serial.println(event.temperature);  
      }
    }
    
    dht.humidity().getEvent(&event);
    //mesma coisa para os dados de umidade.
    if(isnan(event.relative_humidity))
    {
      Serial.print("Last humidity value is not a number and it won't be published.");
    }
    else
    {
      if (humidity.publish(event.relative_humidity)) 
      {
        Serial.print("Sending humidity val ");
        Serial.println(event.relative_humidity);
      }
    }
  }  

  // Função para conectar e reconectar ao servidor de MQTT sempre que necessário
    void MQTT_connect() {
    
    int8_t ret;
    if (mqtt.connected()) {      // Se já estiver conectado, pode sair da função - chamando "return"
      return;
      }
    Serial.print("Connecting to MQTT... ");
    uint8_t retries = 3;
    while ((ret = mqtt.connect()) != 0) { // connect retorna 0 quando conectado
         Serial.println(mqtt.connectErrorString(ret));
         Serial.println("Retrying MQTT connection in 5 seconds...");
         mqtt.disconnect();
         delay(5000);  // espera 5 segundos
         retries--;
         if (retries == 0) {
           // nao faz mais nada e, eventualmente, o WDT(watchdog timer) vai resetar a placa e começar tudo de novo
           while (1);
         }
    }
    Serial.println("MQTT Connected!");
  }