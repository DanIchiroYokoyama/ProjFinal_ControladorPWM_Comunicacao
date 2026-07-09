#include <Arduino.h>
// Inclusão da biblioteca NATIVA do ESP-IDF para controle do MCPWM, exigência do projeto
#include "driver/mcpwm.h" 

// Bibliotecas estritamente necessárias para OLED e BLE
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Pinos
#define TRIG_PIN 5
#define ECHO_PIN 18
#define SERVO_PIN 26 // Pino do sinal PWM pro Servo

// Servomotor
#define SERVO_MIN_PULSEWIDTH_US 500 // Pulso p 0 graus
#define SERVO_MAX_PULSEWIDTH_US 2500 // Pulso p 180 graus

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Bluetooth
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
// configuração do servo
void setup_mcpwm() {
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, SERVO_PIN);

  // onfiguração do PWM
  mcpwm_config_t pwm_config;
  pwm_config.frequency = 50; // Frequência de 50 Hz
  pwm_config.cmpr_a = 0;     
  pwm_config.cmpr_b = 0;     
  pwm_config.counter_mode = MCPWM_UP_COUNTER; 
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;

  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
}

void setup() {
  Serial.begin(115200); 

  // configuração do HC-SR04
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  setup_mcpwm();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println(F("Erro ao iniciar o OLED"));
      for(;;); 
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);

  BLEDevice::init("ESP32_ServoBLE");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  pServer->getAdvertising()->start();
}

void loop() {
  // Leitura do Sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.0343 / 2; // Converte para cm

  // Lógica de controle do servomotor
  float dist_limitada = distance;
  if (dist_limitada < 10.0) dist_limitada = 10.0;
  if (dist_limitada > 50.0) dist_limitada = 50.0;

  // distância pro ângulo do servo
  int angulo = map(dist_limitada, 10, 50, 0, 180);
  
  // ângulo para a largura de pulso
  uint32_t pulse_width = map(angulo, 0, 180, SERVO_MIN_PULSEWIDTH_US, SERVO_MAX_PULSEWIDTH_US);

  mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, pulse_width);

  // Monitoramento serial UART
  Serial.print("Distancia: "); 
  Serial.print(distance); 
  Serial.print(" cm | Angulo: "); 
  Serial.print(angulo); 
  Serial.println(" graus");

  // Atualização do display
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

  // Envio por bluetooth
  if (deviceConnected) {
    char ble_msg[50];
    snprintf(ble_msg, sizeof(ble_msg), "Dist: %.1fcm Ang: %d gra", distance, angulo);
    pCharacteristic->setValue(ble_msg);
    pCharacteristic->notify();
  }
  delay(100);
}