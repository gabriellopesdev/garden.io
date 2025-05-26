#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <SPIFFS.h>

// Configurações de Wi-Fi
const char *ssid = "Lopes_2G";
const char *password = "Lopes12310";

// Configurações do MQTT
const char *mqtt_server = "192.168.0.25"; // Substitua pelo IP do seu broker Mosquitto
const char *mqtt_topic = "sensor/umidade";
const char *mqtt_command_topic = "sensor/comando";

WiFiClient espClient;
PubSubClient client(espClient);

#define SENSOR_PIN 33 // Pino ADC
#define RELE_PIN 26   // Pino do relé

int seco = 3000;
int molhado = 1300;

// Callback para processar mensagens recebidas no tópico MQTT
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);

  // Verifica se a mensagem é do tópico "sensor/comando"
  if (String(topic) == mqtt_command_topic)
  {
    // Converte o payload para uma string
    String message;
    for (unsigned int i = 0; i < length; i++)
    {
      message += (char)payload[i];
    }
    Serial.print("Comando recebido: ");
    Serial.println(message);

    // Se o comando for "acionar", ativa o relé por 20 segundos
    if (message == "acionar")
    {
      Serial.println("Ativando o relé por 20 segundos...");
      digitalWrite(RELE_PIN, HIGH); // Liga o relé
      delay(20000);                 // Aguarda 20 segundos
      digitalWrite(RELE_PIN, LOW);  // Desliga o relé
      Serial.println("Relé desativado.");
    }
  }
}

void setup_wifi()
{
  delay(10);
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  // Loop até que a conexão com o broker seja estabelecida
  while (!client.connected())
  {
    Serial.print("Conectando ao broker MQTT...");
    if (client.connect("ESP32Client"))
    {
      Serial.println("Conectado!");

      // Subscreve ao tópico "sensor/comando"
      client.subscribe(mqtt_command_topic);
      Serial.print("Inscrito no tópico: ");
      Serial.println(mqtt_command_topic);
    }
    else
    {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(RELE_PIN, OUTPUT);
  digitalWrite(RELE_PIN, LOW);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setKeepAlive(120);
  client.setCallback(callback); // Define a função de callback para mensagens recebidas
}
unsigned long lastReadTime = 0;
const unsigned long readInterval = 30000;
void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop(); // Necessário para manter a conexão viva

  unsigned long now = millis();
  if (now - lastReadTime >= readInterval)
  {
    lastReadTime = now;

    int leitura = analogRead(SENSOR_PIN);
    leitura = constrain(leitura, molhado, seco);
    int umidade = map(leitura, seco, molhado, 0, 100);

    Serial.print("Valor ADC: ");
    Serial.print(leitura);
    Serial.print(" | Umidade: ");
    Serial.print(umidade);
    Serial.println("%");

    char payload[50];
    snprintf(payload, sizeof(payload), "{\"umidade\": %d}", umidade);
    client.publish(mqtt_topic, payload);
  }
}