#include <ESP8266WiFi.h>
#define FOGAO    D3     // incluindo a biblioteca do módulo wifi e definindo os pinos.
#define sensor_temp A0

// Configuração de IP da rede

IPAddress myIP(192, 168, 43, 53);
IPAddress myMASC(255, 255, 255, 0); //definindo os IP
float temperatura = 0;                      //criando as variáveis (utilizando float pois temperatura não é inteiro e bool por o fato de fogão ser 0 ou 1)
bool fogao = 0;

//funcoes para tratar requisição e enviar respostas
void hardwareInit(void);
void trataRequest(void);
void enviaResposta(WiFiClient client, bool fogao, float temperatura);
void atualizaSaidas(bool fogao);

// Dados da rede WiFi
const char* ssid = "AndroidAP";
const char* password = "teste1208";

//corpo html
const char* headerHTLM = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<!DOCTYPE HTML>\n<html>\n";
const char* tailHTML = "</html>\n";
WiFiServer server(80);//porta comunicacao

void setup() {
  pinMode(A0, INPUT);
  Serial.begin(115200);
  pinMode(FOGAO, OUTPUT);
  digitalWrite(FOGAO, 0);
  Serial.print("\nConectando a rede: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);       // Conecta na rede
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    if (++i > 30) ESP.restart(); // Reinicia ESP após 30 segundos
  }
  Serial.println("\nWiFi Conectado!");
  server.begin();               // Inicia Server
  Serial.print("Use essa URL para conectar: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  temperatura = analogRead(sensor_temp);
  temperatura = temperatura * 3.3 / 1023;          //configura temperatura
  temperatura = temperatura / 0.01;
  Serial.print("Temperatura: ");    
  Serial.println(temperatura);
  trataRequest();   // chama funcao trata requisição do app
  atualizaSaidas(fogao);//chama funcao de atualização após passar pela de requisição
}

void trataRequest(void) {                                                                                   
  WiFiClient client = server.available();       // Testa se foi feita requisição
  Serial.println("testando cliente");
  delay(1000);
  if (!client) return;
  Serial.println("novo cliente");
  while (!client.available()) delay(1);    // Aguarda recepção dos dados
  Serial.println("dados recebidos");
  String request = client.readStringUntil('\r');           // Le dados
  Serial.println(request);
  client.flush();
  if (request.indexOf("status") != -1) {
    enviaResposta(client, fogao, temperatura);   // passa para funcao enviarespost os valores sobre o status da temperatura e fogao
    return;
  }
  if (request.indexOf("fogao") != -1) {
    if (request.indexOf("on") != -1) fogao = true;                                                                 // Liga fogao
    if (request.indexOf("off") != -1) fogao = false;                                                              // Desliga fogao
    enviaResposta(client, fogao, temperatura);                                                                //envia a nova resposta
    return;
  }
}

void enviaResposta(WiFiClient client, bool fog, float temp) { //funcao que rece status do cliente, estado fogao e valor temperatura
  // Monta corpo da resposta seguindo a estrutura -> [{Fogao:XX, Temperatura:ZZ}]
  String resposta = "[{Fogao:";
  if (fog) resposta = resposta + "ON";     //fog resposta recebe ON                                                                  
  else resposta = resposta + "OFF";        //resposta OFF
  resposta += "],{Temperatura:" + String(temperatura) + "}]"; //envia valor temperatura
  client.print(headerHTLM);                                                                                         //cabeçalho do html
  client.print(resposta);
  client.print(tailHTML);                                                                              //examina o que está sempre sendo atualizado
}

void atualizaSaidas(bool fog){  //atualiza a saída do fogão
  if (fog) {
    digitalWrite(FOGAO, 1);    
  }
  else
  {
    digitalWrite(FOGAO, 0);
  }
}
