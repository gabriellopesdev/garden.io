## Configurando e Executando o Mosquitto MQTT com Docker

### Etapas para Build e Execução

1. **Certifique-se de que o Docker está instalado**:

   - Verifique se o Docker está instalado no seu sistema. Caso não esteja, siga as instruções no site oficial do [Docker](https://www.docker.com/).

2. **Estrutura do Projeto**:

   - Certifique-se de que o arquivo `Dockerfile` está localizado no diretório `infra` e que o arquivo de configuração `mosquitto.conf` está no caminho correto:
     ```
     /Users/gabriellopes/Projects/garden.io/
     ├── infra/
     │   └── Dockerfile
     ├── mosquitto/
     │   └── config/
     │       └── mosquitto.conf
     ```

3. **Build da Imagem Docker**:

   - Navegue até o diretório `infra` onde o `Dockerfile` está localizado:
     ```bash
     cd /Users/gabriellopes/Projects/garden.io/infra
     ```
   - Execute o comando para construir a imagem Docker:
     ```bash
     docker build -t mosquitto-mqtt .
     ```

4. **Executar o Container**:
   - Após o build, execute o container com o seguinte comando:
     ```bash
     docker run -it -p 1883:1883 -v "$PWD/mosquitto/config:/mosquitto/config" mosquitto-mqtt 
     ```
