# Garden.IO – Sistema de Irrigação Inteligente

Garden.IO é um projeto de **irrigação automática inteligente** para jardins, baseado em IoT. O sistema utiliza um microcontrolador **ESP32**, um **sensor capacitivo de umidade do solo** e uma **bombinha de aquário** acionada por um **módulo relé**. Os dados de umidade são enviados via MQTT (sobre TCP/IP) a um broker local (Mosquitto). Uma API em **Flask (Python)** verifica o nível de umidade atual e a previsão do tempo (por WeatherAPI). Se a umidade estiver abaixo do limiar configurado e não houver previsão de chuva, a API publica no broker o comando `"acionar"`. O ESP32, inscrito nesse tópico de comando, liga o relé por um tempo determinado para irrigar. Assim, a irrigação é **automatizada**, evitando desperdício de água em dias de chuva.

## Componentes de Hardware

- **ESP32 DevKit V1**: microcontrolador Wi-Fi responsável por ler sensores e controlar o relé.
- **Sensor de umidade do solo (capacitivo)**: mede a umidade em % do solo via leitura analógica.
- **Módulo relé 5V**: aciona a bomba de água.
- **Bomba de água (6V–12V DC)**: fornece água para irrigação.
- **Breadboard e jumpers**: facilitam as conexões.
- **Fonte: 4 pilhas AA (6V)**: alimenta o ESP32 e os componentes de baixa potência.

## Funcionamento Básico

1. O **ESP32** lê periodicamente a umidade do solo e publica os dados no broker MQTT no tópico `sensor/umidade`, no formato JSON:

   ```json
   { "umidade": 42 }
   ```

2. Um **servidor Flask em Python**, inscrito nesse tópico, processa os dados recebidos e consulta a **previsão do tempo** usando a API da WeatherAPI.
3. Se a umidade estiver abaixo do limiar definido (ex: 30%) e **não houver previsão de chuva**, o servidor publica a mensagem `"acionar"` no tópico `sensor/comando`.
4. O **ESP32**, ao receber a mensagem no tópico `sensor/comando`, aciona o relé por um tempo configurado (ex: 20 segundos), ativando a bombinha de água.
5. O sistema repete esse ciclo periodicamente, sem necessidade de intervenção manual.

## Software e Código

### Firmware do ESP32 (`main.cpp`)

- Desenvolvido em C++ usando PlatformIO.
- Realiza leitura do sensor, conversão para %, publicação MQTT e escuta do tópico de comando.
- Código modular com reconexão automática ao Wi-Fi e broker MQTT.

### Servidor Flask (`app.py`)

- Desenvolvido em Python 3 com bibliotecas:

  ```
  Flask
  paho-mqtt
  requests
  ```

- Escuta o tópico `sensor/umidade` e processa as mensagens recebidas.
- A cada nova leitura de umidade:

  - Consulta a WeatherAPI para verificar se há previsão de chuva.
  - Publica `"acionar"` no tópico `sensor/comando` se necessário.

## Arquitetura e Comunicação

- **Broker MQTT**: Mosquitto local (porta 1883).
- **Protocolo**: MQTT sobre TCP/IP.
- **Tópicos utilizados**:

  - `sensor/umidade`: ESP32 publica dados do sensor.
  - `sensor/comando`: Flask publica comandos de ativação.

- **Fluxo**:

  1. ESP32 → Broker: publica `{ "umidade": XX }`.
  2. Flask → consulta API climática → decide.
  3. Flask → Broker: publica `"acionar"` se necessário.
  4. ESP32 → ativa bomba de água.

## Como Reproduzir o Projeto

1. **Instale o broker MQTT** (Mosquitto) em uma máquina na mesma rede.
2. **Configure o firmware do ESP32**:

   - Edite o `main.cpp` com as credenciais Wi-Fi e IP do broker.
   - Compile e envie para o ESP32 usando PlatformIO ou Arduino IDE.

3. **Monte o hardware**: conecte o sensor, relé e bombinha conforme descrito.
4. **Configure e execute a API Flask**:

   - Instale as dependências Python.
   - Ajuste a chave da WeatherAPI no código.
   - Execute o arquivo `app.py` para iniciar o monitoramento automático.

## Conclusão

O Garden.IO é uma solução leve, eficiente e automatizada para irrigação de jardins com controle remoto via MQTT e lógica baseada em previsão do tempo. Ideal para aplicações residenciais, escolares ou de demonstração de IoT.
