#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <FirebaseESP8266.h>
#include <addons/RTDBHelper.h>

/* 2. If work with RTDB, define the RTDB URL and database secret */
#define DATABASE_URL "home-automation-56421-default-rtdb.asia-southeast1.firebasedatabase.app" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define DATABASE_SECRET "4Q6QEa82fsrgqoNaVMrdgQfnD71QGkdQ6IWj7slF"

/* 3. Define the Firebase Data object */
FirebaseData fbdo;

/* 4, Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* Define the FirebaseConfig data for config data */
FirebaseConfig config;

const char* wlan_ssid             = "Tang 1 Tro 198_2.4G";
const char* wlan_password         = "khongbiet";
const char* ws_host               = "192.168.1.2";
const int   ws_port               = 8080;

FirebaseJsonArray *listDevices;

#define led1 16

const char* roomName = "q";

// base URL for SockJS (websocket) connection
// The complete URL will look something like this(cf. http://sockjs.github.io/sockjs-protocol/sockjs-protocol-0.3.3.html#section-36):
// ws://<ws_host>:<ws_port>/<ws_baseurl>/<3digits>/<randomstring>/websocket
// For the default config of Spring's SockJS/STOMP support, the default base URL is "/socketentry/".
const char* ws_baseurl            = "/iot/"; // don't forget leading and trailing "/" !!!

WebSocketsClient webSocket;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  connectToWifi();
  connectToWebSocket();
  pinMode(led1, OUTPUT);
  digitalWrite(led1, LOW);

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  connectToFirebase();
  fetchAllDevices();
}

void loop() {
  // put your main code here, to run repeatedly:
  webSocket.loop();
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:
      {
        Serial.printf("[WSc] Connected to url: %s\n",  payload);
        break;
      }
    case WStype_TEXT:
      {

        String text = (char*) payload;
        if (payload[0] == 'h') {

          Serial.println("Heartbeat!");

        } else if (payload[0] == 'o') {

          // on open connection
          char *msg = "[\"CONNECT\\naccept-version:1.1,1.0\\nheart-beat:10000,10000\\n\\n\\u0000\"]";
          webSocket.sendTXT(msg);
          delay(1000);


        } else if (text.startsWith("a[\"CONNECTED")) {

          // subscribe to some channels
          subscribeToChannel(roomName);
          delay(1000); 
        }
        else if (text.startsWith("a[\"MESSAGE")) {
          processJsonData(text);
        }
        break;
      }
  }

}

String extractString(String _received) {
  char startingChar = '{';
  char finishingChar = '}';

  String tmpData = "";
  bool _flag = false;
  for (int i = 0; i < _received.length(); i++) {
    char tmpChar = _received[i];
    if (tmpChar == startingChar) {
      tmpData += startingChar;
      _flag = true;
    }
    else if (tmpChar == finishingChar) {
      tmpData += finishingChar;
      break;
    }
    else if (_flag == true) {
      tmpData += tmpChar;
    }
  }

  return tmpData;

}

void processJsonData(String _received) {
  String json = extractString(_received);
  json.replace("\\", "");
  Serial.println(json);
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
  JsonObject obj = doc.as<JsonObject>();
  String device = obj["device"];
  int number = obj["number"];
  String status = obj["status"];
  changeDeviceStatus(device, number, status);
}

void changeDeviceStatus(String device, int number, String status) {
  Serial.println("Device number: ");
  Serial.println(number);
  if (device == "led") {
    // if (number == 1) {
    //   digitalWrite(led1, status == "on" ? HIGH : LOW);
    // }
    Serial.println("ListData number: ");
    Serial.println(listDevices->size());
    for (size_t i = 0; i < listDevices->size(); i++) {
      FirebaseJsonData firebaseData;
      listDevices->get(firebaseData, i);

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, firebaseData.to<String>());
      JsonObject obj = doc.as<JsonObject>();
      
      int id = obj["id"];
      if (id == number) {
        int pin = obj["pin"];
        digitalWrite(pin, status == "on" ? LOW : HIGH); // for led mod only
        updateDeviceStatus(i, status);
      }
    }
  }
}

void subscribeToChannel(String _channelName) {
  String msg = "[\"SUBSCRIBE\\nid:sub-0\\ndestination:/home-device/messages/" + _channelName + "\\n\\n\\u0000\"]";
  webSocket.sendTXT(msg);
}

void connectToWifi() {
  delay(500);
  Serial.print("Logging into WLAN: "); Serial.print(wlan_ssid); Serial.print(" ...");
  WiFi.begin(wlan_ssid, wlan_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" success.");
  Serial.print("IP: "); Serial.println(WiFi.localIP());
}

void connectToWebSocket() {
  String socketUrl = ws_baseurl;
  socketUrl += random(0, 999);
  socketUrl += "/";
  socketUrl += random(0, 999999); // should be a random string, but this works (see )
  socketUrl += "/websocket";
  Serial.println(ws_host + String(ws_port) + socketUrl);

  // connect to websocket
  webSocket.begin(ws_host, ws_port, socketUrl);
  webSocket.setExtraHeaders();
  //    webSocket.setExtraHeaders("foo: I am so funny\r\nbar: not"); // some headers, in case you feel funny
  webSocket.onEvent(webSocketEvent);
}

void connectToFirebase() {
   /* Assign the certificate file (optional) */
  // config.cert.file = "/cert.cer";
  // config.cert.file_storage = StorageType::FLASH;

  /* Assign the database URL and database secret(required) */
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;

  Firebase.reconnectWiFi(true);

  // The WiFi credentials are required for Pico W
  // due to it does not have reconnect feature.
  #if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    config.wifi.clearAP();
    config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
  #endif

  /* Initialize the library with the Firebase authen and config */
  Firebase.begin(&config, &auth);
}

void fetchAllDevices() {
  Serial.println("Fetch all Devices...");
  while(!Firebase.ready()) {
    Serial.println("Wait for firebase connected...");
  }

  if (Firebase.ready()) {
    if (Firebase.getArray(fbdo, "/device")) {

      if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_array) {
        listDevices = fbdo.to<FirebaseJsonArray *>();
        for (size_t i = 0; i < listDevices->size(); i++) {
          FirebaseJsonData firebaseData;
          listDevices->get(firebaseData, i);

          DynamicJsonDocument doc(1024);
          deserializeJson(doc, firebaseData.to<String>());
          JsonObject obj = doc.as<JsonObject>();
          
          int pin = obj["pin"];
          String status = obj["status"];
          pinMode(pin, OUTPUT);
          digitalWrite(pin, status == "on" ? LOW : HIGH);
        }
      } 

    } else {
      Serial.println(fbdo.errorReason());
    }
  }
}

void updateDeviceStatus(int deviceIndex, String status) {
  for (size_t i = 0; i < listDevices->size(); i++) {
    FirebaseJsonData firebaseData;
    listDevices->get(firebaseData, i);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, firebaseData.to<String>());
    JsonObject obj = doc.as<JsonObject>();  

    if (i == deviceIndex) {
      FirebaseJson updateData;
      FirebaseJson json;
      updateData.set("status",status);
      String deviceIndexString = String(deviceIndex);
      String path = "/device/" + deviceIndexString;
      Serial.println(path);
      if (Firebase.updateNode(fbdo, path, updateData)) {
        Serial.println(fbdo.dataPath());

        Serial.println(fbdo.dataType());

        Serial.println(fbdo.jsonString()); 

        fetchAllDevices();
      } else {
        Serial.println(fbdo.errorReason());
      }
    }
  }
}
