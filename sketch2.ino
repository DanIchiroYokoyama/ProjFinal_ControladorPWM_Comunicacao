// SEL0433 - Aplicação de Microprocessadores
// Trabalho Final - Controle de Servomotor via Sensor com Display OLED
// Daniel Ichiro P. Y. Pereira - 15635969
// Aljan Almeida Dias - 15509165

#include <Arduino.h>
#include "driver/mcpwm.h" // driver da ESP32 para MCPWM (Motor Control PWM)

#include <Wire.h> // biblioteca de comunicação I2C
#include <Adafruit_GFX.h> // biblioteca do driver do OLED
#include <Adafruit_SSD1306.h> // driver do display OLED
#include <BLEDevice.h> // BLE da ESP32
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// definição dos pinos usados
#define TRIG_PIN 5 // pino que dispara o pulso ultrassônico do HC-SR04
#define ECHO_PIN 18 // pino que recebe o eco (mede o tempo de retorno do som)
#define SERVO_PIN 26 // pino de sinal PWM que vai para o servomotor

// faixa de pulso do servo
#define SERVO_MIN_PULSEWIDTH_US 500   // pulso de 500us  -> corresponde a 0 graus
#define SERVO_MAX_PULSEWIDTH_US 2500  // pulso de 2500us -> corresponde a 180 graus

// configuração do display OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// Cria o display com o barramento I2C e sem pino de reset dedicado
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// não funcionou
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks{
  void onConnect(BLEServer* pServer){
    deviceConnected = true;
  }
  void onDisconnect(BLEServer* pServer){
    deviceConnected = false;
    pServer->getAdvertising()->start();
  }
};

// configuração do MCPWM para gerar o sinal PWM do servo
void setup_mcpwm(){
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, SERVO_PIN);

  mcpwm_config_t pwm_config;
  pwm_config.frequency = 50; // 50Hz -> período de 20ms, padrão para servos analógicos
  pwm_config.cmpr_a = 0;
  pwm_config.cmpr_b = 0;
  pwm_config.counter_mode = MCPWM_UP_COUNTER; // contador conta de 0 até o topo e reinicia
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0; // 0 = sinal não invertido (nível alto = "pulso ativo")

  // aplica a configuração no timer 0
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
}

void setup(){
  Serial.begin(115200); // inicia a comunicação serial 
  delay(500);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("Configurando MCPWM (Delay)");
  setup_mcpwm(); // configura o PWM do servo
  delay(100);

  Serial.println("Configurando Display (Delay)");
  Wire.begin(21, 22); // inicia o barramento I2C
  // se não der certo trava aqui de propósito
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
      Serial.println(F("Erro no OLED"));
      while(true) { delay(10); }
  }
  display.clearDisplay(); // limpa o buffer de vídeo
  display.setTextColor(WHITE); // cor do texto
  display.setTextSize(1); // tamanho da fonte
  display.setCursor(0,0);
  display.display(); // envia o buffer para a tela
  delay(100);

  Serial.println("Configurando BLE (Delay + Teste)");
  // Bloco BLE comentado propositalmente: causava reinicializações por watchdog
  // (task "btController" travando o núcleo 0). Fica desativado até ser revisado
  // com folgas (delay) entre cada etapa de inicialização do BLE.
  /* BLEDevice::init("ESP32_ServoBLE");
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

void loop(){
  // disparo do pulso
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2); // garante nível baixo estável antes do pulso
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10); // pulso de disparo de 10us
  digitalWrite(TRIG_PIN, LOW);

  // mede quanto tempo o pino ECHO fica em nível alto (tempo de ida e volta do som).
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  float distance = 0;
  if(duration == 0){
      distance = 50.0; // sem eco detectado -> assume distância máxima
  } 
  else{
      distance = duration * 0.0343 / 2; // (tempo * velocidade do som) / 2
      // (divide por 2 porque o tempo medido é ida + volta)
  }

  // limite da distância até 10 a 50 cm
  float dist_limitada = distance;
  if (dist_limitada < 10.0) dist_limitada = 10.0;
  if (dist_limitada > 50.0) dist_limitada = 50.0;

  // converte distância em ângulo
  int angulo = map(dist_limitada, 10, 50, 0, 180);
  // converte ângulo em largura de pulso
  uint32_t pulse_width = map(angulo, 0, 180, SERVO_MIN_PULSEWIDTH_US, SERVO_MAX_PULSEWIDTH_US);

  mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, pulse_width);

  Serial.print("Distancia: ");
  Serial.print(distance);
  Serial.print(" cm | Angulo: ");
  Serial.print(angulo);
  Serial.println(" graus");

  // atualiza o OLED
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

  delay(100); // delay pra estabilização
}
