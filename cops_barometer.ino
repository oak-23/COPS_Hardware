#include "HX710AB.h"
#include <WiFiNINA.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

//  adjust pins to your setup
uint8_t DOUT = 2; // sensor data pin
uint8_t SCLK = 3; // sensor clock pin

HX710B barometer(DOUT, SCLK); // create a barometer class

// wifi connection
char ssid[] = "test"; // wifi name
char pass[] = ""; // wifi password

int status = WL_IDLE_STATUS;

// modify server name
char server[] = "www.cops.com";

WiFiClient client;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // Initialise barometer
  barometer.begin();


  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);

    // Initialise WiFi
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // create ipaddress objects
  IPAddress ip = WiFi.localIP();
  IPAddress gateway = WiFi.gatewayIP();
  Serial.print("IP Address: ");
  Serial.println(ip);


  // Can't calibrate like this because ours is hx710B 

  // barometer.set_scale(2280.f);
  // barometer.tare();
}

void loop() {
  // put your main code here, to run repeatedly:

  // for pressure sensor
  if (barometer.is_ready()) {
    // Get pressure in some units
    float pressure_measured = barometer.read(false);


    // Testing statements (debugging)
    Serial.print("Pressure: ");
    Serial.println(pressure_measured, 2); // Print with 2 decimal places

    // Calculate Percentage
    float initial_pressure = 100; // change this 
    float water_exerted_pressure = 200; // change this 
    float total_pressure = water_exerted_pressure - initial_pressure; // calculate total volume in terms of pressure
    float percentage_left = 100 - (((total_pressure - pressure_measured) / total_pressure) * 100); 

  } else {
    Serial.println("Pressure sensor not found.");
  }

  // for sending to server

  // data to send to server
  postData = postVariable + percentage_left;

  if (status == WL_CONNECTED) {
    // declare http object
    HTTPClinet http;
    http.begin(server); // Specify your server address and endpoint
    http.addHeader("Content-Type", "application/json"); // Important header

    // Initialise Json file
    DynamicJsonDocument doc(1024);
    doc["pressure"] = pressure;
    doc["percentage_left"] = percentage_left;
    String httpRequestData;
    serializeJsonPretty(doc, httpRequestData);
  }
    // Send post request
    int httpResponseCode = http.POST(httpRequestData);

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
