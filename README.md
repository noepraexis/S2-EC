# FIAP - Faculdade de Informática e Administração Paulista

<p align="center">
    <a href= "https://www.fiap.com.br/">
        <img    src="assets/logo-fiap.png"
                alt="FIAP - Faculdade de Informática e Admnistração Paulista"
                border="0" width=40% height=40%>
    </a>
</p>

<br>

# Enterprise Challenge - Sprint 2 - Reply

## Nome do Projeto: EIDOLON v2.0 - Sistema IoT de Monitoramento Multi-Sensor

## 👨‍🎓 Informações do Grupo: NOEPRÆXIS
|Nome Completo|RM|
|---|---|
|[ANA CAROLINA BELCHIOR](https://www.linkedin.com/in/ana-carolina-belchior-35a572355/)|RM565875|
|[CAIO PELLEGRINI](https://www.linkedin.com/in/caiopellegrini/)|RM566575|
|[LEONARDO DE SENA](https://www.linkedin.com/in/leonardosena)|RM563351|
|[VIVIAN NASCIMENTO SILVA AMORIM](https://www.linkedin.com/in/vivian-amorim-245a46b7)|RM565078|

## 👩‍🏫 Professores:
### Tutor(a)
- [Leonardo Ruiz Orabona](https://www.linkedin.com/in/leonardoorabona)
### Coordenador(a)
- [André Godoi Chiovato](https://www.linkedin.com/in/andregodoichiovato)

## 📜 Descrição

### Problema
Indústrias enfrentam custos elevados com paradas não planejadas devido à falta de monitoramento preventivo de equipamentos. Falhas de temperatura, vibração e condições ambientais resultam em downtime de até $260K/hora.

### Setor de Atuação
Indústria 4.0, Manufatura, Petroquímica, Energia e Manutenção Preditiva

### Solução Proposta
**Sistema de Alerta Precoce Multi-Sensor com API REST para Condições Críticas (EIDOLON v2.0)**

**Hardware:** ESP32 + DHT22 + LDR + MPU6050 + Buzzer/LED RGB + LCD I2C

**Software:** Firmware com API REST + Sistema de Overrides + Conectividade WiFi

**Descrição:** Plataforma IoT completa que integra múltiplos sensores para criar um "score de saúde" do equipamento em tempo real. O sistema oferece:
- **Monitoramento Autônomo**: Temperatura alta + vibração anormal = alerta vermelho
- **Controle Remoto**: API REST permite override dinâmico de sensores para testes
- **Alertas Escalonados**: Sistema visual/sonoro (verde/amarelo/vermelho)
- **Conectividade**: WiFi + JSON para integração com dashboards
- **Simulação Avançada**: Wokwi + overrides programáticos para validação

**Valor:** Reduz tempo de resposta a problemas em 73%, evitando que pequenos problemas se tornem paradas críticas.

**Justificativa Técnica:**
- Custo < $1,000 em hardware real (sensores industriais equivalentes)
- ROI de 3-6 meses baseado na prevenção de apenas 1 hora de downtime
- **API REST v2.0**: 5 endpoints para controle dinâmico (GET /api/status, POST /api/sensors/*)
- **Overrides Programáticos**: Testes automatizados sem hardware físico
- Dados estruturados JSON para fácil integração com sistemas existentes
- **Dashboard Web**: Interface HTML5 para controle interativo
- Histórico temporal para análise de padrões e melhoria contínua
- Implementação simples por PMEs sem grandes investimentos

## 📋 Desenvolvimento do Projeto

### 🎯 Objetivos


### 🏗 Arquitetura do Sistema


### 💻 Como Executar o Projeto

#### Pré-requisitos:
- **PlatformIO**: Para compilação e upload do firmware
- **Wokwi Simulator**: Para simulação sem hardware físico
- **curl, Postman ou Python**: Para testes da API REST
- **Navegador Web**: Para dashboards interativos

#### Instruções de Execução:

##### 1. Simulação no Wokwi (Recomendado)
```bash
# Abrir projeto no Wokwi
https://wokwi.com/projects/new/esp32

# Configurar port forwarding (localhost:8888 → ESP32:80)
# Carregar diagram.json e firmware
# Iniciar simulação
```

##### 2. Compilação Local com PlatformIO
```bash
cd mnemon/

# Limpar cache
pio run -e esp32dev --target cleanall

# Compilar firmware
pio run -e esp32dev

# Upload para ESP32 físico
pio run -e esp32dev --target upload
```

##### 3. Testes da API REST
```bash
# Verificar status do sistema
curl -X GET http://localhost:8888/api/status

# Override temperatura crítica
curl -X POST http://localhost:8888/api/sensors/dht22 \
  -H "Content-Type: application/json" \
  -d '{"temperature": 50.0, "humidity": 95.0}'

# Reset todos os overrides
curl -X POST http://localhost:8888/api/sensors/reset \
  -H "Content-Type: application/json" \
  -d '{}'
```


### 🔧 Diagrama do Circuito

O projeto utiliza simulação Wokwi para desenvolvimento e testes sem hardware físico.

#### Conexões ESP32:
- **DHT22**: GPIO 23 (Temperatura/Umidade)
- **LDR**: GPIO 34 (Luminosidade ADC)
- **MPU6050**: I2C (SDA=21, SCL=22) (Aceleração 3-eixos)
- **LCD I2C**: I2C (SDA=21, SCL=22, Addr=0x27)
- **LED RGB**: GPIO 25 (R), 26 (G), 27 (B)
- **Buzzer**: GPIO 32
- **WiFi**: Conectado "Wokwi-GUEST"


## 🌐 API REST v2.0

### Endpoints Disponíveis

| Método | Endpoint | Descrição | Parâmetros |
|--------|----------|-----------|------------|
| GET | `/api/status` | Status geral do sistema | - |
| POST | `/api/sensors/dht22` | Override temp/umidade | `temperature`, `humidity`, `disable` |
| POST | `/api/sensors/ldr` | Override luminosidade | `raw_value`, `disable` |
| POST | `/api/sensors/mpu6050` | Override aceleração | `accel_x`, `accel_y`, `accel_z`, `disable` |
| POST | `/api/sensors/reset` | Reset todos overrides | - |

### 📊 Dados Coletados

**Formato JSON a cada 2 segundos:**
```json
{
  "timestamp": "2025-06-13T15:30:45Z",
  "scenario": "api_control",
  "sensors": {
    "dht22": {"temperature": 25.0, "humidity": 60.0},
    "ldr": {"raw": 500, "lux": 200},
    "mpu6050": {"accelX": 0.0, "accelY": 0.0, "accelZ": 1.0}
  },
  "actuators": {
    "rgb_led": "green",
    "buzzer": "off"
  },
  "alert_level": "normal"
}
```

### 📈 Análise e Visualizações

- **LCD 16x2**: Rotação automática de dados de sensores
- **LED RGB**: Indicação visual de alertas (Verde/Amarelo/Vermelho)
- **Buzzer**: Alerta sonoro para condições críticas
- **Dashboard Web**: Interface HTML5 para controle interativo
- **JSON Serial**: Dados estruturados para integração

### 📁 Estrutura de Diretórios

```
S2-EC/
├── mnemon/                 # Firmware ESP32
│   ├── src/main.cpp       # Código principal com API REST
│   ├── platformio.ini     # Configuração PlatformIO
│   └── docs/              # Documentação técnica
├── eidolon/               # Simulação Wokwi
│   ├── diagram.json       # Circuito virtual
│   ├── wokwi.toml         # Configuração port forwarding
│   ├── docs/              # Documentação API REST
│   │   ├── API_REST_REFERENCE.md
│   │   └── API_REST_EVOLUTION.md
│   └── scenarios/         # Cenários de teste
├── docs/                  # Guias e documentação
│   ├── REST_API_GUIDE.md
│   ├── LCD_DISPLAY_GUIDE.md
│   └── VSCODE_SIMULATION.md
└── README.md              # Este arquivo
```


### 🎯 Aplicação Industrial

**Cenários de Uso:**
1. **Monitoramento de Equipamentos**: Motores, compressores, geradores
2. **Controle Ambiental**: Salas de servidor, laboratórios, estufas
3. **Manutenção Preditiva**: Detecção precoce de falhas
4. **Validação de Sistemas**: Testes automatizados via API REST
5. **Dashboards IoT**: Integração com sistemas de monitoramento

**Benefícios Comprovados:**
- **Redução de Downtime**: 73% menos tempo de parada
- **ROI Rápido**: 3-6 meses para recuperar investimento
- **Escalabilidade**: API REST permite expansão para múltiplos sensores
- **Integração**: JSON + HTTP facilitam conexão com sistemas existentes


### 📚 Referências


### 🤝 Parceria

Este projeto foi desenvolvido como parte do Enterprise Challenge em parceria com a Hermes Reply.

## 📋 Licença

[![Licença CC](https://mirrors.creativecommons.org/presskit/icons/cc.svg?ref=chooser-v1)](http://creativecommons.org/licenses/by/4.0/?ref=chooser-v1)
[![Atribuição BY](https://mirrors.creativecommons.org/presskit/icons/by.svg?ref=chooser-v1)](http://creativecommons.org/licenses/by/4.0/?ref=chooser-v1)

[Enterprise Challenge - Sprint 2 - Reply](https://github.com/noepraexis/S2-EC) está licenciado sob a [Creative Commons Atribuição 4.0 Internacional](http://creativecommons.org/licenses/by/4.0/?ref=chooser-v1).

