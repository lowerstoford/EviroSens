#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SensorManager.h"
#include "Config.h"

AsyncWebServer server(80);
SensorManager sensorManager;

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

void TaskWiFi(void *pvParameters);
void TaskSensorRead(void *pvParameters);

void setup() {
  Serial.begin(115200);
  
  sensorManager.begin();

  WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASSWORD);

  xTaskCreatePinnedToCore(TaskWiFi, "WiFiTask", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskSensorRead, "SensorTask", 4096, NULL, 1, NULL, 1);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/sensordata", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", sensorManager.getSensorData());
  });
}

void loop() {
  // Empty. Things are done in Tasks.
}

void TaskWiFi(void *pvParameters) {
  (void) pvParameters;
  bool serverStarted = false;

  for (;;) {
    if (WiFi.status() == WL_CONNECTED) {
      if (!serverStarted) {
        server.begin();
        Serial.println("HTTP server started");
        serverStarted = true;
      }
    } else {
      Serial.println("WiFi not connected");
      serverStarted = false;
    }
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

void TaskSensorRead(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    // The sensor data is read when getSensorData() is called
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}