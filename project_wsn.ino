#include <ESP8266WiFi.h>
#include <Servo.h>

#define MAX_COUNT 3

int count = 0;
int flag=1;

const char* ssid = "Coco";      // Your WiFi SSID
const char* password = "NomNomNoo";  // Your WiFi password

Servo servo;
int servoPin = 14;  // GPIO pin for the servo (D5)

int enter_TRIG = 16;  // GPIO pin for entrance ultrasonic sensor trigger (D0)
int enter_ECHO = 13;  // GPIO pin for entrance ultrasonic sensor echo (D7)
int exit_TRIG = 2;    // GPIO pin for exit ultrasonic sensor trigger (D4)
int exit_ECHO = 15;   // GPIO pin for exit ultrasonic sensor echo (D8)

int redLED = 4;  // GPIO pin for red LED (D2)
int greenLED = 5;  // GPIO pin for green LED (D1)
WiFiServer server(80);

void setup() {
  Serial.begin(1200);  // Start serial communication at 1200 baud rate
  servo.attach(servoPin);
  moveServo(0);
  pinMode(enter_TRIG, OUTPUT);
  pinMode(enter_ECHO, INPUT);
  pinMode(exit_TRIG, OUTPUT);
  pinMode(exit_ECHO, INPUT);

  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  server.begin();
  Serial.println("Server started at IP: " + WiFi.localIP().toString());
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    handleClient(client);
  }

  checkParking();
  // delay(1);
}

void checkParking() {
  int entranceDistance = 20;
  if(getDistance(enter_TRIG, enter_ECHO)<20) entranceDistance=getDistance(enter_TRIG, enter_ECHO);
  int exitDistance = 20;
  if(getDistance(exit_TRIG, exit_ECHO)<20) exitDistance=getDistance(exit_TRIG, exit_ECHO);

  Serial.print("Entrance Distance: ");
  Serial.println(entranceDistance);
  Serial.print("Exit Distance: ");
  Serial.println(exitDistance);

  if (entranceDistance < exitDistance) {
    manageEntry();
    delay(3000);
  } else if(entranceDistance > exitDistance) {
    manageExit();
    delay(3000);
  }
  else{
    Serial.println("No Entry exit detected.");
    delay(3000);
  }
}

int getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3000);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(3000);
  digitalWrite(trigPin, LOW);
  return pulseIn(echoPin, HIGH) / 58;  // Convert to centimeters
}

void manageEntry() {
    flag=0;
  if (count < MAX_COUNT) {
    count++;
    Serial.print("Occupied slots = ");
    Serial.println(count);
    moveServo(180);  // Open barrier
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
    if(count==MAX_COUNT){
    Serial.print("Entry is ");
    Serial.println(count);
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    Serial.println("Parking Full");
    }
  }
  else if(count==MAX_COUNT){
    Serial.print("Entry is ");
    Serial.println(count);
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    Serial.println("Parking Full: Ghar jao, Bye Bye :)");
  }
  delay(3000);
  moveServo(0);  // Open barrier

}

void manageExit() {
  if (count > 0) {
    count--;
    flag=0;
    Serial.print("Occupied slots = ");
    Serial.println(count);
    // moveServo(0);  // Close barrier
    
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
    
    moveServo(180);  // Open barrier
  } else if(count<0 || (count==0 && flag==0)){
    Serial.println("Occupied none: Ghost exit detected");
  }
  delay(3000);
  moveServo(0);  // Open barrier
}

void moveServo(int angle) {
  servo.write(angle);
  delay(2000);  // Wait for the servo to move (milisec)
}



void handleClient(WiFiClient& client) {
  String responseHTML = "<!DOCTYPE html><html><head><title>Parking Status</title></head><body>";
  
  responseHTML += "<h1>Parking Status</h1>";
  responseHTML += "<p>Available Slots: " + String(MAX_COUNT - count) + "</p>";
  responseHTML += "<p>Occupied Slots: " + String(count) + "</p>";
    // Add JavaScript for auto-refresh
  responseHTML += "<script>setTimeout(function(){location.reload()}, 5000);</script>";
  responseHTML += "</body></html>";

  // Send HTTP response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  
  client.println();
  client.println(responseHTML);
  
  // Give some time to send the data
  delay(1);
}
