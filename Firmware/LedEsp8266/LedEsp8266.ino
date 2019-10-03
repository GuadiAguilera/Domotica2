#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Estudiantes";//escribe tu SSID
const char* password = "educar_2018";//escribe tu password
//const char* ssid = "LabElectro-2";//escribe tu SSID
//const char* password = "njLHwHh43";//escribe tu password
const char* mqtt_server = "test.mosquitto.org"; /// MQTT Broker

int EstadoLuz = 0;
int Lampara = ; // Usaremos el pin GPIO2 de el ESP8266/DO
WiFiServer server(80);

WiFiClient espClient;
PubSubClient clienteMQTT(espClient);
long lastMsg = 0;
char msg[50];

void setup() {
  Serial.begin(115200); //Inicializamos la comunicacion con el modulo
  delay(10);
  pinMode(Lampara, OUTPUT); //Definimos el pin GPI02 como salida
  digitalWrite(Lampara, LOW); //Iniciamos con el led Apagado

  // Conectamos a el WiFi
  Serial.println();
  Serial.println();
  Serial.print("Conectando a: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");

  // Iniciar servidor
  server.begin();
  Serial.println("Servidor iniciado");

  // Imprimimos el IP que nos asigna el moduoo para conectarnos
  Serial.print("Usa esta URL para conectarte: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

   while(!clienteMQTT.connected()){
    if (clienteMQTT.connect("ModuloLampara01")){
      Serial.println("Connected ");
      Serial.print("MQTT Server ");
      Serial.print(mqtt_server);
      Serial.print(":");
      Serial.println(String(mqtt_port)); 
      Serial.print("ESP8266 IP ");
      Serial.println(WiFi.localIP()); 
      clienteMQTT.subscribe("Casa/Habitaciones/Cocina/Luz1");
    }else{
      Serial.print("Falla en la conexion codigo: ");
      Serial.print(clienteMQTT.state());
      Serial.println("reconexion en 5 segundos");
      delay(5000);
    }
  }
}
void mensajeRecibido(char* topic, byte* payload, unsigned int length) {
String spayload = "";
String stopic = "";
String nluz = "";
 Serial.print("Message arrived [");
 Serial.print(topic);
 Serial.print("] ");
 for (int i = 0; i < length; i++) {
  Serial.print((char)payload[i]);
  spayload += (char)payload[i]; // spayload = spayload + 
 }
  for (int i = 0; i < length; i++) {
  Serial.print(topic[i]);
  stopic += (topic[i]); // spayload = spayload +
  nluz = splitString (stopic,'/',3);
 }
 Serial.println();
 Serial.print("spayload: ");
 Serial.println(spayload);
 
 if(nluz=="Luz1"){
  if (spayload=="ON"){
      Serial.println("Rele encendido");
      digitalWrite(Lampara, HIGH);
      EstadoLuz = HIGH;
  } else if (spayload=="OFF"){
      Serial.println("Rele Apagado");
      digitalWrite(Lampara, LOW);
      EstadoLuz = LOW;
 }
}
}// void mensajeRecibido

void loop() {
      clienteMQTT.loop();
}

String splitString (String Data, char Separator, int index){
  int found= 0;
  int strIndex[] ={0,-1};
  int maxIndex = Data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
      if(Data.charAt(i)==Separator || i==maxIndex){
          found++;
          strIndex[0] = strIndex[1]+1;
          strIndex[1] = (i ==maxIndex) ? i+1 : i;
      }
  }
  return found>index ? Data.substring(strIndex[0], strIndex[1]) : "";
}

