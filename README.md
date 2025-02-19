# README - Controle de LEDs e Display com Joystick no RP2040

## Objetivos
Este projeto tem como objetivos principais:
- Compreender o funcionamento do conversor analógico-digital (ADC) no RP2040.
- Utilizar o PWM para controlar a intensidade de dois LEDs RGB com base nos valores do joystick.
- Representar a posição do joystick no display SSD1306 por meio de um quadrado móvel.
- Aplicar o protocolo de comunicação I2C na integração com o display.

## Descrição do Projeto

O joystick fornecerá valores analógicos correspondentes aos eixos X e Y, que serão utilizados para:

### Controle dos LEDs RGB:
- **LED Azul:** O brilho será ajustado conforme o valor do eixo Y.
  - Quando o joystick estiver na posição central (valor 2048), o LED permanecerá apagado.
  - Movendo para cima (valores menores) ou para baixo (valores maiores), o LED aumentará o brilho gradualmente, atingindo a intensidade máxima nos extremos (0 e 4095).
- **LED Vermelho:** O mesmo princípio, mas de acordo com o eixo X.
  - Posição central (valor 2048) mantém o LED apagado.
  - Movendo para esquerda (valores menores) ou direita (valores maiores), o LED aumentará o brilho, atingindo a intensidade máxima nos extremos (0 e 4095).
- Os LEDs serão controlados via PWM para permitir uma variação suave da intensidade luminosa.

### Controle do Display SSD1306:
- Exibir um quadrado de 8x8 pixels, inicialmente centralizado, que se moverá proporcionalmente aos valores capturados pelo joystick.

### Funcionalidade dos Botões:
- **Botão do Joystick (GPIO 22):**
  - Alternar o estado do LED Verde a cada acionamento.
  - Modificar a borda do display para indicar que foi pressionado, alternando entre diferentes estilos de borda a cada novo acionamento.
- **Botão A (GPIO 5):**
  - Ativar ou desativar os LEDs PWM a cada acionamento.

## Componentes Utilizados

Os seguintes componentes estão interconectados à placa BitDogLab:
- **LED RGB:** conectado às GPIOs 11 (verde), 12 (azul) e 13 (vermelho).
- **Botão do Joystick:** conectado à GPIO 22.
- **Joystick:** conectado às GPIOs 26 (eixo Y) e 27 (eixo X).
- **Botão A:** conectado à GPIO 5.
- **Display SSD1306:** conectado via I2C (GPIO 14 - SDA e GPIO 15 - SCL).

## Como Compilar e Executar

1. Clone o repositório:
   ```bash
   git clone <URL_DO_REPOSITORIO>
   cd <NOME_DO_DIRETORIO>
   ```
2. Compile o código utilizando o SDK do RP2040:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```
3. Carregue o firmware na placa via USB.
4. Monitore a execução via porta serial.

## Vídeo
[Vídeo explicando e demonstrando na placa](https://www.canva.com/design/DAGfgBoLlAk/klZqxNQcpXn_Grlj61znwA/edit?utm_content=DAGfgBoLlAk&utm_campaign=designshare&utm_medium=link2&utm_source=sharebutton)
