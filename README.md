# Belt-Tensioner-Robot-Servo

Sistema de tensionamento de cintos para sim racing usando servos de robótica de alto torque e SimHub.

Baseado no projeto original de **blekenbleu**:  
- Descrição do dispositivo serial customizado:  
  https://blekenbleu.github.io/Arduino/SimHubCustomSerial  
- Sketch original (Blue Pill):  
  https://github.com/blekenbleu/blekenbleu.github.io/tree/master/Arduino/Blue_ASCII_Servo  
- Perfil SimHub original:  
  https://blekenbleu.github.io/Arduino/proxy_G.shsds.txt  

Demonstração em vídeo do meu tensionador:  
- https://www.youtube.com/watch?v=9a0rFGwfBp4  

Modelos 3D:  
- Rolos do tensionador: https://www.thingiverse.com/thing:5490048  
- Caixa para supercaps + Arduino: https://www.thingiverse.com/thing:5490068  

---

## Visão Geral

Este projeto usa:

- **Arduino Nano** como controlador.
- **Dois servos de alto torque** (um para cada lado do cinto).
- **SimHub** para ler dados do jogo (forças G) e enviar comandos via **Custom Serial Device**.

O fluxo básico é:

1. O SimHub lê acelerações lateral e longitudinal do jogo.
2. Um script em JavaScript converte essas forças em **tensão esquerda** e **tensão direita** (valores de 0 a 127), aplica suavização e formato de protocolo.
3. Os valores são enviados pela serial para o Arduino.
4. O Arduino interpreta esses bytes, aplica offsets, inversão (se configurada) e move os servos em tempo real.

---

## Hardware

### Componentes principais

- Arduino Nano.
- 2 × servos de alto torque, 180°, ~60 kg·cm (por exemplo, DSServo RDS5160 SSG ou similar).
- Módulo DC-DC step-up/step-down com capacidade de corrente adequada para os servos.
- Fonte 12 V (ou similar) dimensionada para a corrente total dos servos.
- Cabeamento e conectores adequados (por exemplo, XT60, se desejar).
- Opcional: banco de supercapacitores para suavizar picos de corrente.

### Ligação elétrica

#### 1. Alimentação dos servos

- **Nunca** alimente servos de alto torque diretamente da porta 5 V do Arduino.
- Use uma **fonte dedicada** ou módulo DC-DC ajustado para a tensão de operação dos servos (por exemplo, aproximadamente 8.2 V, conforme mencionado na descrição original; ajuste de acordo com a especificação dos seus servos).
- Ligue:

  - Saída positiva do módulo DC-DC → fio **V+** de alimentação de ambos os servos.
  - Saída negativa do módulo DC-DC → fio **GND** de ambos os servos.

#### 2. Alimentação do Arduino

- O Arduino Nano pode ser alimentado:
  - Pela porta USB (recomendado durante configuração e testes), ou
  - Pelo pino **VIN** (7–12 V) a partir da mesma fonte 12 V usada para o módulo DC-DC (com terra comum).

#### 3. Terra comum (GND)

- É **obrigatório** que o GND da fonte dos servos e o GND do Arduino estejam conectados entre si:
  - GND da fonte / módulo DC-DC → GND dos servos.
  - GND da fonte / módulo DC-DC → GND do Arduino (qualquer pino GND).
- Isso garante que os sinais de controle dos pinos digitais do Arduino tenham a mesma referência de terra que os servos.

#### 4. Sinais de controle dos servos

- Cada servo tem três fios típicos:
  - **Sinal** (geralmente laranja/branco).
  - **V+** (vermelho).
  - **GND** (preto/marrom).

- Conexão dos **sinais**:
  - Servo esquerdo:
    - Fio de sinal → pino `SERVO_LEFT` do Arduino (no código, pino 2).
  - Servo direito:
    - Fio de sinal → pino `SERVO_RIGHT` do Arduino (no código, pino 3).

- Conexão de **alimentação**:
  - Fios V+ de ambos os servos → saída positiva do módulo DC-DC.
  - Fios GND de ambos os servos → GND do módulo DC-DC (que também está ligado ao GND do Arduino).

#### 5. LED de status

- O código utiliza um LED de status ligado ao pino `LED` (pino 17 no mapping utilizado).
- Conexão típica:
  - Pino 17 → resistor (por exemplo, 220 Ω) → anodo do LED.
  - Catodo do LED → GND.
- O código pisca o LED com diferentes padrões para indicar:
  - Estado ocioso (idle).
  - Atuação do servo esquerdo.
  - Atuação do servo direito.

#### 6. Proteções recomendadas

- Fusível na linha de alimentação principal dos servos (por exemplo, 10 A, dependendo do seu conjunto).
- Conectores robustos (tipo XT60) entre fonte e módulo DC-DC.
- Se usar supercapacitores:
  - Colocá-los em paralelo com a saída do módulo DC-DC (respeitando tensão máxima).
  - Usar circuito de balanceamento se forem ligados em série.

---

## Software – Arduino

Principais características do sketch:

- Controla dois servos (`servo_left` e `servo_right`) pelos pinos definidos em `SERVO_LEFT` e `SERVO_RIGHT`.
- Usa a EEPROM para armazenar:
  - Offset do servo esquerdo (`ladd`).
  - Offset do servo direito (`radd`).
  - Flags de inversão de sentido dos servos.
- Na inicialização:
  - Lê offsets e flags da EEPROM.
  - Se a EEPROM não tiver valores válidos, usa valores padrão.
  - Move os servos para a posição “relaxada” configurada.
- No loop:
  - Gera padrão de pisca no LED para indicar estado.
  - Lê bytes vindos pela serial e interpreta conforme protocolo:
    - Comandos `< 2` ajustam offsets e salvam na EEPROM.
    - Comando específico para inversão salva flags na EEPROM.
    - Valores de posição movimentam os servos, aplicando:
      - Offset.
      - Inversão, se habilitada.
      - Limites de segurança de ângulo.

---

## Software – SimHub (JavaScript Custom Serial)

O script de saída do SimHub:

- Lê as propriedades de física do jogo (forças G longitudinais e laterais).
- Aplica ganhos configuráveis (`yaw_gain`, `decel_gain`).
- Garante que apenas **desaceleração (freio)** seja usada para tensionar.
- Converte essas forças em:
  - Tensão do cinto esquerdo.
  - Tensão do cinto direito.
- Usa um filtro IIR controlado por `smooth` para suavizar as variações.
- Limita as tensões em um intervalo seguro definido por `tmax`.
- Codifica os valores em bytes, usando:
  - Bit menos significativo (bit 0) para identificar qual servo é destinado (esquerdo/direito).
  - Faixas mínimas diferentes para cada lado, conforme protocolo esperado pelo Arduino.
- Envia pela serial:
  - Comandos de posição (tensão) para os dois servos.
  - Comando de configuração de inversão de direção, conforme checkboxes no SimHub:
    - `invert_servo_left`.
    - `invert_servo_right`.

---

## Fluxo de Uso

1. **Montagem de hardware**:
   - Fixar servos na estrutura do cockpit.
   - Passar os cintos pelos roletes/rolos.
   - Fazer todas as ligações elétricas conforme descrito acima.
2. **Configuração do Arduino**:
   - Gravar o sketch no Arduino Nano.
   - Conectar o Nano ao PC via USB.
3. **Configuração no SimHub**:
   - Adicionar o Arduino Nano como **Custom Serial Device**.
   - Carregar o perfil do belt tensioner.
   - Ajustar:
     - Offsets iniciais dos servos (posição relaxada).
     - `tmax` (tensão máxima aceitável).
     - Ganhos de yaw e desaceleração.
     - Suavização (`smooth`).
     - Inversão de servos caso algum esteja mecanicamente invertido.
4. **Teste**:
   - Com jogo rodando, verificar se:
     - Ao frear, ambos os cintos tensionam.
     - Em curva à direita, o cinto direito tensiona mais.
     - Em curva à esquerda, o cinto esquerdo tensiona mais.
     - Os movimentos são suaves e confortáveis (ajustar `tmax`, ganhos e `smooth` conforme necessário).

---

## Notas Finais

- Servos de alto torque podem consumir muita corrente próximos ao travamento mecânico. Ajuste `tmax` e offsets para evitar que o sistema force demais o mecanismo ou o ombro.
- Sempre teste com cuidado as primeiras vezes, aumentando a tensão gradualmente até encontrar um ponto confortável e seguro.