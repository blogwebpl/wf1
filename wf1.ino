
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h"

const char* ssid = SSID;
const char* wifi_password = WIFI_PASSWORD;

const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT;
const char* mqtt_client_id = MQTT_CLIENT_ID;
const char* mqtt_user =   MQTT_USER;
const char* mqtt_password =   MQTT_PASSWORD;
int mqtt_will_qos = MQTT_WILL_QOS;
const char* mqtt_will_topic = MQTT_WILL_TOPIC;
const char* mqtt_will_message = MQTT_WILL_MESSAGE;
boolean mqtt_will_retain = MQTT_WILL_RETAIN;
boolean mqtt_clean_session = MQTT_CLEAN_SESSION;

unsigned long last_mqtt_reconnect = 0;
unsigned long last_wifi_reconnect = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if (strcmp(topic,"home/blind/remote") == 0) {
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  }
}

void mqtt_connect() {
  Serial.println("Attempting MQTT connection");
  if (client.connect(mqtt_client_id, mqtt_user, mqtt_password, mqtt_will_topic, mqtt_will_qos, mqtt_will_retain, mqtt_will_message, mqtt_clean_session)) {
    Serial.println("MQTT connected");
    client.publish("wf1/status", "online", true);
    client.subscribe("home/blind/remote");
  } else {
    Serial.print("MQTT failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
  }
}

void wifi_connect() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_mqtt() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);
}

void setup_serial_port() {
  Serial.begin(19200);
}


void setup() {
  setup_serial_port();
  setup_mqtt();
}

void loop() {
  unsigned long current_millis = millis();

  if ((WiFi.status() != WL_CONNECTED) && (current_millis - last_wifi_reconnect > 30000)) {
    wifi_connect();
    last_wifi_reconnect = current_millis;
  }
  if ((!client.connected()) && (WiFi.status() == WL_CONNECTED) && (current_millis - last_mqtt_reconnect > 5000)) {
    mqtt_connect();
    last_mqtt_reconnect = current_millis;
  }
  client.loop();
}
