#include "DHT.h" // Incluye librería para sensor DHT
#include <WiFi.h> //para ESP32
#include <ThingSpeak.h>
#include <UniversalTelegramBot.h>
#include <ESP32Servo.h>
#include <WiFiClientSecure.h>

#define DHTPIN 14    // Pin donde está conectado el sensor
#define DHTTYPE DHT22 // Sensor DHT22 (temperatura y humedad)

DHT dht(DHTPIN, DHTTYPE); // Se declara el objeto para el sensor DHT

Servo servoVentana1;  // Crea un objeto servo para controlar la primera ventana
Servo servoVentana2;  // Crea un objeto servo para controlar la segunda ventana

int pinSensorLuz = 12; // Pin para el sensor de luz (fotorresistor) 
int pinBomba = 25; // Pin para controlar la mini bomba sumergible
int pinBuzzer = 27; // el buzzer está conectado al pin 12
int Ventilador = 26; // el ventilador está conectado al pin 26

int pinServoVentana1 = 18;  // pin para el servo de la ventana 1
int pinServoVentana2 = 19;  // pin para el servo de la ventana 2

// Posiciones del servo
int posicionAbierta = 30;  // 
int posicionCerrada = 0;   // 

// Umbral de temperatura para activar ventilador y ventanas 
float umbralTemperatura = 24.0;  
float umbralHumedad = 300;

// Configuración WiFi
const char* ssid = "TP-LINK_DE16";
const char* password = "66743667";

// Configuración ThingSpeak
unsigned long myChannelNumber = 2494859;
const char* myWriteAPIKey = "0WAYEI76X6HG040D";

// Configuración bot de Telegram
#define BOTtoken "6993660886:AAESXpUxAiYp0ja-pkzpK7As-6JWKyfJEwQ"  //Token del bot de Telegram

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

WiFiClient  clientTS;

void enviarMensajeTelegram(float temperatura, float humedad, int valorLuz);
void handleNewMessages(int numNewMessages); 

void setup() {
  Serial.begin(9600); // Inicia comunicación serial
  dht.begin();  // Inicializa el sensor DHT
  pinMode(pinBomba, OUTPUT); // Configura el pin de la bomba como salida
  pinMode(pinSensorLuz, INPUT); // Configura el pin del sensor de luz como entrada 
  pinMode(pinBuzzer, OUTPUT);  // Configura el pin del buzzer como salida
  pinMode(Ventilador, OUTPUT); // Configura el pin del ventilador como salida
  
  // Inicializa los servos
  servoVentana1.attach(pinServoVentana1);  // Asocia el servo a su pin
  servoVentana2.attach(pinServoVentana2);  // Asocia el servo a su pin

  // Mueve los servos a la posición cerrada inicialmente
  servoVentana1.write(posicionCerrada);
  servoVentana2.write(posicionCerrada);

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado");

  // Inicializar ThingSpeak
  ThingSpeak.begin(clientTS);
}

void loop() {
  // Lee los sensores
  float humedad = dht.readHumidity();
  float temperatura = dht.readTemperature();
  int valorLuz = analogRead(pinSensorLuz);
  float humedadSuelo = 300; 
  
  enviarMensajeTelegram(20.0, 50.0, 100);
  handleNewMessages(5);

  if (temperatura > umbralTemperatura) {
    // Abre ventanas y enciende el ventilador
    servoVentana1.write(posicionAbierta);
    servoVentana2.write(posicionAbierta);
    digitalWrite(Ventilador, HIGH);
  } else {
    // Cierra ventanas y apaga el ventilador
    servoVentana1.write(posicionCerrada);
    servoVentana2.write(posicionCerrada);
    digitalWrite(Ventilador, LOW);
  }

  if (humedad < umbralHumedad) {
    // Si la humedad está por debajo del umbral activa el buzzer 
    digitalWrite(pinBuzzer, HIGH);
  } else {
    // Si la humedad está por encima del umbral, apaga el buzzer
    digitalWrite(pinBuzzer, LOW);
  }

  // Enviar datos a ThingSpeak
  ThingSpeak.setField(1, temperatura);
  ThingSpeak.setField(2, humedad);
  ThingSpeak.setField(3, valorLuz);
  ThingSpeak.setField(4, humedadSuelo);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
   
  // Control de la bomba basado en humedad del suelo
  if (humedadSuelo < 300) {
    digitalWrite(pinBomba, HIGH); //activa la bomba
  } else {
    digitalWrite(pinBomba, LOW); //desactiva la bomba
  }

  // Chequear nuevos mensajes de Telegram cada 2 segundos
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    Serial.println("Recibido mensaje de Telegram");
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }

  delay(2000);
}
  // Enviar mensaje por Telegram
  
  void enviarMensajeTelegram(float temperatura, float humedad, int valorLuz) {
  String mensaje = "Temperatura: " + String(temperatura) + " °C\n";
  mensaje += "Humedad: " + String(humedad) + " %\n";
  mensaje += "Nivel de luz: " + String(valorLuz) + "\n";
}
void handleNewMessages(int numNewMessages) {
  
}