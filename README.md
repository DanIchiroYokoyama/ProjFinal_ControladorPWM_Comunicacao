# ProjFinal_ControladorPWM_Comunicacao
Daniel Ichiro P. Y. Pereira 15635969
Aljan Almeida Dias 15509165
Projeto Final de Aplicação de Microprocessadores

Na primeira parte do projeto, implementamos o controle de brilho de um LED RGB ligado à ESP32. Pra isso, usamos a biblioteca LEDC para gerenciar as cores (vermelho, verde e azul) em canais PWM independentes em uma frequência de 5 kHz e resolução de 8 bits. A lógica consistiu em mudar a intensidade luminosa (duty cycle de 0% a 100%) de forma contínua e aplicar taxas de incremento diferentes pra cada cor, o que gerou um efeito visual dinâmico. Todo o comportamento do sistema incluindo os valores exatos de brilho aplicados em cada canal foram printados via comunicação serial.

Na segunda parte, fizemos o controle de um servomotor usando também a ESP32. Um sensor mede a distância de um objeto e o código limita essa leitura num intervalo. A ESP32 mapeia essa distância para definir o ângulo do motor e converte esse ângulo na largura de pulso correspondente. Usamos a biblioteca nativa MCPWM pra configurar o sinal PWM direto no hardware do microcontrolador numa frequência de 50 Hz o que evita sobrecarregar o processador. Enquanto controla o motor, o sistema também exibe a distância atual e o ângulo no display OLED ligado por I2C. A comunicação sem fio via Bluetooth Low Energy (BLE) chegou a ser criada pra transmitir os dados pro celular mas como não deu certo a conexão foi removida no código final e o display só mostra "Desconectado".

BIBLIOTECAS:

driver/mcpwm.h: Usamos pro controle do sinal PWM por hardware, pra definir a frequência em 50Hz e pra ajustar a largura de pulso pro controle de posição do servomotor.

Wire.h, Adafruit_GFX.h e Adafruit_SSD1306.h: Usamos em conjunto pra comunicação por I2C e textos no display OLED.

BLEDevice.h, BLEServer.h, BLEUtils.h e BLE2902.h: Bibliotecas baseadas no Arduino pra ESP32, usamos para o Bluetooth LE transmitir notificações dos dados dos sensores.

PRINCIPAIS TRECHOS:

Parte 1:

if (distance < 10.0) {
      duty_cycle = 0.0; // Muito próximo: para o motor
  } else if (distance > 50.0) {
      duty_cycle = 100.0; // Caminho livre: velocidade máxima
  } else {
      // Mapeia a distância linearmente para o PWM
      duty_cycle = map(distance, 10, 50, 20, 100); 
  }
  
(Dentro do loop, o sistema lê o pulso do sensor e converte pra centímetros. Depois usamos a função map pra traduzir a faixa de distância em uma faixa de potência. Se o obstáculo tiver muito perto, o motor desliga por segurança.)

if (deviceConnected) {
      char ble_msg[50];
      snprintf(ble_msg, sizeof(ble_msg), "Dist: %.1fcm PWM: %.1f%%", distance, duty_cycle);
      pCharacteristic->setValue(ble_msg);
      pCharacteristic->notify(); // Dispara o dado via BLE
  }
  
(Sempre que um celular está conectado (deviceConnected == true) o código formata a distância e a potência do motor em texto e usa o comando notify para empurrar essa atualização para o aplicativo do usuário.)

Parte 2:

// Limita a distância e mapeia para o respectivo ângulo e pulso do servo
float dist_limitada = distance;
if (dist_limitada < 10.0) dist_limitada = 10.0;
if (dist_limitada > 50.0) dist_limitada = 50.0;

int angulo = map(dist_limitada, 10, 50, 0, 180);
uint32_t pulse_width = map(angulo, 0, 180, SERVO_MIN_PULSEWIDTH_US, SERVO_MAX_PULSEWIDTH_US);

// Ajusta o hardware do MCPWM com a largura de pulso em microssegundos
mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, pulse_width);

(Faz a limitação e a conversão dos valores de distância ângulo e largura para o display. Então aplica direto no canal do MCPWM que controla o motor.)

Serial.println("Configurando BLE (Delay + Teste)");
// sessão não funcionou /*   
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
pServer->getAdvertising()->start(); */

(Tentativa de implementação da BLE, o que resultou em um loop infinito no display. Esse padrão de "watchdog disparado pelo btController" é um problema bem documentado e normalmente aparece quando a stack é inicializada de forma pesada sem deixar o agendador do FreeRTOS estabilizar, isso é de acordo com as discussões já feitas no site StackOverflow.)
