# Sistema Embarcado de Monitoramento Industrial

A proposta desenvolve-se sob uma estrutura modular, reflitindo os princípios de **responsabilidade única**, **independência funcional e separação de preocupações**. O projeto é composto por **quatro módulos principais**, organizados de forma a espelhar o fluxo complet de um sistema ciberfísico: desde a simulação do hardware até a extração de conhecimento sobre os fenômenos monitorados.

---

## 1. `Eidolon` - Módulo de Hardware Simulado
**Responsabilidade:** Modelar e representar digitalmente o circuito fisíco composto por ESP32 e sensores industriais, operando am ambiente de simulação.

**Funções principais:**
- Construção do circuito virtual na plataforma Wokwi;
- Escolha e parametrização dos sensores simulados (temperatura, vibração, luminosidade etc.);
- Simulação de variações realistas nos sinais fisícos (ruídos, flutuações, limites).

---

## 2. `Mnemon` - Módulo de Firmware (Software Embarcado)
**Responsabilidade:** Implementar a lógica embarcada responsável pela aquisição dos dados sensoriais e sua transmissão estruturada para o exterior do microcontrolador.

**Funções principais:**
- Leitura periódica do sensores conectados ao ESP32;
- Aplicação de pré-processamento e controle de fluxo local;
- Transmissão dos dados via Serial/UART para captura externa;
- Implementação de lógica de debounce, filtros, ou estruturação do playload.

---

## 3. `Katabase` - Módulo de Coleta e Armazenamento
**Responsabilidade:** Realizar a captura, registro e estruturação dos dados adquiridos, garantind integridade temporal e persistência em formato utilizável para análise.

**Funções principais:**
- Escuta contínua da porta serial para recepção dos dados;
- Exportação automática ou manual para formatos como CSV;
- Organização dos dados brutos conforme variáveis e metadados;
- Estruturação da base para posterior exploração estatística.

---

## 4. `Gnoseon` - Módulo de Análise e Visualização
**Responsabilidade:** Explorar os dados registrados, identificando padrões, correlações e anomalias, e apresentando os resultados de forma visual, estatística e interpretável.

**Funções principais:**
- Processamento dos dados utilizando **Python** ou **R**;
- Geração de gráficos exploratórios (linha, barra, dispersão, boxplot);
- Cálculo de estatísticas descriticas e medidas de tendência;
- Extração de **insight operacionais**, inclusive com base em lógica preditiva simples.

