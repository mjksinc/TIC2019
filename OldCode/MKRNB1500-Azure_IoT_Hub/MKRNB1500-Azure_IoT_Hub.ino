/*
  Azure IoT Hub - SaS Token

  This sketch securely connects to an Azure IoT Hub using MQTT over NB IoT/LTE Cat M1.
  the native NBSSL library is used to securley connect to the Hub, then Username/Password credentials
  are used to authenticate.

  It publishes a message every 5 seconds to "devices/{deviceId}/messages/events/" topic
  and subscribes to messages on the "devices/{deviceId}/messages/devicebound/#"
  topic.
*/

// Libraries to include in the code
#include <ArduinoMqttClient.h>
#include <MKRNB.h>

// Additional file secretly stores credentials
#include "arduino_secrets.h"

// Enter your sensitive data in arduino_secrets.h
const char broker[] = SECRET_BROKER;
String deviceId = SECRET_DEVICE_ID;


NB nbAccess;
GPRS gprs;
NBSSLClient sslClient;
MqttClient mqttClient(sslClient);

unsigned long lastMillis = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Set the client id used for MQTT as the device id
  mqttClient.setId(deviceId);

  // Set the username to the correct format and includes password
  String username;

  username += broker;
  username += "/";
  username += deviceId;
  username += "/api-version=2018-06-30";

  String password = SECRET_PASS;

  mqttClient.setUsernamePassword(username, password);

  // Set the message callback, this function is
  // called when the MQTTClient receives a message
  mqttClient.onMessage(onMessageReceived);
}

void loop() {
  if (nbAccess.status() != NB_READY || gprs.status() != GPRS_READY) {
    connectNB();
  }

  if (!mqttClient.connected()) {
    // MQTT client is disconnected, connect
    connectMQTT();
  }

  // poll for new MQTT messages and send keep alives
  mqttClient.poll();

  // publish a message roughly every 10 seconds.
  if (millis() - lastMillis > 30000) {
    lastMillis = millis();

    publishMessage();
  }
}

unsigned long getTime() {
  // get the current time from the cellular module
  return nbAccess.getTime();
}

void connectNB() {
  Serial.println("Attempting to connect to the cellular network");

  while ((nbAccess.begin() != NB_READY) ||
         (gprs.attachGPRS() != GPRS_READY)) {
    // failed, retry
    Serial.print(".");
    delay(1000);
  }

  Serial.println("You're connected to the cellular network");
  Serial.println();
}

void connectMQTT() {
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  while (!mqttClient.connect(broker, 8883)) {
    // failed, retry
    Serial.print(".");
    Serial.println(mqttClient.connectError());
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe("devices/" + deviceId + "/messages/devicebound/#");
}

void publishMessage() {
  Serial.println("Publishing message");

  String newMessage = getMeasurement();

  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage("devices/" + deviceId + "/messages/events/");
  mqttClient.print(newMessage);
  mqttClient.endMessage();
}

String getMeasurement() {
  long temp = random(10,30);
  long humidity = random(80,99);

  String formattedMessage = "{\"temperature\": ";
  formattedMessage += temp;
  formattedMessage += "";

  formattedMessage += ", \"humidity\": ";
  formattedMessage += humidity;
  formattedMessage += "}";

  return formattedMessage;
}

void onMessageReceived(int messageSize) {
  // when receiving a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();

  Serial.println();
}
