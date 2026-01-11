#include <WiFiS3.h>

char ssid[] = "Airtel_suma_8138";
char pass[] = "air24004";
int status = WL_IDLE_STATUS;

const char* server = "cops.snehashish.tech";
int port = 8080;
const char* path = "/sensor/quantity";

WiFiClient client;


void setup() {
  // put your setup code here, to run once:
  pinMode(2, INPUT);   // Connect HX710 OUT to Arduino pin 2
  pinMode(3, OUTPUT);  // Connect HX710 SCK to Arduino pin 3

  Serial.begin(9600);
  // IMPORTANT: remove while (!Serial) on non‑native USB boards
  delay(1000); // give Serial time to come up
  Serial.println("Booting...");

  // Connect to WiFi
  while (status != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(1000);
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  float pressureResult = readPressure();

  Serial.print("Pressure: ");
  Serial.println(pressureResult, 6);

  sendPressure(pressureResult);

  delay(5000);  // send every 5 seconds
}

float readPressure() {
  char formattedValue[8];
  // wait for the current reading to finish
  while (digitalRead(2)) {}

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

  // get the 2's complement
  result = result ^ 0x800000;

  // pulse the clock line 3 times to start the next pressure reading
  for (char i = 0; i < 3; i++) {
    digitalWrite(3, HIGH);
    digitalWrite(3, LOW);
  }
  
  float pressure = (result /100000000.0)/14.504 ;
  
  float initial_pressure = 100; // change this 
  float water_exerted_pressure = 200; // change this 
  float total_pressure = water_exerted_pressure - initial_pressure; // calculate total volume in terms of pressure
  float percentage_left = 100 - (((total_pressure - (pressure - initial_pressure)) / total_pressure) * 100); 

  return pressure; // Return the result
}

void sendPressure(float pressure) {
  if (!client.connect(server, port)) {
    Serial.println("Connection failed");
    return;
  }

  // Build JSON body
  String jsonBody = "{ \"pressure\": " + String(pressure, 6) + " }";

  // HTTP POST request
  client.print(String("POST ") + path + " HTTP/1.1\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonBody.length());
  client.println("Connection: close");
  client.println();
  client.println(jsonBody);

  Serial.println("Sent:");
  Serial.println(jsonBody);

  // Read response (optional but recommended)
  while (client.connected()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
    }
  }

  client.stop();
}
