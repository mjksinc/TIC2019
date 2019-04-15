/*
  Azure IoT for Arduino MKR NB 1500

  Author: Matt Sinclair (@mjksinc)

  This sketch securely connects to either Azure IoT Hub or IoT Central using MQTT over NB-IoT/Cat-M1.
  The native NBSSL library is used to securley connect to the Hub, then Username/Password credentials
  are used to authenticate.

  BEFORE USING:
  - Ensure that SECRET_BROKER and CONN_STRING in arduino_secrets.h are completed
  - Change msgFreq as desired
  - Check ttl to change the life of the SAS Token for authentication with IoT Hub

  If using IoT Central:
  - Follow these intructions to find the connection details for a real device: https://docs.microsoft.com/en-us/azure/iot-central/tutorial-add-device#get-the-device-connection-information
  - Generate a connection string from this website: https://dpscstrgen.azurewebsites.net/

  Full Intructions available here: https://github.com/mjksinc/TIC2019

*/

// Libraries to include in the code
#include <ArduinoMqttClient.h>
#include <MKRNB.h>
#include "./base64.h"
#include "./Sha256.h"
#include "./utils.h"

// Additional file secretly stores credentials
#include "arduino_secrets.h"

// Enter your sensitive data in arduino_secrets.h
const char broker[] = SECRET_BROKER;
// CONN_STRING: connection string from Hub;

String iothubHost;
String deviceId;
String sharedAccessKey;

int msgFreq = 20000; //Message Frequency in millisecods
long ttl = 864000; //Time-to-live for SAS Token (seconds) i.e. 864000 = 1 day (24 hours)

NB nbAccess;
GPRS gprs;
NBSSLClient sslClient;
MqttClient mqttClient(sslClient);
NBScanner scannerNetworks;

unsigned long lastMillis = 0;

/*
 * Establish connection to cellular network, and parse/augment connection string to generate credentials for MQTT connection
 * This only allocates the correct variables, connection to the IoT Hub (MQTT Broker) happens in loop()
 */
void setup() {
  Serial.begin(9600);
  while (!Serial); //Comment out this line so the board will run automatically without opening serial port

  Serial.println("******Splitting Connection String - STARTED*****");
  splitConnectionString();

  //Connects to network to use getTime()
  connectNB();

  Serial.println("******Create SAS Token - STARTED*****");
  // create SAS token and user name for connecting to MQTT broker
  String url = iothubHost + urlEncode(String("/devices/" + deviceId).c_str());
  char *devKey = (char *)sharedAccessKey.c_str();
  long expire = getTime() + ttl;
  String sasToken = createIotHubSASToken(devKey, url, expire);
  String username = iothubHost + "/" + deviceId + "/api-version=2018-06-30";

  Serial.println("******Create SAS Token - COMPLETED*****");
  
  // Set the client id used for MQTT as the device id
  mqttClient.setId(deviceId);
  mqttClient.setUsernamePassword(username, sasToken);

  // Set the message callback, this function is
  // called when the MQTTClient receives a message
  mqttClient.onMessage(onMessageReceived);
}

/*
 * Connect to Network (if not already connected) and establish connection the IoT Hub (MQTT Broker). Messages will be sent every 30 seconds, and will poll for new messages
 * on the "devices/{deviceId}/messages/devicebound/#" topic
 * This also calls publishMessage() to trigger the message send
 */
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

  // publish a message roughly every 30 seconds.
  if (millis() - lastMillis > msgFreq) {
    lastMillis = millis();

    publishMessage();
  }
}

/*
 * Gets current Linux Time in seconds for enabling timing of SAS Token
 */
unsigned long getTime() {
  // get the current time from the cellular module
  return nbAccess.getTime();
}

/*
 * Handles the connection to the NB-IoT Network
 */
void connectNB() {
  Serial.println("\n******Connecting to Cellular Network - STARTED******");

  while ((nbAccess.begin() != NB_READY) ||
         (gprs.attachGPRS() != GPRS_READY)) {
    // failed, retry
    Serial.print(".");
    delay(1000);
  }

  Serial.println("******Connecting to Cellular Network - COMPLETED******");
  Serial.println();
}

/*
 * Establishses connection with the MQTT Broker (IoT Hub)
 * Some errors you may receive:
 * -- (-.2) Either a connectivity error or an error in the url of the broker
 * -- (-.5) Check credentials - has the SAS Token expired? Do you have the right connection string copied into arduino_secrets?
 */
void connectMQTT() {
  Serial.print("Attempting to connect to MQTT broker: ");
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
  mqttClient.subscribe("devices/" + deviceId + "/messages/devicebound/#"); //This is for cloud-to-device messages
  mqttClient.subscribe("$iothub/methods/POST/#"); //This is for direct methods + IoT Central commands
  
}

/*
 * Calls getMeasurement() to read sensor measurements (currently simulated)
 * Prints message to the MQTT Client
 */
void publishMessage() {
  Serial.println("Publishing message");

  String newMessage = getMeasurement();

  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage("devices/" + deviceId + "/messages/events/");
  mqttClient.print(newMessage);
  mqttClient.endMessage();
}

/*
 * Creates the measurements. This currently simulates and structures the data. Any sensor-reading functions would be placed here
 */
String getMeasurement() {
  long temp = random(10,30);
  long humidity = random(80,99);

  // Begin network scan to get signal strength
  scannerNetworks.begin();

  String signalStrength = scannerNetworks.getSignalStrength();

  String formattedMessage = "{\"temperature\": ";
  formattedMessage += temp;

  formattedMessage += ", \"humidity\": ";
  formattedMessage += humidity;

  formattedMessage += ", \"sigstrength\": ";
  formattedMessage += signalStrength;
  
  formattedMessage += "}";

  Serial.println(formattedMessage);
  return formattedMessage;
}

/*
 * Handles the messages received through the subscribed topic and prints to Serial
 */
void onMessageReceived(int messageSize) {

  String topic = mqttClient.messageTopic();
  
  // when receiving a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(topic);
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }
  Serial.println();

  // Responds with confirmation to direct methods and IoT Central commands 
  if (topic.startsWith(F("$iothub/methods"))) {
    String msgId = topic.substring(topic.indexOf("$rid=") + 5);

    String responseTopic = "$iothub/methods/res/200/?$rid=" + msgId; //Returns a 200 received message

    mqttClient.beginMessage(responseTopic);
    mqttClient.print("");
    mqttClient.endMessage(); 
  }

  
}

/*
 * Split the connection string into individual parts to use as part of MQTT connection setup
 */
void splitConnectionString() {
    String connStr = CONN_STRING;
    int hostIndex = connStr.indexOf("HostName=");
    int deviceIdIndex = connStr.indexOf(F(";DeviceId="));
    int sharedAccessKeyIndex = connStr.indexOf(";SharedAccessKey=");
    iothubHost = connStr.substring(hostIndex + 9, deviceIdIndex);
    deviceId = connStr.substring(deviceIdIndex + 10, sharedAccessKeyIndex);
    sharedAccessKey = connStr.substring(sharedAccessKeyIndex + 17);
    Serial.print("******Splitting Connection String - COMPLETED*****");
}

/*
 * Build a SAS Token to be used as the MQTT authorisation password
 */
String createIotHubSASToken(char *key, String url, long expire){
    url.toLowerCase();
    String stringToSign = url + "\n" + String(expire);
    int keyLength = strlen(key);

    int decodedKeyLength = base64_dec_len(key, keyLength);
    char decodedKey[decodedKeyLength];

    base64_decode(decodedKey, key, keyLength);

    Sha256 *sha256 = new Sha256();
    sha256->initHmac((const uint8_t*)decodedKey, (size_t)decodedKeyLength);
    sha256->print(stringToSign);
    char* sign = (char*) sha256->resultHmac();
    int encodedSignLen = base64_enc_len(HASH_LENGTH);
    char encodedSign[encodedSignLen];
    base64_encode(encodedSign, sign, HASH_LENGTH);
    delete(sha256);

    return "SharedAccessSignature sr=" + url + "&sig=" + urlEncode((const char*)encodedSign) + "&se=" + String(expire);
}
