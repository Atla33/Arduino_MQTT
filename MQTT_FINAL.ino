#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "TP-Link_AED1";
const char* password = "13689872";
const char* mqtt_server = "18.231.186.76";

const int luzPin = 35;
const int temperaturaPin = 34;
const int ledPin = 32;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  if (strcmp(topic, "/botao") == 0) {
    if (payload[0] == '1') {
      digitalWrite(ledPin, HIGH);
      Serial.println("LED ligado");
      delay(1000);
    } else {
      digitalWrite(ledPin, LOW);
      Serial.println("LED desligado");
      delay(1000);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT Broker...");
    if (client.connect("NodeMCU_Client")) {
      Serial.println("Conectado");
      client.subscribe("/botao");
    } else {
      Serial.print("Falha na conexão. Código de erro: ");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int luzValue = map(analogRead(luzPin), 0, 4095, 0, 1000);  
  float temperaturaValue = (analogRead(temperaturaPin) / 4095.0) * 3.3;  

  float resistance = (3.3 * 10000) / temperaturaValue - 10000;  
  float temperature = 1 / (log(resistance / 10000) / 3950 + 1 / 298.15) - 273.15; 

  char luzMsg[10];
  sprintf(luzMsg, "%d", luzValue);
  client.publish("/luz", luzMsg);  
  Serial.print("Publicado no tópico '/luz': ");
  Serial.println(luzMsg);

  char temperaturaMsg[10];
    sprintf(temperaturaMsg, "%.2f", temperature);  
  client.publish("/temperatura", temperaturaMsg); 
  Serial.print("Publicado no tópico '/temperatura': ");
  Serial.println(temperaturaMsg);

  delay(3000);
}
