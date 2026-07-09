# ProjFinal_ControladorPWM_Comunicacao
Daniel Ichiro P. Y. Pereira 15635969
Aljan Almeida Dias 15509165
Projeto Final de Aplicação de Microprocessadores

Na primeira parte do projeto, implementamos o controle de brilho de um LED RGB ligado à ESP32. Pra isso, usamos a biblioteca LEDC para gerenciar as cores (vermelho, verde e azul) em canais PWM independentes em uma frequência de 5 kHz e resolução de 8 bits. A lógica consistiu em mudar a intensidade luminosa (duty cycle de 0% a 100%) de forma contínua e aplicar taxas de incremento diferentes pra cada cor, o que gerou um efeito visual dinâmico. Todo o comportamento do sistema incluindo os valores exatos de brilho aplicados em cada canal foram printados via comunicação serial.

Na segunda parte, fizemos o controle de um servomotor utilizando também a ESP32. Um sensor mede a distância de um obstáculo (entre 10 e 50 cm) e a ESP32 mapeia essa leitura pra ajustar a posição do motor em tempo real (varia de 0° a 180°). Usamos a biblioteca nativa MCPWM e além disso configuramos o sinal PWM direto no hardware do microcontrolador e ajustamos a frequência e a largura de pulso em microssegundos. Isso torna o sistema mais eficiente e evita sobrecarregar o processador. Enquanto controla o motor, o sistema também exibe a distância e o ângulo no display OLED ligado via I2C e transmite esses dados para um celular por meio de uma conexão sem fio Bluetooth Low Energy (BLE). Todo o circuito e a lógica foram validados no simulador Wokwi.

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
// Limpa o buffer de memória da tela anterior
  display.clearDisplay();
  
  // Cabeçalho
  display.setCursor(0,0);
  display.println(" Controle Motor (MCPWM)");
  display.drawLine(0, 12, 128, 12, WHITE); // Desenha uma linha separadora
  
  // Exibição dos parâmetros de funcionamento (Sensores e PWM)
  display.setCursor(0, 20);
  display.print("Dist : "); 
  display.print(distance, 1); // Imprime a distância com 1 casa decimal
  display.println(" cm");
  
  display.setCursor(0, 35);
  display.print("PWM  : "); 
  display.print(duty_cycle, 1); 
  display.println(" %");
  
  // Status do sistema (Comunicação)
  display.setCursor(0, 50);
  display.print("BLE  : "); 
  display.println(deviceConnected ? "Conectado" : "Desconectado");
  
  // Envia todo o buffer montado para o display físico via I2C
  display.display();
(Este bloco cria a interface visual do sistema e exibe os parâmetros em tempo real. Ele consiste em limpar a tela anterior, posicionar o cursor em coordenadas específicas e escrever os textos com os valores atualizados de distância e potência, além do status da conexão bluetooth.
