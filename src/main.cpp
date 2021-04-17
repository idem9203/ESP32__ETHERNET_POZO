#include <Arduino.h>
#include <stdio.h>
#include <WiFi.h>//https://www.arduino.cc/en/Reference/WiFi
#include <SPI.h>
#include "nRF24L01.h" //Libreria de modulo nRF24L01+
#include "RF24.h"

RF24 radio(4,5); /*Declaracion de los pines de control CE y CSN para el  modulo se define "radio" */

// Definicion de los canales de comunicacion
byte canal_E[5] = {'l', 'i', 'n', 'e', '1'}; /*Canal de Escritura del modulo Maestro*/
byte canal_L[5] = {'l', 'i', 'n', 'e', '2'}; /*Canal de Lectura del modulo Maestro*/
char fun[3] = {'S', 'R', 'C'};

//TaskHandle_t Task2;

//Variables para sensar la corriente TC
float iA;
unsigned int RUN;
unsigned int ESTADO;

//Parameters
String request ;
char ssid[10]  = "N DE WIFI";
char password[13]  = "CONTRASEÑA";
String nom  = "ESP32";
//Objects
WiFiServer server(3000);
WiFiClient client;

void peticion_status()
{
  radio.stopListening(); //Paramos la escucha para poder hablar
  Serial.print("Leyendo STATUS del sistema ");
  bool ok = radio.write(&fun[0], sizeof(fun[0])); //Envio de la funcion seleccionada
  if (ok) Serial.println("Ok...");
  else Serial.println("Failed");

  radio.startListening(); //Volvemos a la escucha
  unsigned long started_waiting_at = millis();
  bool timeout = false;

  while (! radio.available() && ! timeout) //Esperamos 500ms
  {
    if (millis() - started_waiting_at > 500) timeout = true;
  }

  if (timeout) Serial.println("Error, No ha habido respuesta a tiempo");
  else{
    //Leemos el mensaje recibido
    
    radio.read(&ESTADO, sizeof(unsigned int));
    
    if (ESTADO == 1) 
    {
      Serial.println("Motor ENCENDIDO, vaciando pozo...");
      RUN = 1;
    }
    if (ESTADO == 2) 
    {
      Serial.println("Motor bloqueado por sobrecorriente...");
      RUN = 0;
    }
    if (ESTADO == 3) 
    {
      Serial.println("Esperando el llenado del pozo...");
      RUN = 0;
    }
    if (ESTADO == 4) 
    {
      Serial.println("Motor ENCENDIDO, con sobre corriente 110% de su nominal...");
      RUN = 1;
    }
    if (ESTADO == 5) 
    {
      Serial.println("Motor ENCENDIDO, con sobre corriente 115% de su nominal...");
      RUN = 1;
    }
    if (ESTADO == 6) 
    {
      Serial.println("Motor ENCENDIDO, con sobre corriente 120% de su nominal...");
      RUN = 1;
    }
    if (ESTADO == 7) 
    {
      Serial.println("Motor ENCENDIDO, con sobre corriente 130% de su nominal...");
      RUN = 1;
    }
    
  }
}

void peticion_corriente ()
{
  radio.stopListening(); //Paramos la escucha para poder hablar
  Serial.print("Leyendo Corriente  ");
  bool ok = radio.write(&fun[2], sizeof(fun[2])); //Envio de la funcion seleccionada
  if (ok) Serial.println("ok...");
  else Serial.println("failed");

  radio.startListening();                      //Volvemos a la escucha
  unsigned long started_waiting_at = millis();
  bool timeout = false;

  while ( ! radio.available() && ! timeout ) // Esperamos 500ms
  {
    if (millis() - started_waiting_at > 500) timeout = true;
  }       
   
  if ( timeout ) Serial.println("Error, No ha habido respuesta a tiempo");
  else
  { // Leemos el mensaje recibido
    radio.read( &iA, sizeof(float)); //Funcion de lectura de datos
    Serial.print("Corriente = ");
    Serial.print(iA);
    Serial.println(" C");
  }
}

void peticion_encender ()
{
  radio.stopListening(); //Paramos la escucha para poder hablar
  Serial.print("ENCENDER MOTOR ");
  bool ok = radio.write(&fun[1], sizeof(&fun[1])); //Envio de la funcion seleccionada
  if (ok) Serial.println("Ok...");
  else Serial.println("Failed");

  radio.startListening(); //Volvemos a la escucha
  unsigned long started_waiting_at = millis();
  bool timeout = false;

  while (! radio.available() && ! timeout) //Esperamos 500ms
  {
    if (millis() - started_waiting_at > 500) timeout = true;
  }
      
  if (timeout) Serial.println("Error, No ha habido respuesta a tiempo");
  else{
    //Leemos el mensaje recibido
    radio.read(&RUN, sizeof(unsigned int));

    if (RUN == 1) Serial.println("MOTOR ENCENDIDO");
    else Serial.println("Intenta encender nuevamente");
  }
}

void handleRequest(String request)
{ /* function handleRequest */ 
  ////Handle web client request
  String pwmCmd;
  //Digital Ouputs
  if (request.indexOf("/encender") > 0) 
  {
    peticion_encender();
    delay(3500);
  }
  if (request.indexOf("/") > 0);
}

void webpage(WiFiClient client)
{ /* function webpage */ 
  ////Send webpage to client
  peticion_status();
  peticion_corriente();
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<title> IDEM_ESP32 </title>");
  client.println("<meta http-equiv='content-type' content='text/html; charset=UTF-8'>");
  client.println("<meta name='apple-mobile-web-app-capable' content='yes' />");
  client.println("<meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' />");
  client.println("<meta http-equiv='refresh' content='5'>");
  client.println("</head>");
  client.println("<body bgcolor = '#FFFFFF'> ");
  client.println("<hr/>");
  client.println("<h1 style='color:#3AAA35;'><center>BOMBA DE POZO - "+nom+" Web Controller</center></h1>");
  client.println("<hr/>");
  client.println("<center> <h2> PETICIONES: </h2>");
  client.println("  STATUS: ");
  if (ESTADO == 1) client.println("  <input type = \"text\" size=\"50\" value= \"Motor ENCENDIDO, vaciando pozo...\" readonly></input>");
  if (ESTADO == 2) client.println("  <input type = \"text\" size=\"50\" value= \"Motor bloqueado por sobrecorriente...\" readonly></input>");
  if (ESTADO == 3) client.println("  <input type = \"text\" size=\"50\" value= \"Esperando el llenado del pozo...\" readonly></input>");
  if (ESTADO == 4) client.println("  <input type = \"text\" size=\"50\" value= \"Motor ENCENDIDO, con sobre corriente 110% de su nominal...\" readonlyy></input>");
  if (ESTADO == 5) client.println("  <input type = \"text\" size=\"50\" value= \"Motor ENCENDIDO, con sobre corriente 115% de su nominal...\" readonly></input>");
  if (ESTADO == 6) client.println("  <input type = \"text\" size=\"50\" value= \"Motor ENCENDIDO, con sobre corriente 120% de su nominal...\" readonly></input>");
  if (ESTADO == 7) client.println("  <input type = \"text\" size=\"50\" value= \"Motor ENCENDIDO, con sobre corriente 130% de su nominal...\" readonly></input>");
  //client.println("  <a href='/status'><button>STATUS</button></a>");
  client.println("<br>");
  client.println("<hr/>");  
  client.println("VALOR DE CORRIENTE: ");
  client.println("  <input value=" + String(iA) + "A" + " readonly></input>");
  //client.println("  <a href='/corriente'><button>CORRIENTE</button></a>");
  client.println("<hr/>");
  client.println("  <a href='/encender'><button>RUN</button></a>");
  client.println("  </center><center>");
  client.println("<table>");
  client.println("<tr>");
  if (RUN == 1)
  {
    client.println("   <td>MOTOR ENCENDIDO</td>");
  }else
  {
    client.println("<td>MOTOR APAGADO</td>    ");
  }
  client.println("</tr>");
  client.println("<cennter></table><br><a href='/'><button>FINALIZAR</button></a><br /></center> ");
  client.println("<hr/></center></body></html>");

  delay(1);
}

/*
void loop2(void *parameter)
{

}
*/

void setup()
{
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  delay(100);

  radio.begin();
  radio.setRetries(15, 15); //Maximos reintentos
  //radio.setPayloadSize(8); //Reduce el paylooad de 32 si tienes problemas
  radio.openWritingPipe(canal_E);
  radio.openReadingPipe(1, canal_L);

  //Multitarea
  /*
  xTaskCreatePinnedToCore(      //Tarea que apunta a un nucleo en particular
    loop2,                      //Funcion que contiene bucle infinito o codigo a ejecutar
    "Task_2",                   //Nombre de la tarea que queremos darle
    1000,                       //Tamaño de la pila
    NULL,                       //Parametros que le querramos pasar
    1,                          //Prioridad
    &Task2,                     //Nombre de la tarea
    0                           //Nucleo en el que se ejecutara
  );
  */

  //Init Serial USB
  Serial.begin(115200);

  Serial.println(F("Initialize System"));
  //Init ESP32Wifi
  Serial.print("Connecting to ");Serial.println(ssid);
  WiFi.begin(ssid, password);
  // Connect to Wifi network.
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);Serial.print(F("."));
  }
  server.begin();
  Serial.println();
  Serial.println(F("ESP32Wifi initialized"));
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());

  Serial.println("Sistema de comunicacion con nRF24L01");
  Serial.println("Digite la letra de la variable a consultar");
  Serial.println("\"S\"-STATUS,\"R\"-ENCENDER, \"C\"-CORRIENTE");
}

void loop()
{
  WiFiClient client = server.available();
  if (client) 
  {
    while(client.connected())
    {
      if (client.available()) 
      {
        String request = client.readStringUntil('\r');
        Serial.println(request);
        handleRequest(request);
      }
      webpage(client);//Return webpage
      break;
   }
  }
}
