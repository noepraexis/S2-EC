# Eidolon REST API Reference

## Base URL

```
http://localhost:8888
```

When running in Wokwi simulator, the ESP32 connects to WiFi "Wokwi-GUEST" and port forwarding is configured from `localhost:8888` to `target:80`.

## Authentication

No authentication required. The API is designed for local development and testing environments.

## Headers

### Request Headers

| Header | Type | Required | Description |
|--------|------|----------|-------------|
| `Content-Type` | string | Yes (for POST) | Must be `application/json` for POST requests |

### Response Headers

| Header | Type | Description |
|--------|------|-------------|
| `Content-Type` | string | Always `application/json` |
| `Access-Control-Allow-Origin` | string | Set to `*` for CORS support |

## Error Responses

All endpoints return standard HTTP status codes:

| Status Code | Description |
|-------------|-------------|
| `200` | Success |
| `400` | Bad Request - Invalid JSON or parameters |
| `404` | Not Found - Endpoint does not exist |

### Error Response Format

```json
{
  "error": "string"
}
```

---

## Endpoints

### Get System Status

Returns the current status of the monitoring system including sensor overrides and alert level.

```http
GET /api/status
```

#### Request

##### curl

```bash
curl -L -X GET 'http://localhost:8888/api/status' \
-H 'Accept: application/json'
```

##### Python

```python
import requests

response = requests.get('http://localhost:8888/api/status')
print(response.json())
```

##### JavaScript

```javascript
const response = await fetch('http://localhost:8888/api/status');
const data = await response.json();
console.log(data);
```

#### Response

##### 200 (application/json)

Returns system status information.

##### Response Schema

| Field | Type | Description |
|-------|------|-------------|
| `status` | string | System status. Always `"online"` |
| `scenario` | string | Current scenario name. Default: `"api_control"` |
| `alert_level` | string | Current alert level: `"normal"`, `"yellow"`, or `"red"` |
| `uptime_seconds` | integer | System uptime in seconds |
| `wifi_ip` | string | ESP32 IP address on WiFi network |
| `overrides` | object | Status of sensor overrides |
| `overrides.dht22` | boolean | Whether DHT22 values are overridden |
| `overrides.ldr` | boolean | Whether LDR values are overridden |
| `overrides.mpu6050` | boolean | Whether MPU6050 values are overridden |

##### Response Example

```json
{
  "status": "online",
  "scenario": "api_control",
  "alert_level": "normal",
  "uptime_seconds": 120,
  "wifi_ip": "192.168.1.100",
  "overrides": {
    "dht22": false,
    "ldr": false,
    "mpu6050": false
  }
}
```

---

### Control DHT22 Sensor

Override temperature and/or humidity values from the DHT22 sensor.

```http
POST /api/sensors/dht22
```

#### Request

##### Request Body Schema (application/json)

| Field | Type | Required | Description | Range |
|-------|------|----------|-------------|-------|
| `temperature` | number | No | Temperature in Celsius | -40.0 to 80.0 |
| `humidity` | number | No | Relative humidity percentage | 0.0 to 100.0 |
| `disable` | boolean | No | Disable override and use real sensor | - |

##### curl

```bash
# Set temperature only
curl -L -X POST 'http://localhost:8888/api/sensors/dht22' \
-H 'Content-Type: application/json' \
-d '{
  "temperature": 25.5
}'

# Set both temperature and humidity
curl -L -X POST 'http://localhost:8888/api/sensors/dht22' \
-H 'Content-Type: application/json' \
-d '{
  "temperature": 35.0,
  "humidity": 85.0
}'

# Disable override
curl -L -X POST 'http://localhost:8888/api/sensors/dht22' \
-H 'Content-Type: application/json' \
-d '{
  "disable": true
}'
```

##### Python

```python
import requests

# Set critical temperature
response = requests.post(
    'http://localhost:8888/api/sensors/dht22',
    json={'temperature': 48.0, 'humidity': 90.0}
)
print(response.json())
```

##### JavaScript

```javascript
const response = await fetch('http://localhost:8888/api/sensors/dht22', {
  method: 'POST',
  headers: {'Content-Type': 'application/json'},
  body: JSON.stringify({
    temperature: 45.0,
    humidity: 80.0
  })
});
const data = await response.json();
```

#### Response

##### 200 (application/json)

Confirms the values that were set.

##### Response Schema

| Field | Type | Description |
|-------|------|-------------|
| `success` | boolean | Always `true` for successful requests |
| `temperature` | number | Current temperature override value |
| `humidity` | number | Current humidity override value |
| `override_active` | boolean | Whether override is currently active |

##### Response Example

```json
{
  "success": true,
  "temperature": 48.0,
  "humidity": 90.0,
  "override_active": true
}
```

##### Alert Thresholds

| Level | Temperature | Humidity | LED Color | Buzzer |
|-------|-------------|----------|-----------|---------|
| Normal | < 35°C | < 80% | Green | Off |
| Yellow | ≥ 35°C | ≥ 80% | Yellow | Off |
| Red | ≥ 45°C | ≥ 90% | Red | On |

---

### Control LDR Sensor

Override luminosity values from the LDR (Light Dependent Resistor) sensor.

```http
POST /api/sensors/ldr
```

#### Request

##### Request Body Schema (application/json)

| Field | Type | Required | Description | Range |
|-------|------|----------|-------------|-------|
| `raw_value` | integer | No | ADC raw value | 0 to 4095 |
| `disable` | boolean | No | Disable override and use real sensor | - |

##### curl

```bash
# Set dark environment (low light)
curl -L -X POST 'http://localhost:8888/api/sensors/ldr' \
-H 'Content-Type: application/json' \
-d '{
  "raw_value": 100
}'

# Set bright environment
curl -L -X POST 'http://localhost:8888/api/sensors/ldr' \
-H 'Content-Type: application/json' \
-d '{
  "raw_value": 900
}'

# Disable override
curl -L -X POST 'http://localhost:8888/api/sensors/ldr' \
-H 'Content-Type: application/json' \
-d '{
  "disable": true
}'
```

##### Python

```python
import requests

# Simulate darkness
response = requests.post(
    'http://localhost:8888/api/sensors/ldr',
    json={'raw_value': 50}
)
print(f"Lux value: {response.json()['lux']}")
```

##### JavaScript

```javascript
// Simulate normal lighting
const response = await fetch('http://localhost:8888/api/sensors/ldr', {
  method: 'POST',
  headers: {'Content-Type': 'application/json'},
  body: JSON.stringify({raw_value: 500})
});
```

#### Response

##### 200 (application/json)

Returns both raw ADC value and calculated lux.

##### Response Schema

| Field | Type | Description |
|-------|------|-------------|
| `success` | boolean | Always `true` for successful requests |
| `raw_value` | integer | Current ADC raw value override |
| `lux` | number | Calculated luminosity in lux |
| `override_active` | boolean | Whether override is currently active |

##### Response Example

```json
{
  "success": true,
  "raw_value": 100,
  "lux": 15.2,
  "override_active": true
}
```

##### Lux Conversion Reference

| Environment | Raw Value | Approximate Lux |
|-------------|-----------|-----------------|
| Complete darkness | 0-100 | 0-15 |
| Dark room | 100-300 | 15-100 |
| Normal indoor | 300-700 | 100-500 |
| Bright indoor | 700-900 | 500-900 |
| Direct light | 900-1023 | 900-1000 |

---

### Control MPU6050 Sensor

Override acceleration values from the MPU6050 accelerometer/gyroscope sensor.

```http
POST /api/sensors/mpu6050
```

#### Request

##### Request Body Schema (application/json)

| Field | Type | Required | Description | Range |
|-------|------|----------|-------------|-------|
| `accel_x` | number | No | X-axis acceleration in g | -2.0 to 2.0 |
| `accel_y` | number | No | Y-axis acceleration in g | -2.0 to 2.0 |
| `accel_z` | number | No | Z-axis acceleration in g | -2.0 to 2.0 |
| `disable` | boolean | No | Disable override and use real sensor | - |

##### curl

```bash
# Set resting state (1g on Z-axis)
curl -L -X POST 'http://localhost:8888/api/sensors/mpu6050' \
-H 'Content-Type: application/json' \
-d '{
  "accel_x": 0.0,
  "accel_y": 0.0,
  "accel_z": 1.0
}'

# Simulate strong vibration
curl -L -X POST 'http://localhost:8888/api/sensors/mpu6050' \
-H 'Content-Type: application/json' \
-d '{
  "accel_x": 1.5,
  "accel_y": -1.2,
  "accel_z": 0.8
}'

# Set only Z-axis
curl -L -X POST 'http://localhost:8888/api/sensors/mpu6050' \
-H 'Content-Type: application/json' \
-d '{
  "accel_z": 1.8
}'
```

##### Python

```python
import requests
import math

# Simulate circular motion
for angle in range(0, 360, 30):
    x = 0.5 * math.cos(math.radians(angle))
    y = 0.5 * math.sin(math.radians(angle))
    
    response = requests.post(
        'http://localhost:8888/api/sensors/mpu6050',
        json={'accel_x': x, 'accel_y': y, 'accel_z': 1.0}
    )
```

##### JavaScript

```javascript
// Simulate impact
const response = await fetch('http://localhost:8888/api/sensors/mpu6050', {
  method: 'POST',
  headers: {'Content-Type': 'application/json'},
  body: JSON.stringify({
    accel_x: 1.8,
    accel_y: 1.5,
    accel_z: 0.5
  })
});
```

#### Response

##### 200 (application/json)

Confirms the acceleration values that were set.

##### Response Schema

| Field | Type | Description |
|-------|------|-------------|
| `success` | boolean | Always `true` for successful requests |
| `accel_x` | number | Current X-axis acceleration override (g) |
| `accel_y` | number | Current Y-axis acceleration override (g) |
| `accel_z` | number | Current Z-axis acceleration override (g) |
| `override_active` | boolean | Whether override is currently active |

##### Response Example

```json
{
  "success": true,
  "accel_x": 1.5,
  "accel_y": -1.2,
  "accel_z": 0.8,
  "override_active": true
}
```

##### Alert Thresholds

Vibration magnitude is calculated as: `sqrt(x² + y² + z²)`

| Level | Magnitude | LED Color | Buzzer |
|-------|-----------|-----------|---------|
| Normal | < 1.3g | Green | Off |
| Yellow | ≥ 1.3g | Yellow | Off |
| Red | ≥ 1.7g | Red | On |

---

### Reset All Sensors

Disable all sensor overrides and return to using real sensor values.

```http
POST /api/sensors/reset
```

#### Request

##### curl

```bash
curl -L -X POST 'http://localhost:8888/api/sensors/reset' \
-H 'Content-Type: application/json' \
-d '{}'
```

##### Python

```python
import requests

response = requests.post('http://localhost:8888/api/sensors/reset', json={})
print(response.json())
```

##### JavaScript

```javascript
const response = await fetch('http://localhost:8888/api/sensors/reset', {
  method: 'POST',
  headers: {'Content-Type': 'application/json'},
  body: '{}'
});
```

#### Response

##### 200 (application/json)

Confirms that all overrides have been disabled.

##### Response Schema

| Field | Type | Description |
|-------|------|-------------|
| `success` | boolean | Always `true` for successful requests |
| `message` | string | Confirmation message |

##### Response Example

```json
{
  "success": true,
  "message": "All sensor overrides disabled"
}
```

---

## Complete Example Workflows

### Example 1: Temperature Alert Test

```bash
# 1. Check initial status
curl -L -X GET 'http://localhost:8888/api/status'

# 2. Set normal temperature
curl -L -X POST 'http://localhost:8888/api/sensors/dht22' \
-H 'Content-Type: application/json' \
-d '{"temperature": 25.0}'

# 3. Gradually increase temperature
curl -L -X POST 'http://localhost:8888/api/sensors/dht22' \
-H 'Content-Type: application/json' \
-d '{"temperature": 37.0}'  # Yellow alert

# 4. Trigger critical alert
curl -L -X POST 'http://localhost:8888/api/sensors/dht22' \
-H 'Content-Type: application/json' \
-d '{"temperature": 50.0, "humidity": 95.0}'  # Red alert

# 5. Reset to normal
curl -L -X POST 'http://localhost:8888/api/sensors/reset' \
-H 'Content-Type: application/json' \
-d '{}'
```

### Example 2: Combined Sensor Alert

```python
import requests
import time

base_url = 'http://localhost:8888/api'

# Set multiple critical conditions
requests.post(f'{base_url}/sensors/dht22', json={'temperature': 40.0})
requests.post(f'{base_url}/sensors/ldr', json={'raw_value': 50})  # Dark
requests.post(f'{base_url}/sensors/mpu6050', json={'accel_x': 1.5, 'accel_y': 1.0})

# Check system status
status = requests.get(f'{base_url}/status').json()
print(f"Alert Level: {status['alert_level']}")  # Should be "red"

# Wait and reset
time.sleep(5)
requests.post(f'{base_url}/sensors/reset', json={})
```

### Example 3: Real-time Dashboard

```html
<!DOCTYPE html>
<html>
<head>
    <title>Eidolon Control Panel</title>
</head>
<body>
    <h1>Sensor Control</h1>
    
    <div>
        <label>Temperature (°C): <span id="temp-value">25</span></label>
        <input type="range" id="temp" min="0" max="60" value="25" 
               oninput="updateSensor('dht22', {temperature: this.value})">
    </div>
    
    <div>
        <label>Humidity (%): <span id="humidity-value">60</span></label>
        <input type="range" id="humidity" min="0" max="100" value="60"
               oninput="updateSensor('dht22', {humidity: this.value})">
    </div>
    
    <div>
        <label>Light (ADC): <span id="light-value">500</span></label>
        <input type="range" id="light" min="0" max="1023" value="500"
               oninput="updateSensor('ldr', {raw_value: this.value})">
    </div>
    
    <button onclick="resetAll()">Reset All</button>
    
    <div id="status"></div>

    <script>
        const API_URL = 'http://localhost:8888/api';
        
        async function updateSensor(sensor, data) {
            const response = await fetch(`${API_URL}/sensors/${sensor}`, {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(data)
            });
            
            // Update display values
            if (data.temperature) document.getElementById('temp-value').innerText = data.temperature;
            if (data.humidity) document.getElementById('humidity-value').innerText = data.humidity;
            if (data.raw_value) document.getElementById('light-value').innerText = data.raw_value;
            
            checkStatus();
        }
        
        async function resetAll() {
            await fetch(`${API_URL}/sensors/reset`, {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: '{}'
            });
            checkStatus();
        }
        
        async function checkStatus() {
            const response = await fetch(`${API_URL}/status`);
            const status = await response.json();
            document.getElementById('status').innerHTML = 
                `<h3>Status: ${status.alert_level.toUpperCase()}</h3>`;
        }
        
        // Check status on load
        checkStatus();
    </script>
</body>
</html>
```

---

## System Behavior

### Alert Level Calculation

The system evaluates all sensor values and sets the alert level to the highest severity detected:

1. **Temperature/Humidity**: Checked against defined thresholds
2. **Vibration**: Calculated as magnitude of acceleration vector
3. **Light**: Currently informational only (no alert thresholds)

### Visual Feedback

| Alert Level | LCD Display | LED Color | Buzzer | Serial Output |
|-------------|-------------|-----------|---------|---------------|
| Normal | "api_ctrl OK" | Green | Off | `"alert_level": "normal"` |
| Yellow | "api_ctrl WARN" | Yellow | Off | `"alert_level": "yellow"` |
| Red | "api_ctrl ALERT" | Red | On | `"alert_level": "red"` |

### Serial Output Format

The system outputs JSON to serial every 2 seconds:

```json
{
  "timestamp": "2025-06-13T00:00:03Z",
  "scenario": "api_control",
  "step": 0,
  "elapsed_seconds": 3,
  "sensors": {
    "dht22": {
      "temperature": 25,
      "humidity": 60
    },
    "ldr": {
      "raw": 1001,
      "lux": 20
    },
    "mpu6050": {
      "accelX": 0,
      "accelY": 0,
      "accelZ": 1
    }
  },
  "actuators": {
    "rgb_led": "green",
    "buzzer": "off"
  },
  "alert_level": "normal"
}
```

---

## Rate Limits

No rate limiting is implemented. The system processes all requests immediately.

## Best Practices

1. **Reset Between Tests**: Always reset sensors after testing to avoid interference
2. **Gradual Changes**: For realistic simulation, change values gradually
3. **Monitor Serial**: Keep serial monitor open to see real-time system response
4. **Check Status**: Use `/api/status` to verify override states
5. **Valid Ranges**: Stay within sensor ranges to avoid unexpected behavior

## Troubleshooting

### API Not Responding

1. Check WiFi connection in serial output
2. Verify port forwarding is active
3. Ensure Wokwi simulation is running

### Values Not Changing

1. Check if override is active via `/api/status`
2. Verify JSON syntax in requests
3. Check serial output for errors

### Unexpected Alert Levels

1. Remember that highest severity wins
2. Check all sensor values, not just the one you changed
3. Magnitude calculation for vibration uses all three axes

---

## Version

API Version: 2.0  
Firmware Version: EIDOLON v2.0  
Last Updated: 2025-06-13