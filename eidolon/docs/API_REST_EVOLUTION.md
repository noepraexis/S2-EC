# 🚀 Evolução: De Cenários YAML para API REST

## 📅 Linha do Tempo

### v1.0 - Cenários YAML Estáticos
- **Controle**: Arquivos YAML pré-definidos
- **Execução**: `wokwi-cli --scenario arquivo.yaml`
- **Limitações**: 
  - Sequências fixas de comandos
  - Sem interação em tempo real
  - Requer edição de arquivos para mudanças

### v2.0 - API REST Dinâmica (ATUAL)
- **Controle**: Requisições HTTP em tempo real
- **Execução**: `curl -X POST http://localhost:8888/api/sensors/*`
- **Vantagens**:
  - ✅ Controle instantâneo e granular
  - ✅ Integração com qualquer linguagem/ferramenta
  - ✅ Possibilita dashboards e aplicações web
  - ✅ Testes automatizados mais flexíveis

## 🔄 Comparação Prática

### Cenário YAML (Antigo):
```yaml
# arquivo sensor_test.yaml
steps:
  - delay: 1000ms
  - set-control:
      part-id: dht22
      control: temperature
      value: 45.0
  - delay: 5000ms
  - set-control:
      part-id: dht22
      control: temperature
      value: 25.0
```

### API REST (Novo):
```bash
# Controle direto e imediato
curl -X POST localhost:8888/api/sensors/dht22 \
  -H "Content-Type: application/json" \
  -d '{"temperature": 45.0}'

# Aguardar quanto quiser...

curl -X POST localhost:8888/api/sensors/dht22 \
  -H "Content-Type: application/json" \
  -d '{"temperature": 25.0}'
```

## 🎯 Casos de Uso Avançados

### 1. Dashboard Web Interativo
```javascript
// Controle via JavaScript
async function setTemperature(value) {
  await fetch('http://localhost:8888/api/sensors/dht22', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({temperature: value})
  });
}

// Slider HTML para controle em tempo real
<input type="range" min="0" max="100" onchange="setTemperature(this.value)">
```

### 2. Testes Automatizados Python
```python
import requests
import pytest

def test_alert_system():
    # Reset inicial
    requests.post("http://localhost:8888/api/sensors/reset")
    
    # Teste condição crítica
    response = requests.post(
        "http://localhost:8888/api/sensors/dht22",
        json={"temperature": 50.0}
    )
    
    # Verificar resposta
    status = requests.get("http://localhost:8888/api/status").json()
    assert status["alert_level"] == "red"
```

### 3. Simulação de Condições Reais
```python
import math
import time
import requests

# Simular variação senoidal de temperatura
for t in range(0, 360, 10):
    temp = 25 + 10 * math.sin(math.radians(t))
    requests.post(
        "http://localhost:8888/api/sensors/dht22",
        json={"temperature": temp}
    )
    time.sleep(1)
```

## 📊 Endpoints Disponíveis

| Método | Endpoint | Descrição |
|--------|----------|-----------|
| GET | `/api/status` | Status geral do sistema |
| POST | `/api/sensors/dht22` | Controlar temperatura/umidade |
| POST | `/api/sensors/ldr` | Controlar luminosidade |
| POST | `/api/sensors/mpu6050` | Controlar aceleração |
| POST | `/api/sensors/reset` | Resetar todos os overrides |

## 🌟 Benefícios da Evolução

1. **Flexibilidade Total**: Mudar valores instantaneamente sem editar arquivos
2. **Integração Universal**: Qualquer linguagem pode fazer requisições HTTP
3. **Testes Dinâmicos**: Cenários complexos e adaptativos
4. **Interface Visual**: Possibilidade de criar GUIs e dashboards
5. **Automação Avançada**: Scripts que respondem a condições em tempo real

## 💡 Conclusão

A evolução de **cenários YAML estáticos** para **API REST dinâmica** transformou o sistema Eidolon de uma ferramenta de simulação básica em uma **plataforma completa de desenvolvimento IoT**, permitindo:

- 🎮 **Controle interativo** em tempo real
- 🔧 **Desenvolvimento ágil** com feedback instantâneo
- 🧪 **Testes sofisticados** com lógica condicional
- 🌐 **Integração ampla** com ecossistema de ferramentas

O sistema agora é verdadeiramente **dinâmico**, **extensível** e **pronto para produção**!