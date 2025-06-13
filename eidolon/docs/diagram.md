# Circuito Wokwi

**DHT22 no GPIO23**
- GPIO23 é um pino sem funções especiais durante o boot, evitando interferências
- Suporta interrupts necessários para o protocolo one-wire do DHT22
- Fisicamente próximo ao 3.3V e GND, minimizando comprimento dos fios e ruído
- Não interfere com JTAG (GPIOs 12-15) ou Flash (GPIOs 6-11)

**LDR no GPIO 34 com Pull-down 10KΩ**
* GPIO34
    - É um dos ADC1 (GPIOs 32-39) que funciona mesmo com WiFi ativo
    - Input-only Pin: ideal para sensores, economiza GPIOs bidirecionais para outras funções
    - ADC1_CH6 tem menor ruído que os outros canais ADC
* Resistor 10KΩ
    - Forma divisor de tensão ideal com LDR (tipicamente 1KΩ-10Ω)
    - Fornece range completo 0-3.3V para máxima resolução ADC (12-bit)
    - Valor padrão que garante resposta rápida (<1ms) sem consumo excessivo

**MPU6050 nos GPIOs 21/22 (I2C)**
- GPIOs 21/22 são os pinos I2C padrão do ESP32 com pull-ups internos
- Hardware I2C permite 400KHz necessários para 100Hz de amostragem
- Localização permite barramento I2C curto, reduzindo capacitância parasita
- Deixa I2C secundário (GPIOs 16/17) livre para expansão futura

**LED RGB nos GPIOs 25/26/27 com Resistores 220Ω**
* GPIOs
    - Todos suportam PWM hardware para controle de brilho suave
    - Consecutivos facilitam mapeamento mental (R=25, G=26, B=27)
    - Não são usados por periféricos internos ou boot strapping
    - Distantes do ADC, evitando acoplamento de ruído PWM
* Resistores 220Ω
    - Limitacorrente a 10mA por LED (3.3V-2V)/220Ω ≈ 6mA
    - Total 18mA está dentro do limite de 40mA por GPIO
    - Brilho adequado para visualizaçaõ industrial sem ofuscar
    - Protege ESP32 mesmo se software configurar todos HIGH simultaneamente

**Buzzer no GPIO 32**
- GPIO 32 é ADC1_CH4 mas como usamos para saída PWM, não há conflito
- Suporta LEDC PWM para tons variáveis (não apenas beep fixo)
- Fisicamente distante do MPU6050, evitando vibração mecânica nos dados
- Permite usar touch sensor (se necessário) sem reconfiguração

**Organização Física do Layout**
* Sensores em cima:
    - Simula montagem real onde sensores ficam no equipamento monitorado
    - Facilita identificação visual durante debug
    - Reduz loops de terra mantendo caminhos de retorno curtos
* Atuadores embaixo:
    - Separa sinais de potência (PWM) dos sensíveis (ADC/I2C)
    - Permite blindagem futura entre zonas se necessário
    - Interface homem-máquina fica acessível ao operador

**Cores dos Fios**
- Vermelho/Preto (Alimentação): Padrão universal, previne inversão
- Azul/Amarelo (I2C): Padrão industrial (azul=SDA, amarelo=SCL)
- Verde (Dados): Cor neutra para sinais digitais únicos
- Facilita manutenção e troubleshooting em campo

**Decisões de Roteamento**
* Alimentação em estrela:
    - Cada sensor tem caminho direto ao 3.3V/GND
    - Evita quedas de tensãoem cascata
    - Minimiza acoplamento de ruído entre sensores
* Separação de domínios
    - Sinais analógicos (LDR) isolados dos digitais rápidos
    - I2C com roteamento paralelo para manter impedância característica
    - PWM dos LEDs agrupados para sincronização de fase

**Capacidade de Expansão**
Pinos reservados para futuro:
- GPIO 16/17: I2C secundário para display OLED
- GPIO 18/19: SPI para cartão SD de logging
- GPIOs 35-39: ADCs adicionais para mais sensores analógicos
- GPIO 2: LED interno para debug sem afetar RGB

**Proteções Implementadas**
- Sem conexões em GPIOs 0/2/12/15: Evita problemas de boot
- Resistores limitadores: Protegem contra curtos acidentais
- Pull-down no ADC: Previne flutuação e leituras errôneas
- 3.3V only: Todos os componentes nativos 3.3V, sem level shifters.

Cada decisão foi tomda considerando não apenas a funcionalidade, mas também confiabilidade industrial, facilidade de manutenção e escalabilidade futura - críticos para um sistema que protege contra perdas de $260K USD/hora.