#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>

// ==========================================
// 1. WI-FI & MQTT SETTINGS
// ==========================================
const char* ssid = "Fibre home 2";
const char* password = "Hajar@ahmed1024";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

// ==========================================
// 2. HARDWARE PINS
// ==========================================
#define DHT_PIN 4
#define DHT_TYPE DHT11
#define MQ2_PIN 34
#define RAIN_PIN 35
#define PIR_PIN 14
#define DOOR_PIN 27
#define WINDOW_PIN 26
#define BUZZER_PIN 12

// LED Pins for the Rooms
#define KITCHEN_PIN 13
#define LIVING_ROOM_PIN 32
#define BEDROOM_PIN 33

// RFID Pins
#define SS_PIN 5
#define RST_PIN 2

// OLED Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// ==========================================
// 3. OBJECTS & GLOBAL STATE
// ==========================================
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MFRC522 rfid(SS_PIN, RST_PIN);

enum SystemMode { HOME, AWAY };
SystemMode currentMode = HOME; 

float currentTemp = 0.0;
float currentHum = 0.0;
String systemStatus = "System Normal";

// ==========================================
// 4. MQTT CALLBACK
// ==========================================
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  String topicStr = String(topic);

  if (topicStr == "dari/control/mode") {
    if (message == "HOME") { 
      currentMode = HOME; 
      systemStatus = "Mode: HOME"; 
      digitalWrite(KITCHEN_PIN, HIGH);
      digitalWrite(LIVING_ROOM_PIN, HIGH);
      digitalWrite(BEDROOM_PIN, HIGH);
    } 
    else if (message == "AWAY") { 
      currentMode = AWAY; 
      systemStatus = "Mode: AWAY"; 
      digitalWrite(KITCHEN_PIN, LOW);
      digitalWrite(LIVING_ROOM_PIN, LOW);
      digitalWrite(BEDROOM_PIN, LOW);
    }
  } 
  else if (topicStr == "dari/control/lights/kitchen") {
    if (message == "ON") digitalWrite(KITCHEN_PIN, HIGH);
    else digitalWrite(KITCHEN_PIN, LOW);
  }
  else if (topicStr == "dari/control/lights/living_room") {
    if (message == "ON") digitalWrite(LIVING_ROOM_PIN, HIGH);
    else digitalWrite(LIVING_ROOM_PIN, LOW);
  }
  else if (topicStr == "dari/control/lights/bedroom") {
    if (message == "ON") digitalWrite(BEDROOM_PIN, HIGH);
    else digitalWrite(BEDROOM_PIN, LOW);
  }
}

// ==========================================
// 5. FREERTOS TASKS
// ==========================================

void TaskSafety(void *pvParameters) {
  static bool rainAlertSent = false; 
  for (;;) {
    int gasLevel = analogRead(MQ2_PIN);
    int rainLevel = analogRead(RAIN_PIN);
    if (gasLevel > 2000) { 
      digitalWrite(BUZZER_PIN, HIGH);
      systemStatus = "GAS DETECTED!";
      client.publish("dari/alerts", "GAS LEAK DETECTED!");
    } 
    else if (rainLevel < 2500) { 
      systemStatus = "Raining Outside";
      digitalWrite(BUZZER_PIN, LOW); 
      if (!rainAlertSent) {
        client.publish("dari/alerts", "It's raining! Hide your plants.");
        client.publish("dari/sensors/rain", "RAIN_START"); 
        rainAlertSent = true;
      }
    } 
    else {
      digitalWrite(BUZZER_PIN, LOW);
      rainAlertSent = false; 
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
  }
}

void TaskSecurity(void *pvParameters) {
  for (;;) {
    if (currentMode == AWAY) {
      bool motion = digitalRead(PIR_PIN) == HIGH;
      
      // FLIPPED LOGIC: Magnet near (LOW) now triggers the OPEN alert
      bool doorOpened = (digitalRead(DOOR_PIN) == LOW);
      bool windowOpened = (digitalRead(WINDOW_PIN) == LOW);
      
      if (motion || doorOpened || windowOpened) {
        digitalWrite(BUZZER_PIN, HIGH);
        systemStatus = "INTRUDER ALERT!";
        if (motion) client.publish("dari/alerts", "MOTION DETECTED!");
        if (doorOpened) client.publish("dari/alerts", "DOOR OPENED!");
        if (windowOpened) client.publish("dari/alerts", "WINDOW OPENED!");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
      } else {
        digitalWrite(BUZZER_PIN, LOW);
        systemStatus = "System Armed (AWAY)";
      }
    }
    vTaskDelay(500 / portTICK_PERIOD_MS); 
  }
}

void TaskTelemetry(void *pvParameters) {
  for (;;) {
    if (client.connected()) {
      client.publish("dari/sensors/temp", String(dht.readTemperature(), 1).c_str());
      client.publish("dari/sensors/hum", String(dht.readHumidity(), 1).c_str());
      client.publish("dari/sensors/gas", String(analogRead(MQ2_PIN)).c_str());
      
      // FLIPPED LOGIC: Magnet near (LOW) publishes "OPEN"
      client.publish("dari/sensors/door", digitalRead(DOOR_PIN) == LOW ? "OPEN" : "CLOSED");
      client.publish("dari/sensors/window", digitalRead(WINDOW_PIN) == LOW ? "OPEN" : "CLOSED");
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS); // Broadcast faster for better responsiveness
  }
}

void TaskDisplay(void *pvParameters) {
  for (;;) {
    currentTemp = dht.readTemperature();
    currentHum = dht.readHumidity();
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Line 1: Header
    display.setCursor(15, 0);
    display.print("DARI BIN YEDI");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    // Line 2: Mode
    display.setCursor(0, 15);
    display.print("Mode: ");
    display.print(currentMode == HOME ? "HOME" : "AWAY");
    
    // Line 3: Temp & Humidity side-by-side
    display.setCursor(0, 25);
    if (!isnan(currentTemp)) {
      display.print("T:"); display.print(currentTemp, 1); display.print("C  ");
      display.print("H:"); display.print(currentHum, 1); display.print("%");
    }
    
    // Line 4: Door & Window status side-by-side
    display.setCursor(0, 37);
    display.print("Door:");
    display.print(digitalRead(DOOR_PIN) == LOW ? "OPEN " : "CLS  ");
    display.print("Win:");
    display.print(digitalRead(WINDOW_PIN) == LOW ? "OPEN" : "CLS");

    // Line 5: System Alerts
    display.setCursor(0, 52);
    display.print(systemStatus);
    
    display.display();
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Screen refreshes every 1 second now!
  }
}

// ==========================================
// 6. SETUP & MAIN LOOP
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(1000); 
  
  Serial.println("\n\n--- Dari Bin Yedi System Booting ---");
  
  pinMode(MQ2_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(DOOR_PIN, INPUT_PULLUP);
  pinMode(WINDOW_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(KITCHEN_PIN, OUTPUT);
  pinMode(LIVING_ROOM_PIN, OUTPUT);
  pinMode(BEDROOM_PIN, OUTPUT);

  dht.begin();
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("ERROR: SSD1306 OLED allocation failed"));
  } else {
    Serial.println(F("OLED Display Initialized Successfully!"));
  }

  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  Serial.println("\nWi-Fi Connected Successfully!");
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  xTaskCreate(TaskSafety, "Safety", 4096, NULL, 2, NULL);
  xTaskCreate(TaskSecurity, "Security", 4096, NULL, 2, NULL);
  xTaskCreate(TaskTelemetry, "Telemetry", 4096, NULL, 1, NULL); 
  xTaskCreate(TaskDisplay, "Display", 4096, NULL, 1, NULL);
  
  Serial.println("System Fully Armed and Ready (RFID Disabled)!");
}

void loop() {
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // CRITICAL FIX: Randomize the ID so the server doesn't block us!
    String clientId = "DariClient-" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("dari/control/mode");
      client.subscribe("dari/control/lights/kitchen");
      client.subscribe("dari/control/lights/living_room");
      client.subscribe("dari/control/lights/bedroom");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      
      // CRITICAL FIX: Wait 5 seconds before retrying to prevent crashing the ESP32!
      delay(5000); 
    }
  } else {
    client.loop(); // Only loop if actually connected
  }
}