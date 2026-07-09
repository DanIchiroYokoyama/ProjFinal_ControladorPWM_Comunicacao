# ProjFinal_ControladorPWM_Comunicacao
Daniel Ichiro P. Y. Pereira 15635969
Aljan Almeida Dias 15509165
Projeto Final de Aplicação de Microprocessadores

Na primeira parte do projeto, implementamos o controle de brilho de um LED RGB ligado à ESP32. Pra isso, usamos a biblioteca LEDC para gerenciar as cores (vermelho, verde e azul) em canais PWM independentes em uma frequência de 5 kHz e resolução de 8 bits. A lógica consistiu em mudar a intensidade luminosa (duty cycle de 0% a 100%) de forma contínua e aplicar taxas de incremento diferentes pra cada cor, o que gerou um efeito visual dinâmico. Todo o comportamento do sistema incluindo os valores exatos de brilho aplicados em cada canal foram printados via comunicação serial.

Na segunda parte, fizemos o controle de um servomotor utilizando também a ESP32. Um sensor mede a distância de um obstáculo (entre 10 e 50 cm) e a ESP32 mapeia essa leitura pra ajustar a posição do motor em tempo real (varia de 0° a 180°). Usamos a biblioteca nativa MCPWM e além disso configuramos o sinal PWM direto no hardware do microcontrolador e ajustamos a frequência e a largura de pulso em microssegundos. Isso torna o sistema mais eficiente e evita sobrecarregar o processador. Enquanto controla o motor, o sistema também exibe a distância e o ângulo no display OLED ligado via I2C e transmite esses dados para um celular por meio de uma conexão sem fio Bluetooth Low Energy (BLE). Todo o circuito e a lógica foram validados no simulador Wokwi.
