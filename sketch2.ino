// SEL0433 - Aplicação de Microprocessadores
// Trabalho Final - Controle de Servomotor via Sensor com Display OLED
// Daniel Ichiro P. Y. Pereira - 15635969
// Aljan Almeida Dias - 15509165

#include <Arduino.h>
#include "driver/mcpwm.h" 

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define TRIG_PIN 5
#define ECHO_PIN 18
#define SERVO_PIN 26 

#define SERVO_MIN_PULSEWIDTH_US 500 
#define SERVO_MAX_PULSEWIDTH_US 2500 

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    pServer->getAdvertising()->start(); 
  }
};

void setup_mcpwm() {
  mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM0A, SERVO_PIN);
  mcpwm_config_t pwm_config;
  pwm_config.frequency = 50; 
  pwm_config.cmpr_a = 0;     
  pwm_config.cmpr_b = 0;     
  pwm_config.counter_mode = MCPWM_UP_COUNTER; 
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_0, &pwm_config);
}

void setup() {
  Serial.begin(115200); 
  delay(500);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("Configurando MCPWM (Delay)");
  setup_mcpwm();
  delay(100); 

  Serial.println("Configurando Display (Delay)");
  Wire.begin(21, 22);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println(F("Erro no OLED"));
      while(true) { delay(10); }
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.display();
  delay(100);

  Serial.println("Configurando BLE (Delay + Teste)");
  // sessão não funcionou
/*   BLEDevice::init("ESP32_ServoBLE");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  pServer->getAdvertising()->start(); */
  
  Serial.println("Funcionando");
}

void loop() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); 
  
  float distance = 0;
  if(duration == 0) {
      distance = 50.0; 
  } else {
      distance = duration * 0.0343 / 2; 
  }

  float dist_limitada = distance;
  if (dist_limitada < 10.0) dist_limitada = 10.0;
  if (dist_limitada > 50.0) dist_limitada = 50.0;

  int angulo = map(dist_limitada, 10, 50, 0, 180);
  uint32_t pulse_width = map(angulo, 0, 180, SERVO_MIN_PULSEWIDTH_US, SERVO_MAX_PULSEWIDTH_US);

  mcpwm_set_duty_in_us(MCPWM_UNIT_1, MCPWM_TIMER_0, MCPWM_OPR_A, pulse_width);

  Serial.print("Distancia: "); 
  Serial.print(distance); 
  Serial.print(" cm | Angulo: "); 
  Serial.print(angulo); 
  Serial.println(" graus");

  display.clearDisplay();
  display.setCursor(0,0);
  display.println(" Controle Servo(MCPWM)");
  display.drawLine(0, 12, 128, 12, WHITE);
  
  display.setCursor(0, 20);
  display.print("Dist  : "); 
  display.print(distance, 1); 
  display.println(" cm");
  
  display.setCursor(0, 35);
  display.print("Angulo: "); 
  display.print(angulo); 
  display.println(" graus");
  
  display.setCursor(0, 50);
  display.print("BLE   : "); 
  display.println(deviceConnected ? "Conectado" : "Desconectado");
  display.display();

  if (deviceConnected) {
    char ble_msg[50];
    snprintf(ble_msg, sizeof(ble_msg), "Dist: %.1fcm Ang: %d gra", distance, angulo);
    pCharacteristic->setValue(ble_msg);
    pCharacteristic->notify();
  }
  delay(100); 
}
