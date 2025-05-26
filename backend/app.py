from flask import Flask
import paho.mqtt.client as mqtt
import requests
import json
import datetime

app = Flask(__name__)

# MQTT Configuration
MQTT_BROKER = "192.168.0.25"
MQTT_PORT = 1883
MQTT_TOPIC = "sensor/umidade"
MQTT_COMMAND_TOPIC = "sensor/comando"

# WeatherAPI Configuration
WEATHER_API_KEY = "d62896c2c9534dbe83513442252605"  
LAT = -23.5505   
LON = -46.6333   

latest_message = None

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
        client.subscribe(MQTT_TOPIC)
    else:
        print(f"Failed to connect, return code {rc}")

def check_weather_and_act():
    url = f"http://api.weatherapi.com/v1/forecast.json?key={WEATHER_API_KEY}&q={LAT},{LON}&days=1"
    try:
        response = requests.get(url)
        data = response.json()
        will_it_rain = data['forecast']['forecastday'][0]['day']['daily_will_it_rain']
        print(f"Previsão de chuva para hoje? {'Sim' if will_it_rain else 'Não'}")

        if will_it_rain == 0:
            print("Sem previsão de chuva — acionando o relé via MQTT...")
            mqtt_client.publish(MQTT_COMMAND_TOPIC, "acionar")
    except Exception as e:
        print(f"Erro ao consultar a WeatherAPI: {e}")

def on_message(client, userdata, msg):
    global latest_message
    latest_message = msg.payload.decode()
    print(f"Mensagem recebida: {latest_message} no tópico: {msg.topic}")

    try:
        data = json.loads(latest_message)
        umidade = data.get("umidade", 100)
        if umidade < 50:
            print(f"Umidade abaixo de 50%: {umidade}% — verificando previsão do tempo...")
            check_weather_and_act()
    except Exception as e:
        print(f"Erro ao processar mensagem MQTT: {e}")

mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
mqtt_client.loop_start()

@app.route('/')
def home():
    return "Servidor Flask com MQTT + WeatherAPI está rodando!"

@app.route('/latest')
def get_latest_message():
    if latest_message:
        return {"topic": MQTT_TOPIC, "message": latest_message}
    return {"error": "Nenhuma mensagem recebida ainda"}, 404

if __name__ == '__main__':
    app.run(debug=True)
