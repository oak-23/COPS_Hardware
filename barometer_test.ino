#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// Connect Arduino to WiFi
const char* ssid = "YOUR_SSID"; // change this
const char* password = "YOUR_PASSWORD"; // change this

// modify server name
const char* serverUrl = "http://cops/endpoint"; // Replace with your server details

WiFiClient client;

void setup() {
  pinMode(2, INPUT);   // Connect HX710 OUT to Arduino pin 2
  pinMode(3, OUTPUT);  // Connect HX710 SCK to Arduino pin 3
  Serial.begin(9600);

  WiFi.mode(WIFI_STA); // Use WIFI_STA for client mode, WIFI_AP for Access Point, WIFI_AP_STA for both

  // Start the Wi-Fi connection
  WiFi.begin(ssid, password);
  Serial.println("Connecting to ");
  Serial.println(ssid);

  // Connect to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("."); // Print dots to show progress
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  // Reading pressure
  while (digitalRead(2)) {} // wait for the current reading to finish

  // read 24 bits
  long result = 0;
  for (int i = 0; i < 24; i++) {
    digitalWrite(3, HIGH);
    digitalWrite(3, LOW);
    result = result << 1;
    if (digitalRead(2)) {
      result++;
    }
  }

  // get the 2s compliment
  result = result ^ 0x800000;

  // pulse the clock line 3 times to start the next pressure reading
  for (char i = 0; i < 3; i++) {
    digitalWrite(3, HIGH);
    digitalWrite(3, LOW);
  }

  float pressure = (result /100000000.0)/14.504 ;
  
  // display pressure
  Serial.println(pressure);

  // Calculate Percentage
    float initial_pressure = 100; // change this 
    float water_exerted_pressure = 200; // change this 
    float total_pressure = water_exerted_pressure - initial_pressure; // calculate total volume in terms of pressure
    float percentage_left = 100 - (((total_pressure - (pressure - initial_pressure)) / total_pressure) * 100); 

  // for sending to server
  if (status == WL_CONNECTED) {
    // declare http object
    HTTPClinet http;
    http.begin(server); // Specify your server address and endpoint
    http.addHeader("Content-Type", "application/json"); // Important header

    // Initialise Json file
    DynamicJsonDocument doc(200);
    doc["pressure"] = pressure;
    doc["percentage_left"] = percentage_left;
    String requestBody;
    serializeJsonPretty(doc, requestBody);
  }
    // Send post request
    int httpResponseCode = http.POST(requestBody);

    // Testing statements (debugging)
    if (httpResponseCode > 0) {
      Serial.printf("Posted Json:c", httpResponseCode);
      String response = http.getString();
      Serial.println(response);
    } else {
      Serial.printf("Error: ", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
  }

  delay(5000); // Post every 5 seconds
}
