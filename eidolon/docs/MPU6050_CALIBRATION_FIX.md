# 🔧 MPU6050 Calibração e Correção de Escala

## ❌ Problema Identificado

O MPU6050 estava reportando `accelZ: 2` ao invés do valor esperado `9.81g`, causando alertas falsos.

## 🔍 Análise da Causa

### Problema de Saturação:
- **MPU6050 Escala:** ±2g com resolução 16384 LSB/g
- **Valor máximo:** 32767 (signed 16-bit) = 2.0g máximo
- **Valor original:** 9.81g × 16384 = 160,659 → **SATURAÇÃO** → 32767
- **Resultado:** 32767 ÷ 16384 = 2.0g

### Valores Observados:
```
// Antes da correção:
Raw: az=32767 | g: Z=2.000  ❌

// Depois da correção:
Raw: az=16384 | g: Z=1.000  ✅
```

## ✅ Correções Aplicadas

### 1. **diagram.json - Valor Base Corrigido:**
```json
"mpu6050": {
  "attrs": {
    "accelX": "0",
    "accelY": "0", 
    "accelZ": "1"  // ✅ Mudado de "9.81" para "1"
  }
}
```

### 2. **Cenários YAML - Faixas Realistas:**
```yaml
# Valores dentro da faixa ±2g
- control: accel-z
  value: 1.0    # ✅ Gravidade normal (era 9.81)
  
- control: accel-z  
  value: 1.1    # ✅ Vibração leve (era 9.91)
  
- control: accel-z
  value: 0.9    # ✅ Vibração leve (era 9.71)
```

### 3. **Firmware - Thresholds Ajustados:**
```cpp
// Antes (baseado em 9.81g):
const float ACCEL_YELLOW = 0.5;  // ❌ Muito baixo
const float ACCEL_RED = 1.0;     // ❌ Muito baixo

// Depois (baseado em 1g):
const float ACCEL_YELLOW = 1.3;  // ✅ 30% acima do normal
const float ACCEL_RED = 1.7;     // ✅ 70% acima do normal
```

## 📊 Faixas de Operação

### **MPU6050 no Wokwi:**
- **Escala:** ±2g (limitação do simulador)
- **Resolução:** 16384 LSB/g
- **Faixa Válida:** -2.0g a +2.0g

### **Valores de Referência:**
```
Repouso:      1.0g  (gravidade terrestre)
Vibração Leve: ±0.1g (1.0 ± 0.1)
Vibração Forte: ±0.7g (1.0 ± 0.7)
Limite Máximo: ±2.0g (saturação)
```

### **Sistema de Alertas:**
```
🟢 NORMAL:  magnitude < 1.3g
🟡 YELLOW:  1.3g ≤ magnitude < 1.7g  
🔴 RED:     magnitude ≥ 1.7g
```

## 🧪 Validação dos Resultados

### **Teste 1 - Condições Normais:**
```json
{
  "mpu6050": {"accelX":0,"accelY":0,"accelZ":1},
  "actuators": {"rgb_led":"green","buzzer":"off"},
  "alert_level": "normal"
}
```

### **Teste 2 - Vibração (Cenário):**
```yaml
# Cenário aplica accelZ: 1.1
{
  "mpu6050": {"accelX":0,"accelY":0,"accelZ":1.1},
  "alert_level": "normal"  # Ainda dentro do limite
}
```

### **Teste 3 - Condição Crítica:**
```yaml
# Temperatura alta dispara alerta
{
  "dht22": {"temperature":48,"humidity":85},
  "alert_level": "red"
}
```

## 🎯 Funcionalidades Validadas

### ✅ **Hardware Configurado:**
- MPU6050 I2C funcionando (SDA=D21, SCL=D22)
- Escala ±2g configurada corretamente
- Conversão 16384 LSB/g implementada

### ✅ **Cenários Funcionais:**
- `sensor_validation.yaml` - Teste completo dos sensores
- `realistic_conditions.yaml` - Variações realistas
- Controles Wokwi aplicando valores corretamente

### ✅ **Sistema de Alertas:**
- Limites ajustados para nova escala
- LED RGB respondendo corretamente
- Buzzer ativado apenas em condições críticas

## 🚀 Como Testar

```bash
# 1. Compilar firmware corrigido
cd mnemon
source ../.venv-s2-ec/bin/activate
pio run -e esp32dev

# 2. Executar cenário com valores corretos
cd ../eidolon
~/bin/wokwi-cli --scenario scenarios/realistic_conditions.yaml

# 3. Verificar output esperado:
# ✅ accelZ: 1 (condições normais)
# ✅ alert_level: "normal" 
# ✅ LED verde, buzzer desligado
```

## 📋 Status Final

**🟢 PROBLEMA RESOLVIDO**
- MPU6050 reportando valores corretos
- Cenários aplicando controles corretamente  
- Sistema de alertas funcionando
- Simulação completamente operacional

O sistema agora simula corretamente um acelerômetro em repouso (1g) com possibilidade de detectar vibrações e movimentos dentro da faixa ±2g disponível no Wokwi.