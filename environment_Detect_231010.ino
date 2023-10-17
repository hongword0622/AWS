#include <WiFi.h> 
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include "HTTPClient.h"
#include "DHT.h"
#define RXX 16
#define TXX 17
#define SERIAL_BAUD 9600
#include "PMS.h"
#include  <SoftwareSerial.h>
SoftwareSerial PMSSerial(34, 35);
PMS pms(PMSSerial);
PMS::DATA data;
int pms1_0,pms2_5,pms10_0;

SoftwareSerial mySerial1(TXX,RXX);
unsigned int CO2_ppm;
static unsigned int ucRxBuffer[10];

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

char* ssid = "120-2 1F";
char* password = "082372535";
//本機IP位置
char* mqttServer = "192.168.1.105";
//你的Mosquitto設定的帳號,密碼
const char* mqttUserName = "hong01"; 
const char* mqttPwd = "a337457A";  
const char* clientID = "hong01S";     
//本機IP位置
char* host = "192.168.1.105";
String URL = "http://192.168.1.105/PMS/pmss.php";
void setup_wifi() {
  delay(10);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
}
void reconnect() {
  while (!client.connected()) {
    if (client.connect(clientID, mqttUserName, mqttPwd)) {
      Serial.println("MQTT connected");
    } else {
      Serial.print(mqttServer);
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // 等5秒之後再重試
      
    }
  }
}
void setup() {
  Serial.begin(9600);
  mySerial1.begin(9600);
  PMSSerial.begin(9600);
  setup_wifi();
  dht.begin();
  client.setServer(mqttServer, 1883);

}
void loop(){

    if (WiFi.status() != WL_CONNECTED) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
  }
    if (!client.connected()) {
    reconnect();
  }
  client.loop();
   while (!pms.read(data)) {}  
   float temperature = dht.readTemperature();
   float humidity = dht.readHumidity();
   Serial.println(temperature);
   Serial.println(humidity);
  pms1_0 = data.PM_AE_UG_1_0;
  pms2_5 = data.PM_AE_UG_2_5;
  pms10_0 = data.PM_AE_UG_10_0;
  Serial.print("PM 1.0 (ug/m3): ");
  Serial.println(pms1_0);
  Serial.print("PM 2.5 (ug/m3): ");
  Serial.println(pms2_5);
  Serial.print("PM 10.0 (ug/m3): ");
  Serial.println(pms10_0);
  Serial.println();
  delay(2000);
 unsigned char CO2_Read[]={0x42,0x4d,0xe3,0x00,0x00,0x01,0x72};
   unsigned char CO2_val[12];
    mySerial1.write(CO2_Read,7);
    delay(1000);
    while(mySerial1.available()) {
       mySerial1.readBytes(CO2_val, 12); // 讀取回應之12 bytes 位元組的資料
    }
   CO2_ppm =CO2_val[4]*256+CO2_val[5];
   Serial.println(CO2_ppm);
   client.publish("esp32/ds/co2",String(CO2_ppm).c_str());  
  
   client.publish("esp32/dht/temp",String(temperature).c_str());    // 發布MQTT主題與訊息
   client.publish("esp32/dht/humd",String(humidity).c_str()); 
  
   client.publish("esp32/pms/pm25",String(pms2_5).c_str());
   client.publish("esp32/pms/pm1",String(pms1_0).c_str());
   client.publish("esp32/pms/pm10",String(pms10_0).c_str());  

   String postData = "temperature=" + String(temperature) + "&humidity=" + String(humidity)+"&PM1_0=" + String(pms1_0)+"&PM2_5=" + String(pms2_5)+"&PM10=" + String(pms10_0)+"&CO2_ppm=" + String(CO2_ppm);
     HTTPClient http; 
  http.begin(URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  int httpCode = http.POST(postData); 
  String payload = http.getString(); 
  
  Serial.print("URL : "); Serial.println(URL); 
  Serial.print("Data: "); Serial.println(postData); 
  Serial.print("httpCode: "); Serial.println(httpCode); 
  Serial.print("payload : "); Serial.println(payload); 
  Serial.println("--------------------------------------------------");
  Serial.println(WiFi.localIP());
   delay(300000);
}