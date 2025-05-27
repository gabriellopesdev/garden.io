# Garden.IO – Sistema de Irrigação Inteligente

Garden.IO é um projeto de **irrigação automática inteligente** para jardins, baseado em IoT. O sistema utiliza um microcontrolador **ESP32**, um **sensor capacitivo de umidade do solo** e uma **bombinha de aquário** acionada por um **módulo relé**. Os dados de umidade são enviados via MQTT (sobre TCP/IP) a um broker local (Mosquitto). Uma API em **Flask (Python)** verifica o nível de umidade atual e a previsão do tempo (por WeatherAPI). Se a umidade estiver abaixo do limiar configurado e não houver previsão de chuva, a API publica no broker o comando `"acionar"`. O ESP32, inscrito nesse tópico de comando, liga o relé por um tempo determinado para irrigar. Assim, a irrigação é **automatizada**, evitando desperdício de água em dias de chuva.

## Componentes de Hardware

- **ESP32 DevKit V1**: microcontrolador Wi-Fi responsável por ler sensores e controlar o relé.
- **Sensor de umidade do solo (capacitivo)**: mede a umidade em % do solo via leitura analógica. Ligado ao pino ADC do ESP32.
- **Módulo relé 5V**: aciona a bomba de água de 12V. Conectado ao ESP32 (p.ex. pino digital 26) e ao VCC/GND.
- **Bombinha de aquário (12V DC)**: fornece água para irrigação. Ligada à fonte de 12V através do relé.
- **Breadboard e jumpers**: facilitam as conexões entre ESP32, sensor e relé.
- **Fonte de alimentação / 4 pilhas AA (6V)**: alimenta o ESP32 e os demais componentes. Em geral usa-se uma fonte de 5V/USB para o ESP32 e 12V separado para a bomba (4 pilhas AA podem alimentar 6V para o ESP32 via conversor).

## Funcionamento Básico

1. **Leitura do Sensor**: O ESP32 lê periodicamente a tensão do sensor de umidade (por exemplo, a cada 30 segundos) e converte o valor em porcentagem de umidade do solo.
2. **Publicação MQTT**: Essa porcentagem é enviada ao broker Mosquitto no tópico `sensor/umidade`, no formato JSON, por exemplo:

   ```cpp
   int leitura = analogRead(SENSOR_PIN);
   int umidade = map(leitura, seco, molhado, 0, 100);
   char payload[50];
   snprintf(payload, sizeof(payload), "{\"umidade\": %d}", umidade);
   client.publish("sensor/umidade", payload);
   ```

3. **Servidor Flask (Python)**: Uma aplicação web Flask executa localmente (pode ser num PC ou Raspberry Pi). Ela consome os dados MQTT ou consulta periodicamente o broker para obter a última umidade. Paralelamente, a cada verificação ela faz uma requisição à WeatherAPI (ou outra API de meteorologia) para obter previsão de chuva.
4. **Decisão de Irrigação**: Se a umidade atual estiver abaixo do valor mínimo configurado **_e_** a previsão apontar probabilidade de chuva baixa (ou nenhuma chuva), a API Flask publica no broker o comando de irrigação:

   ```python
   mqtt_client.publish("sensor/comando", "acionar")
   ```

5. **Ação no ESP32**: O ESP32 está inscrito no tópico `sensor/comando`. Quando recebe a mensagem `"acionar"`, ele ativa o relé ligado à bomba por um tempo definido (por exemplo 20 segundos). Em código Arduino/C++:

   ```cpp
   void callback(char* topic, byte* payload, unsigned int length) {
       String message;
       for (unsigned int i = 0; i < length; i++) {
           message += (char)payload[i];
       }
       if (String(topic) == "sensor/comando" && message == "acionar") {
           digitalWrite(RELE_PIN, HIGH);  // Liga bomba
           delay(20000);                 // Aguarda 20 segundos
           digitalWrite(RELE_PIN, LOW);   // Desliga bomba
       }
   }
   ```

6. **Loop Contínuo**: Esse processo se repete continuamente, garantindo que o jardim seja regado somente quando necessário e sem intervenção manual.

## Software e Código

### Firmware do ESP32 (main.cpp)

- **Bibliotecas**: Utiliza Arduino core para ESP32, além de `WiFi.h` (conexão Wi-Fi) e `PubSubClient.h` (cliente MQTT).
- **Configurações**: No início do código são definidas as credenciais Wi-Fi e o IP do broker MQTT. Exemplo:

  ```cpp
  const char *ssid = "SEU_SSID";
  const char *password = "SUA_SENHA";
  const char *mqtt_server = "192.168.0.25"; // IP do Mosquitto
  const char *mqtt_topic = "sensor/umidade";
  const char *mqtt_command_topic = "sensor/comando";
  #define SENSOR_PIN 33   // Pino ADC do sensor
  #define RELE_PIN   26   // Pino digital do relé
  ```

- **Leitura e Publicação**: No loop principal, a cada intervalo (ex.: 30s) lê o sensor analógico, mapeia para 0–100% e publica:

  ```cpp
  int leitura = analogRead(SENSOR_PIN);
  leitura = constrain(leitura, molhado, seco);
  int umidade = map(leitura, seco, molhado, 0, 100);
  char payload[50];
  snprintf(payload, sizeof(payload), "{\"umidade\": %d}", umidade);
  client.publish(mqtt_topic, payload);
  ```

- **Recebimento de Comandos**: Uma função de callback (`client.setCallback(callback)`) é registrada para tratar mensagens recebidas no tópico `sensor/comando`. Se o comando for `"acionar"`, o relé é ativado.
- **Execução**: No `setup()`, inicializa serial, configura pinos, conecta ao Wi-Fi e configura o cliente MQTT. No `loop()`, garante reconexão ao broker e executa `client.loop()` para processar mensagens.

_Exemplo de trecho do código principal do ESP32:_

```cpp
// Setup e conexão MQTT
Serial.begin(115200);
setup_wifi();  // conecta ao Wi-Fi
client.setServer(mqtt_server, 1883);
client.setCallback(callback);

// Loop principal
if (!client.connected()) reconnect();
client.loop();

// A cada 30s, lê sensor e publica umidade
unsigned long now = millis();
if (now - lastReadTime >= 30000) {
    lastReadTime = now;
    int leitura = analogRead(SENSOR_PIN);
    int umidade = map(constrain(leitura, molhado, seco), seco, molhado, 0, 100);
    char payload[50];
    snprintf(payload, sizeof(payload), "{\"umidade\": %d}", umidade);
    client.publish("sensor/umidade", payload);
}
```

### API Flask (Servidor Python)

- **Ambiente**: Requer Python 3 com bibliotecas `Flask`, `paho-mqtt` (ou `flask-mqtt`), e `requests`. Um arquivo `requirements.txt` típico incluiria:

  ```
  Flask
  paho-mqtt
  requests
  python-dotenv
  ```

- **Funcionamento**: A aplicação Flask roda em uma máquina na mesma rede. Ela mantém conexão com o broker MQTT (usando o IP configurado) e também acessa a API de previsão do tempo (WeatherAPI).

- **Endpoints Principais**: Por exemplo, um endpoint `/irrigar` pode ser criado para acionar a checagem de irrigação manualmente. A lógica interna seria:

  1. Obter último valor de umidade (via MQTT ou consulta de armazenamento).
  2. Chamar a WeatherAPI (substituir `API_KEY` e `LOCALIZACAO`):

     ```python
     resposta = requests.get(
       "http://api.weatherapi.com/v1/forecast.json",
       params={"key": "API_KEY", "q": "LOCALIZACAO", "hours": 1}
     ).json()
     chuva = resposta['forecast']['forecastday'][0]['day']['daily_chance_of_rain']
     ```

  3. Se `umidade < LIMIAR` e `chuva < 50%` (por exemplo), publica comando ao tópico MQTT:

     ```python
     mqtt_client.publish("sensor/comando", "acionar")
     ```

  4. Retorna JSON de status para o usuário.

- **Exemplo de Código Flask**:

  ```python
  from flask import Flask, jsonify
  import requests, paho.mqtt.client as mqtt

  app = Flask(__name__)
  mqtt_client = mqtt.Client()
  mqtt_client.connect("192.168.0.25", 1883)  # IP do broker

  @app.route("/irrigar", methods=["GET"])
  def irrigar():
      umidade = get_current_humidity()  # Função que lê a umidade atual (ex: armazenada ou via MQTT)
      # Consulta a previsão do tempo pela WeatherAPI
      res = requests.get(
          "http://api.weatherapi.com/v1/forecast.json",
          params={"key": "SUA_API_KEY", "q": "SuaLocalizacao", "days": 1}
      ).json()
      chuva = res['forecast']['forecastday'][0]['day']['daily_chance_of_rain']

      if umidade < 30 and chuva < 50:
          mqtt_client.publish("sensor/comando", "acionar")
          return jsonify({"status": "Irrigação acionada"})
      return jsonify({"status": "Irrigação não necessária"})
  ```

- **Como Rodar**: Defina as variáveis de ambiente (por exemplo, `API_KEY`, credenciais de MQTT) ou ajuste diretamente no código. Depois, num terminal, instale as dependências e execute:

  ```bash
  pip install -r requirements.txt
  python app.py  # ou flask run --host=0.0.0.0
  ```

## Arquitetura de Comunicação (MQTT e Fluxo de Dados)

- O **ESP32** conecta-se à rede Wi-Fi local (via TCP/IP) e conecta ao **broker MQTT Mosquitto** (IP local). Ele publica mensagens de sensor e escuta comandos de irrigação.
- O **broker Mosquitto** atua como ponte de mensagens leve (via TCP na porta 1883). É usado para comunicação em tempo real entre ESP32 e servidor Python.
- O **Servidor Flask** também se conecta ao broker MQTT. Ele pode **assinar o tópico** `sensor/umidade` ou obter os dados a partir dele, e publica no tópico `sensor/comando` quando deve irrigar.
- **Tópicos MQTT Utilizados**:

  - `sensor/umidade`: onde o ESP32 publica o nível de umidade (payload JSON).
  - `sensor/comando`: onde o servidor publica comandos (ex.: `"acionar"`). O ESP32 está inscrito aqui e age quando recebe mensagens.

- **Fluxo de Dados**:

  1. ESP32 lê o sensor → Publica `{ "umidade": X }` em `sensor/umidade`.
  2. Broker distribui essa mensagem ao servidor Flask.
  3. Servidor Flask verifica umidade + previsão do tempo.
  4. Se necessário, Flask publica `"acionar"` em `sensor/comando`.
  5. ESP32 recebe `"acionar"` → Ativa relé da bomba → Irriga o jardim.

**Resumo**: o sistema usa o protocolo **MQTT sobre TCP/IP** para integrar sensores e atuadores. Essa arquitetura permite escalabilidade (vários sensores e válvulas), além de ser **leve e eficiente** em redes locais. A lógica central de irrigação (decisão baseada em sensor + clima) ocorre no servidor Flask, enquanto o ESP32 foca em medições e acionamento.

## Como Reproduzir o Projeto

1. **Configurar Ambiente**: Instale o Mosquitto (broker MQTT) numa máquina local ou Raspberry Pi. Garanta que ele escute na rede local (IP acessível pelo ESP32 e servidor).
2. **Preparar o Firmware do ESP32**:

   - Edite o `main.cpp` com seu SSID, senha Wi-Fi e IP do broker MQTT.
   - Ajuste os pinos `SENSOR_PIN` e `RELE_PIN` conforme sua montagem.
   - Compile e faça upload do código ao ESP32 (usando PlatformIO no VSCode ou Arduino IDE).

3. **Montagem do Hardware**: Conecte o sensor de umidade ao ADC do ESP32, o relé a um pino digital configurado, e a bomba ao relé. Alimente o ESP32 (5V via USB ou pack 4xAA) e a bomba (fonte 12V separada, ou pack adequado).
4. **Servidor Flask**:

   - Crie um arquivo `.env` ou ajuste diretamente no código as credenciais da WeatherAPI (`API_KEY`) e do broker MQTT.
   - Instale dependências: `pip install Flask paho-mqtt requests`.
   - Execute o servidor: `python app.py`. O Flask estará disponível no endereço configurado (ex.: `http://<IP_do_servidor>:5000/`).

5. **Testar o Sistema**: Verifique as mensagens no broker (por exemplo, use um cliente MQTT ou logs). A cada leitura do sensor, deve haver uma publicação em `sensor/umidade`. Acesse o endpoint `/irrigar` (ou similar) do Flask para forçar a verificação. Se as condições estiverem certas, o ESP32 acionará a bomba conforme esperado.

## Considerações Finais

Esse sistema automatiza a irrigação usando dados reais de solo e de clima. O uso do **protocolo MQTT/TCP-IP** permite que todos os componentes (ESP32, servidor Python, dispositivos de monitoramento) comuniquem-se de forma eficiente em uma rede Wi-Fi local. Com base nos sensores de umidade e na previsão meteorológica, o Garden.IO **aciona automaticamente a irrigação**, economizando água e evitando intervenções manuais desnecessárias.

**Dependências de Software:** ESP32 (Arduino/PubSubClient), Python 3, Flask, paho-mqtt, requests, e acesso à WeatherAPI.

**Exemplo de uso:** após ligar os dispositivos e rodar o servidor, monitore o tópico `sensor/umidade` para ver as leituras periódicas. Para acionar manualmente, acesse via navegador ou `curl` o endpoint do Flask (ex.: `http://localhost:5000/irrigar`) e observe o ESP32 ativar o relé se as condições forem atendidas.
