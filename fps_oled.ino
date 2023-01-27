#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "bazinga";
const char* password = "mynameisjamesandilovetosleep";
String server = "192.168.0.70";
String message = "";
WiFiClient client;

#define OLED_MOSI 13 
#define OLED_CLK  14  
#define OLED_CS   15
#define OLED_RST   4 
#define OLED_DC    5


// Create the OLED display
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64,OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);

void setup()   {
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());  

  display.begin(0, true); // we dont use the i2c address but we will reset!
  display.clearDisplay();
  display.setRotation(2);
  display.display();

  Serial.println("Starting connection...");
  if (client.connect(server, 80)) {
    Serial.println("connected");
    client.println("GET /sse HTTP/1.0");
    client.println();
  } 
}

void loop() {
  if (!client.connected()) {
    client.stop();
    Serial.println("WIFI disconnected, reconnecting");
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println("WIFI DISCO");
    display.println("RECONNECTING");
    display.display();
    client.stop();

    if (client.connect(server, 80)) {
      Serial.println("connected");
      client.println("GET /sse HTTP/1.0");
      client.println();
    } 
  }
  
  while (client.available() > 0) {
    char c = client.read();
    message += c;  
  }

  if (message.endsWith("\n\n")) {
//    Serial.println("Whole message:");
//    Serial.println(message);
    
    int start_index, end_index;
    int fps, gpu_util;
    char temp[3];   
    int cpu_util[12]; 
    
    start_index = message.indexOf("RTSS FPS") + 9;
    end_index = message.indexOf("{",start_index);
    fps = message.substring(start_index, end_index).toInt();
    Serial.print("FPS: ");
    Serial.println(fps);    
     
       
    start_index = message.indexOf("GPU Utilization") + 16;
    end_index = message.indexOf("{",start_index);
    gpu_util = message.substring(start_index, end_index).toInt();
    if (gpu_util >= 100) gpu_util = 99;
    Serial.print("GPU: ");
    Serial.println(gpu_util);    

    for (int i = 1; i < 13; i++) {
      char search[50];
      sprintf(search, "CPU%d Utilization",i); 
      start_index = message.indexOf(search) + strlen(search) + 1;
      end_index = message.indexOf("{",start_index);
      cpu_util[i-1] = message.substring(start_index, end_index).toInt();
      Serial.printf("CPU%d:%d\n", i, cpu_util[i-1]);
    }
    
    display.clearDisplay();
//    display.drawFastVLine(73,0,24,SH110X_WHITE);
    display.drawFastHLine(0,24,128,SH110X_WHITE);
    display.drawFastHLine(0,63,128,SH110X_WHITE);
    display.setTextSize(3);
    display.setTextWrap(false);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.print("F");
    sprintf(temp, "%3d", fps);
    display.print(temp);
    display.setCursor(78, 0);
    display.print("G");
    sprintf(temp, "%2d", gpu_util);
    display.println(temp);

    for (int i = 0; i < 12; i++) {
      display.fillRect(5 + i*10, 24 + (40-round(cpu_util[i] / 2.5)), 9, round(cpu_util[i] / 2.5),SH110X_WHITE);
    }    
    
    display.display();
    
    
    message = "";
  }
}
