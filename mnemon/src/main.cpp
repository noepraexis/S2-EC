#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Definições dos pinos
#define DHT_PIN 23
#define DHT_TYPE DHT22
#define LDR_PIN 34
#define BUZZER_PIN 32
#define LED_R_PIN 25
#define LED_G_PIN 26
#define LED_B_PIN 27

// Inicialização dos componentes
DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Endereço I2C padrão, 16 colunas, 2 linhas
MPU6050 mpu;

// Configuração WiFi (para Wokwi)
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Servidor web
AsyncWebServer server(80);

// Variáveis globais
String currentScenario = "api_control";
String alertLevel = "normal";
unsigned long lastUpdate = 0;
unsigned long scenarioStartTime = 0;
int testStep = 0;

// Controles manuais dos sensores via API
struct SensorOverrides {
    bool dht22_override = false;
    float temperature_override = 25.0;
    float humidity_override = 60.0;
    
    bool ldr_override = false;
    int ldr_raw_override = 500;
    
    bool mpu6050_override = false;
    float accel_x_override = 0.0;
    float accel_y_override = 0.0;
    float accel_z_override = 1.0;
} sensorOverrides;

// Thresholds para alertas
const float TEMP_YELLOW = 35.0;
const float TEMP_RED = 45.0;
const float HUMIDITY_YELLOW = 80.0;
const float HUMIDITY_RED = 90.0;
const float ACCEL_YELLOW = 1.3;  // Vibração leve acima de 1g normal
const float ACCEL_RED = 1.7;     // Vibração forte

// Declarações das funções (protótipos)
float calculateLux(int rawValue);
String calculateAlertLevel(float temp, float humidity, float ax, float ay, float az);
void updateActuators(String level);
void setRGBColor(String color);
void updateLCDDisplay(float temp, float humidity, float lux, float ax, float ay, float az);
void sendJSONData(float temp, float humidity, int ldrRaw, float lux, float ax, float ay, float az);
void updateScenarioStep();
void setupWiFi();
void setupAPIRoutes();
void handleCORS(AsyncWebServerRequest *request);

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    
    // Inicializar I2C
    Wire.begin(21, 22); // SDA=21, SCL=22
    
    // Inicializar LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();
    
    // Inicializar sensores
    dht.begin();
    mpu.initialize();
    
    // Verificar se o MPU6050 está funcionando
    if (mpu.testConnection()) {
        Serial.println("MPU6050 conectado com sucesso");
    } else {
        Serial.println("ERRO: MPU6050 não encontrado");
    }
    
    // Configurar escala do MPU6050 para ±2g (0 = ±2g, 1 = ±4g, 2 = ±8g, 3 = ±16g)
    mpu.setFullScaleAccelRange(0);
    
    // Configurar pinos dos atuadores
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_R_PIN, OUTPUT);
    pinMode(LED_G_PIN, OUTPUT);
    pinMode(LED_B_PIN, OUTPUT);
    
    // LED inicial verde
    setRGBColor("green");
    digitalWrite(BUZZER_PIN, LOW);
    
    // Mostrar inicialização no LCD
    lcd.setCursor(0, 0);
    lcd.print("EIDOLON v2.0");
    lcd.setCursor(0, 1);
    lcd.print("Inicializando...");
    
    delay(2000);
    scenarioStartTime = millis();
    
    // Configurar WiFi e API REST
    setupWiFi();
    setupAPIRoutes();
    
    Serial.println("Sistema iniciado - Eidolon Multi-Sensor com API REST");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Atualizar dados a cada 2 segundos
    if (currentTime - lastUpdate >= 2000) {
        lastUpdate = currentTime;
        
        // Ler sensores (com possível override via API)
        float temperature, humidity;
        if (sensorOverrides.dht22_override) {
            temperature = sensorOverrides.temperature_override;
            humidity = sensorOverrides.humidity_override;
        } else {
            temperature = dht.readTemperature();
            humidity = dht.readHumidity();
        }
        
        int ldrRaw;
        if (sensorOverrides.ldr_override) {
            ldrRaw = sensorOverrides.ldr_raw_override;
        } else {
            ldrRaw = analogRead(LDR_PIN);
        }
        float lux = calculateLux(ldrRaw);
        
        // Ler MPU6050 (com possível override via API)
        float accelX, accelY, accelZ;
        if (sensorOverrides.mpu6050_override) {
            accelX = sensorOverrides.accel_x_override;
            accelY = sensorOverrides.accel_y_override;
            accelZ = sensorOverrides.accel_z_override;
        } else {
            int16_t ax, ay, az;
            mpu.getAcceleration(&ax, &ay, &az);
            
            // Converter para g (16384 LSB/g para escala ±2g)
            accelX = ax / 16384.0;
            accelY = ay / 16384.0;
            accelZ = az / 16384.0;
        }
        
        // Debug ocasional para verificação
        if (millis() % 10000 < 100) { // A cada 10s por 100ms
            Serial.print("MPU6050 Debug: X=");
            Serial.print(accelX, 2);
            Serial.print("g Y=");
            Serial.print(accelY, 2);
            Serial.print("g Z=");
            Serial.print(accelZ, 2);
            Serial.println("g");
        }
        
        // Calcular nível de alerta
        alertLevel = calculateAlertLevel(temperature, humidity, accelX, accelY, accelZ);
        
        // Atualizar atuadores
        updateActuators(alertLevel);
        
        // Atualizar LCD
        updateLCDDisplay(temperature, humidity, lux, accelX, accelY, accelZ);
        
        // Enviar JSON para console serial
        sendJSONData(temperature, humidity, ldrRaw, lux, accelX, accelY, accelZ);
        
        // Atualizar step do cenário
        updateScenarioStep();
    }
    
    delay(100);
}

float calculateLux(int rawValue) {
    // Fórmula para converter valor ADC para lux
    const float GAMMA = 0.7;
    const float RL10 = 50;
    float resistance = (4095.0 - rawValue) * 10000.0 / rawValue;
    return pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
}

String calculateAlertLevel(float temp, float humidity, float ax, float ay, float az) {
    float accelMagnitude = sqrt(ax*ax + ay*ay + az*az);
    
    // Verificar condições críticas (vermelho)
    if (temp >= TEMP_RED || humidity >= HUMIDITY_RED || accelMagnitude >= ACCEL_RED) {
        return "red";
    }
    
    // Verificar condições de alerta (amarelo)
    if (temp >= TEMP_YELLOW || humidity >= HUMIDITY_YELLOW || accelMagnitude >= ACCEL_YELLOW) {
        return "yellow";
    }
    
    return "normal";
}

void updateActuators(String level) {
    if (level == "red") {
        setRGBColor("red");
        digitalWrite(BUZZER_PIN, HIGH);
    } else if (level == "yellow") {
        setRGBColor("yellow");
        digitalWrite(BUZZER_PIN, LOW);
    } else {
        setRGBColor("green");
        digitalWrite(BUZZER_PIN, LOW);
    }
}

void setRGBColor(String color) {
    // Resetar todos os pinos
    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_G_PIN, LOW);
    digitalWrite(LED_B_PIN, LOW);
    
    if (color == "red") {
        digitalWrite(LED_R_PIN, HIGH);
    } else if (color == "green") {
        digitalWrite(LED_G_PIN, HIGH);
    } else if (color == "yellow") {
        digitalWrite(LED_R_PIN, HIGH);
        digitalWrite(LED_G_PIN, HIGH);
    }
}

void updateLCDDisplay(float temp, float humidity, float lux, float ax, float ay, float az) {
    lcd.clear();
    
    // Linha 1: Cenário e status
    lcd.setCursor(0, 0);
    lcd.print(currentScenario.substring(0, 8)); // Primeiros 8 chars
    lcd.setCursor(9, 0);
    
    if (alertLevel == "red") {
        lcd.print("ALERT!");
    } else if (alertLevel == "yellow") {
        lcd.print("WARN");
    } else {
        lcd.print("OK");
    }
    
    lcd.setCursor(14, 0);
    lcd.print(String(testStep));
    
    // Linha 2: Dados dos sensores (rotativo)
    lcd.setCursor(0, 1);
    
    unsigned long elapsed = (millis() - scenarioStartTime) / 1000;
    int displayMode = (elapsed / 5) % 4; // Muda a cada 5 segundos
    
    switch (displayMode) {
        case 0: // Temperatura e umidade
            lcd.print("T:");
            lcd.print(temp, 1);
            lcd.print("C H:");
            lcd.print(humidity, 0);
            lcd.print("%");
            break;
            
        case 1: // Luminosidade
            lcd.print("Luz: ");
            lcd.print(lux, 0);
            lcd.print(" lux");
            break;
            
        case 2: // Aceleração X,Y
            lcd.print("X:");
            lcd.print(ax, 1);
            lcd.print(" Y:");
            lcd.print(ay, 1);
            break;
            
        case 3: // Aceleração Z e tempo
            lcd.print("Z:");
            lcd.print(az, 1);
            lcd.print(" ");
            lcd.print(elapsed);
            lcd.print("s");
            break;
    }
}

void sendJSONData(float temp, float humidity, int ldrRaw, float lux, float ax, float ay, float az) {
    JsonDocument doc;
    
    // Timestamp ISO 8601
    char timestamp[32];
    unsigned long elapsed = millis() / 1000;
    // Timestamp mais realístico baseado no tempo atual (13 de junho de 2025 como base)
    unsigned long totalSeconds = elapsed + 1734120000; // Offset para 13/06/2025 00:00:00 UTC
    unsigned long days = totalSeconds / 86400;
    unsigned long daySeconds = totalSeconds % 86400;
    unsigned long hours = daySeconds / 3600;
    unsigned long minutes = (daySeconds % 3600) / 60;
    unsigned long seconds = daySeconds % 60;
    
    // Ajustar para 13 de junho de 2025 como data base
    int day = 13 + (days % 30); // Simular progressão de dias
    int month = 6;
    int year = 2025;
    
    if (day > 30) {
        day = day - 30;
        month++;
        if (month > 12) {
            month = 1;
            year++;
        }
    }
    
    sprintf(timestamp, "%04d-%02d-%02dT%02d:%02d:%02dZ", 
            year, month, day, (int)hours, (int)minutes, (int)seconds);
    
    doc["timestamp"] = timestamp;
    doc["scenario"] = currentScenario;
    doc["step"] = testStep;
    doc["elapsed_seconds"] = elapsed;
    
    // Dados dos sensores
    JsonObject sensors = doc["sensors"].to<JsonObject>();
    
    JsonObject dht22 = sensors["dht22"].to<JsonObject>();
    dht22["temperature"] = round(temp * 10) / 10.0;
    dht22["humidity"] = round(humidity * 10) / 10.0;
    
    JsonObject ldr = sensors["ldr"].to<JsonObject>();
    ldr["raw"] = ldrRaw;
    ldr["lux"] = round(lux);
    
    JsonObject mpu6050 = sensors["mpu6050"].to<JsonObject>();
    mpu6050["accelX"] = round(ax * 100) / 100.0;
    mpu6050["accelY"] = round(ay * 100) / 100.0;
    mpu6050["accelZ"] = round(az * 100) / 100.0;
    
    // Status dos atuadores
    JsonObject actuators = doc["actuators"].to<JsonObject>();
    
    String rgbColor = "green";
    if (alertLevel == "red") rgbColor = "red";
    else if (alertLevel == "yellow") rgbColor = "yellow";
    
    actuators["rgb_led"] = rgbColor;
    actuators["buzzer"] = (alertLevel == "red") ? "on" : "off";
    
    doc["alert_level"] = alertLevel;
    
    // Enviar JSON
    serializeJson(doc, Serial);
    Serial.println();
}

void updateScenarioStep() {
    // Simular progressão do cenário baseado no tempo
    unsigned long elapsed = (millis() - scenarioStartTime) / 1000;
    
    if (currentScenario == "sensor_validation") {
        testStep = (elapsed / 10) % 8 + 1; // Steps 1-8, muda a cada 10s
    } else if (currentScenario == "realistic_conditions") {
        testStep = (elapsed / 30) % 10 + 1; // Steps 1-10, muda a cada 30s
    } else if (currentScenario == "extreme_conditions") {
        testStep = (elapsed / 20) % 6 + 1; // Steps 1-6, muda a cada 20s
    }
}

// Configuração WiFi
void setupWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Conectando ao WiFi");
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.print("WiFi conectado! IP: ");
        Serial.println(WiFi.localIP());
        
        // Mostrar IP no LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi: OK");
        lcd.setCursor(0, 1);
        lcd.print(WiFi.localIP());
        delay(3000);
    } else {
        Serial.println("\nWiFi falhou - modo standalone");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WiFi: FALHOU");
        lcd.setCursor(0, 1);
        lcd.print("Modo local");
        delay(2000);
    }
}

// Configuração das rotas da API REST
void setupAPIRoutes() {
    // Middleware CORS
    server.onNotFound([](AsyncWebServerRequest *request) {
        if (request->method() == HTTP_OPTIONS) {
            handleCORS(request);
        } else {
            request->send(404, "application/json", "{\"error\":\"Endpoint not found\"}");
        }
    });

    // GET /api/status - Status atual dos sensores
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {        
        JsonDocument doc;
        doc["status"] = "online";
        doc["scenario"] = currentScenario;
        doc["alert_level"] = alertLevel;
        doc["uptime_seconds"] = millis() / 1000;
        doc["wifi_ip"] = WiFi.localIP().toString();
        
        JsonObject overrides = doc["overrides"].to<JsonObject>();
        overrides["dht22"] = sensorOverrides.dht22_override;
        overrides["ldr"] = sensorOverrides.ldr_override;
        overrides["mpu6050"] = sensorOverrides.mpu6050_override;
        
        String responseStr;
        serializeJson(doc, responseStr);
        
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", responseStr);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    // POST /api/sensors/dht22 - Controlar temperatura e umidade
    server.on("/api/sensors/dht22", HTTP_POST, [](AsyncWebServerRequest *request) {
        // Headers serão adicionados no callback de body
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        JsonDocument doc;
        if (deserializeJson(doc, data, len) == DeserializationError::Ok) {
            
            if (!doc["temperature"].isNull()) {
                sensorOverrides.temperature_override = doc["temperature"];
                sensorOverrides.dht22_override = true;
            }
            if (!doc["humidity"].isNull()) {
                sensorOverrides.humidity_override = doc["humidity"];
                sensorOverrides.dht22_override = true;
            }
            if (!doc["disable"].isNull() && doc["disable"]) {
                sensorOverrides.dht22_override = false;
            }
            
            JsonDocument response;
            response["success"] = true;
            response["temperature"] = sensorOverrides.temperature_override;
            response["humidity"] = sensorOverrides.humidity_override;
            response["override_active"] = sensorOverrides.dht22_override;
            
            String responseStr;
            serializeJson(response, responseStr);
            
            AsyncWebServerResponse *resp = request->beginResponse(200, "application/json", responseStr);
            resp->addHeader("Access-Control-Allow-Origin", "*");
            request->send(resp);
        } else {
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        }
    });

    // POST /api/sensors/ldr - Controlar luminosidade
    server.on("/api/sensors/ldr", HTTP_POST, [](AsyncWebServerRequest *request) {
        handleCORS(request);
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        JsonDocument doc;
        if (deserializeJson(doc, data, len) == DeserializationError::Ok) {
            
            if (!doc["raw_value"].isNull()) {
                sensorOverrides.ldr_raw_override = doc["raw_value"];
                sensorOverrides.ldr_override = true;
            }
            if (!doc["disable"].isNull() && doc["disable"]) {
                sensorOverrides.ldr_override = false;
            }
            
            JsonDocument response;
            response["success"] = true;
            response["raw_value"] = sensorOverrides.ldr_raw_override;
            response["lux"] = calculateLux(sensorOverrides.ldr_raw_override);
            response["override_active"] = sensorOverrides.ldr_override;
            
            String responseStr;
            serializeJson(response, responseStr);
            request->send(200, "application/json", responseStr);
        } else {
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        }
    });

    // POST /api/sensors/mpu6050 - Controlar aceleração
    server.on("/api/sensors/mpu6050", HTTP_POST, [](AsyncWebServerRequest *request) {
        handleCORS(request);
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        JsonDocument doc;
        if (deserializeJson(doc, data, len) == DeserializationError::Ok) {
            
            if (!doc["accel_x"].isNull()) {
                sensorOverrides.accel_x_override = doc["accel_x"];
                sensorOverrides.mpu6050_override = true;
            }
            if (!doc["accel_y"].isNull()) {
                sensorOverrides.accel_y_override = doc["accel_y"];
                sensorOverrides.mpu6050_override = true;
            }
            if (!doc["accel_z"].isNull()) {
                sensorOverrides.accel_z_override = doc["accel_z"];
                sensorOverrides.mpu6050_override = true;
            }
            if (!doc["disable"].isNull() && doc["disable"]) {
                sensorOverrides.mpu6050_override = false;
            }
            
            JsonDocument response;
            response["success"] = true;
            response["accel_x"] = sensorOverrides.accel_x_override;
            response["accel_y"] = sensorOverrides.accel_y_override;
            response["accel_z"] = sensorOverrides.accel_z_override;
            response["override_active"] = sensorOverrides.mpu6050_override;
            
            String responseStr;
            serializeJson(response, responseStr);
            request->send(200, "application/json", responseStr);
        } else {
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        }
    });

    // POST /api/sensors/reset - Resetar todos os overrides
    server.on("/api/sensors/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
        handleCORS(request);
        
        sensorOverrides.dht22_override = false;
        sensorOverrides.ldr_override = false;
        sensorOverrides.mpu6050_override = false;
        
        JsonDocument response;
        response["success"] = true;
        response["message"] = "All sensor overrides disabled";
        
        String responseStr;
        serializeJson(response, responseStr);
        request->send(200, "application/json", responseStr);
    });

    server.begin();
    Serial.println("Servidor HTTP iniciado na porta 80");
}

// Manipular headers CORS
void handleCORS(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "");
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(response);
}

// Função para mudar cenário via comando serial (opcional)
void serialEvent() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.startsWith("scenario:")) {
            currentScenario = command.substring(9);
            testStep = 0;
            scenarioStartTime = millis();
            
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Cenario mudou:");
            lcd.setCursor(0, 1);
            lcd.print(currentScenario);
            delay(2000);
        }
    }
}