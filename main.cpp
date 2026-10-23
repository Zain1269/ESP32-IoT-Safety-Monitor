#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

#define DHT_PIN 33
#define DHT_TYPE DHT11
#define LDR_PIN 32
#define BUZZER_PIN 25

DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);

float temperature = 0.0;
float humidity = 0.0;
int ldrRaw = 0;
int lightPercent = 0;
bool alarm = false;

void readSensors() {
    float newTemperature = dht.readTemperature();
    float newHumidity = dht.readHumidity();

    ldrRaw = analogRead(LDR_PIN);
    lightPercent = map(ldrRaw, 0, 4095, 0, 100);
    lightPercent = constrain(lightPercent, 0, 100);

    if (!isnan(newTemperature)) temperature = newTemperature;
    if (!isnan(newHumidity)) humidity = newHumidity;

    alarm = temperature >= 40.0;
    digitalWrite(BUZZER_PIN, alarm ? HIGH : LOW);

    Serial.println("============================");
    Serial.printf("Temperature: %.2f °C\n", temperature);
    Serial.printf("Humidity: %.2f %%\n", humidity);
    Serial.printf("LDR Raw: %d\n", ldrRaw);
    Serial.printf("Light: %d %%\n", lightPercent);
    Serial.printf("System: %s\n", alarm ? "ALERT" : "SAFE");
    Serial.println("============================");
}

void handleRoot() {
    String statusText = alarm ? "ALERT" : "SAFE";
    String statusClass = alarm ? "alert" : "safe";

    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="refresh" content="3">
<title>ESP32 IoT Safety Monitor</title>
<style>
*{box-sizing:border-box}
body{margin:0;padding:25px;font-family:Arial,sans-serif;background:#f2f4f7;color:#222}
.container{max-width:900px;margin:auto}
h1{text-align:center;margin-bottom:30px}
.cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:20px}
.card{background:white;padding:25px;border-radius:15px;text-align:center;box-shadow:0 4px 12px rgba(0,0,0,.10)}
.card h2{margin-top:0;font-size:20px}
.value{font-size:32px;font-weight:bold;margin-top:15px}
.unit{font-size:18px;color:#777}
.status{margin-top:25px;padding:22px;border-radius:15px;text-align:center;font-size:28px;font-weight:bold}
.safe{background:#d4edda;color:#155724}
.alert{background:#f8d7da;color:#721c24}
.info{margin-top:20px;padding:20px;background:white;border-radius:15px;text-align:center;box-shadow:0 4px 12px rgba(0,0,0,.08)}
.footer{text-align:center;margin-top:30px;color:#777;font-size:14px}
</style>
</head>
<body>
<div class="container">
<h1>ESP32 IoT Safety Monitor</h1>
<div class="cards">
<div class="card"><h2>Temperature</h2><div class="value">
)rawliteral";
    html += String(temperature, 1);
    html += R"rawliteral(<span class="unit"> °C</span></div></div>

<div class="card"><h2>Humidity</h2><div class="value">
)rawliteral";
    html += String(humidity, 1);
    html += R"rawliteral(<span class="unit"> %</span></div></div>

<div class="card"><h2>Light Level</h2><div class="value">
)rawliteral";
    html += String(lightPercent);
    html += R"rawliteral(<span class="unit"> %</span></div></div>
</div>

<div class="status )rawliteral";
    html += statusClass;
    html += R"rawliteral(">SYSTEM STATUS: )rawliteral";
    html += statusText;
    html += R"rawliteral(</div>

<div class="info">
<p><strong>Raw LDR Value:</strong> )rawliteral";
    html += String(ldrRaw);
    html += R"rawliteral(</p>
<p><strong>Refresh Rate:</strong> 3 seconds</p>
</div>

<div class="footer">ESP32 DevKit • IoT Safety Monitor</div>
</div>
</body>
</html>
)rawliteral";

    server.send(200, "text/html", html);
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(LDR_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    dht.begin();

    Serial.println();
    Serial.println("==============================");
    Serial.println("ESP32 IoT Safety Monitor");
    Serial.println("==============================");

    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Wi-Fi connected!");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.begin();

    Serial.println("Web server started!");
    Serial.println("Open the IP address above in your browser.");
}

void loop() {
    server.handleClient();
    readSensors();
    delay(2000);
}
