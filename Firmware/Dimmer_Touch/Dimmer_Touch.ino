#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//const char* ssid = "Maestros";//escribe tu SSID
//const char* password = "docentes_2018";//escribe tu password
//const char* ssid = "WiFi Aguilera";//escribe tu SSID
//const char* password = "0142035152";//escribe tu password
//const char* ssid = "LabElectro-2";//escribe tu SSID
//const char* password = "njLHwHh43";//escribe tu password
const char* ssid = "AndroidAP";//escribe tu SSID
const char* password = "ranqueles";//escribe tu password
const char* mqtt_server = "test.mosquitto.org"; /// MQTT Broker
int mqtt_port = 1883;

int Lampara = 16; // Lampara rele...pin DO
const byte zc_pin = 5; //Entrada ZC... pin D1
const byte triac_gate = 4; //Salida PWM... pin D2
int TouchSensor = 14; //sensor...pin D5
int Valor;
int EstadoLuz = 0;
bool ZC = 0;
uint16_t alpha;
WiFiServer server(80);
void ICACHE_RAM_ATTR ZC_detect ();
 
WiFiClient espClient;
PubSubClient clienteMQTT(espClient);
long lastMsg = 0;
char msg[50];

void setup() {
  Serial.begin(115200); //Inicializamos la comunicacion con el modulo
  delay(10);
  pinMode(Lampara, OUTPUT); //Definimos el pin D0 como salida
  digitalWrite(Lampara, LOW); //Iniciamos con la lampara Apagada
  pinMode(TouchSensor, INPUT); 
  pinMode(zc_pin,INPUT_PULLUP); 
  pinMode(triac_gate, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(zc_pin),ZC_detect,FALLING);
  //attachInterrupt(0, ZC_detect, CHANGE); 
  digitalWrite(triac_gate, LOW);// Enable external interrupt (INT0)


  // Conectamos a el WiFi
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

  clienteMQTT.setServer(mqtt_server, mqtt_port);
  clienteMQTT.setCallback(mensajeRecibido);
  
  while(!clienteMQTT.connected()){
    if (clienteMQTT.connect("ModuloDimmerLampara01")){
      Serial.println("Connected ");
      Serial.print("MQTT Server ");
      Serial.print(mqtt_server);
      Serial.print(":");
      Serial.println(String(mqtt_port)); 
      Serial.print("ESP8266 IP ");
      Serial.println(WiFi.localIP()); 
      clienteMQTT.subscribe("Casa/Habitaciones/Habitacion1/#");
    }else{
      Serial.print("Falla en la conexion codigo: ");
      Serial.print(clienteMQTT.state());
      Serial.println("reconexion en 5 segundos");
      delay(5000);
    }
  }
} //void setup

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
 /*for (int i = 0; i < length; i++) {
  Serial.print(topic[i]);
  stopic = (topic[i]); // spayload = spayload +
  nluz = splitString (stopic,'/',3);
 }*/
  nluz = splitString (String(topic),'/',3);
  Serial.print("Luz: ");
  Serial.println(nluz); 
  Serial.println();
  Serial.print("spayload: ");
  Serial.println(spayload);

if (nluz == "Luz1") {
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
if (nluz == "Luz2"){
   if (spayload.toInt()!=0){
  Serial.print("Numero: ");
  Serial.println(spayload.toInt());
  Valor = spayload.toInt();//Dimmer
 }
}
} //Mensaje recibido

void ZC_detect() {
  ZC = 1;
}

void loop()
 {
    if (digitalRead(TouchSensor) == HIGH) //lectura de señal del sensor
  {
    delay(150);
    if (digitalRead(TouchSensor) == HIGH) //lectura de señal del sensor
      {   
        if (EstadoLuz == HIGH)
        {
          EstadoLuz = LOW;
          digitalWrite(Lampara, LOW);   // si la lectura esta en alto el led se enciende
          clienteMQTT.publish("Casa/Habitaciones/Habitacion1/Sensor/", "OFF");
          Serial.println("Rele Apagado");
        }//ESTADO LUZ
        else
        {
          EstadoLuz = HIGH;
          digitalWrite(Lampara, HIGH);    // si no existe lectura el ed se apaga o no enciende
          clienteMQTT.publish("Casa/Habitaciones/Habitacion1/Sensor/", "ON");
          Serial.println("Rele Encendido");
        } //ELSE
      }
  } // if sensor
    //delay(1);
    //Serial.println("Cliente desconectado");
    clienteMQTT.flush();
    clienteMQTT.loop();
 // }

  // Checamos si coincide la solicitud


  if(ZC){
    if(alpha < 9500) {
      delayMicroseconds(alpha);
      digitalWrite(triac_gate, HIGH);
      delayMicroseconds(200);
      digitalWrite(triac_gate, LOW);
    }
    ZC = 0;
 
    alpha = ( 1023 - Valor ) * 10;
    if(alpha > 9500)
      alpha = 9500;
  }
}//loop

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

