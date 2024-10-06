#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <BH1750.h>
#include <Adafruit_MPU6050.h>

#define LED_BUILTIN 2
#define LED_RED 19
#define LED_GREEN 18
#define LED_BLUE 5
#define BUTTON_A 17
#define I2C_SDA 21
#define I2C_SCL 22

const char* ssid = "The Orchard Glamping";
const char* password = "AppleGreen";

WebServer server(80);
Adafruit_BME680 bme;
BH1750 lightMeter;
Adafruit_MPU6050 mpu;

unsigned long lastSensorUpdate = 0;
const long sensorUpdateInterval = 1000;  // Update sensor data every 5 seconds

String sensorData = "";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32 Sensor Pack</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h2>ESP32 Sensor Pack</h2>
  <div id="sensorData">Loading sensor data...</div>
  <script>
    setInterval(function() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("sensorData").innerHTML = this.responseText.replace(/\n/g, '<br>');
        }
      };
      xhttp.open("GET", "/sensordata", true);
      xhttp.send();
    }, 5000);
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON_A, INPUT_PULLUP);

  if (!bme.begin()) {
    Serial.println("Could not find BME680 sensor!");
    setRGBColor(255, 0, 0); // Red for error
    while (1);
  }

  lightMeter.begin();

  if (!mpu.begin()) {
    Serial.println("Could not find MPU6050 sensor!");
    setRGBColor(255, 0, 0); // Red for error
    while (1);
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });

  server.on("/sensordata", HTTP_GET, []() {
    server.send(200, "text/plain", sensorData);
  });

  server.begin();
  Serial.println("HTTP server started");

  setRGBColor(0, 255, 0); // Green for successful setup
}

void loop() {
  server.handleClient();

  unsigned long currentMillis = millis();
  if (currentMillis - lastSensorUpdate >= sensorUpdateInterval) {
    lastSensorUpdate = currentMillis;
    updateSensorData();
  }

  // Blink built-in LED to show the loop is running
  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);
  delay(50);
}

void updateSensorData() {
  String data = "";
  
  if (bme.performReading()) {
    data += "Temperature: " + String(bme.temperature) + " °C\n";
    data += "Pressure: " + String(bme.pressure / 100.0) + " hPa\n";
    data += "Humidity: " + String(bme.humidity) + " %\n";
    data += "Gas: " + String(bme.gas_resistance / 1000.0) + " KOhms\n";
  }

  float lux = lightMeter.readLightLevel();
  data += "Light: " + String(lux) + " lx\n";

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  data += "Accel: X:" + String(a.acceleration.x) + " Y:" + String(a.acceleration.y) + " Z:" + String(a.acceleration.z) + " m/s^2\n";
  data += "Gyro: X:" + String(g.gyro.x) + " Y:" + String(g.gyro.y) + " Z:" + String(g.gyro.z) + " rad/s\n";

  sensorData = data;
}

void setRGBColor(int red, int green, int blue) {
  analogWrite(LED_RED, red);
  analogWrite(LED_GREEN, green);
  analogWrite(LED_BLUE, blue);
}