#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <FirebaseESP8266.h>
// #include <Firebase_ESP_Client.h>
#include <addons/RTDBHelper.h>

/* 2. If work with RTDB, define the RTDB URL and database secret */
#define DATABASE_URL "home-automation-56421-default-rtdb.asia-southeast1.firebasedatabase.app" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define DATABASE_SECRET "4Q6QEa82fsrgqoNaVMrdgQfnD71QGkdQ6IWj7slF"
// #define FIREBASE_PROJECT_ID "home-automation-56421"

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

const char* roomName = "q";

const char* ws_baseurl            = "/iot/"; // don't forget leading and trailing "/" !!!

WebSocketsClient webSocket;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  connectToWifi();
  connectToWebSocket();

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
  /*
  JSON dinh dang la: 
  {
    "id":1,
    ...
  }
  nhung message gui tu server co dinh dang khac:
  [MESSAGE:abc, data: {
    "id":1
  }]
  minh can phan {
    "id":1
  }
  */
  char startingChar = '{'; // ki tu bat dau cua 1 json
  char finishingChar = '}'; // ki tu ket thuc cua 1 json
  String tmpData = ""; // data json tra ve
  bool _flag = false; // co ki hieu da gap ki tu mo dau cua json hay chua
  // duyet qua tung ki tu cua chuoi nhan duoc
  for (int i = 0; i < _received.length(); i++) {
    char tmpChar = _received[i];
    // neu ki tu hien tai bang voi ki tu mo dau json
    if (tmpChar == startingChar) {
      tmpData += startingChar;
      _flag = true; // cong cuoi tra ve voi ki tu hien tai va set co ki hieu thanh true, co nghia la nhung ki tu tiep theo se nam trong json
    }
    else if (tmpChar == finishingChar) {
      tmpData += finishingChar;
      break; // khi gap ki tu ket thuc thi dung lai
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

  // chuyen chuoi vua nhan duoc thanh json
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
  JsonObject obj = doc.as<JsonObject>();
  String device = obj["device"];
  int number = obj["number"];
  String status = obj["status"];

  changeDeviceStatus(device, number, status);
}

void updateDeviceStatus(int deviceIndex, String status) {
  // update du lieu vao firebase
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

void changeDeviceStatus(String device, int number, String status) {
  Serial.println("Device number: ");
  Serial.println(number);
  if (device == "led") {
    // if (number == 1) {
    //   digitalWrite(led1, status == "on" ? HIGH : LOW);
    // }
    Serial.println("ListData number: ");
    Serial.println(listDevices->size());
    // [a,b,c] number = a
    // duyet qua mang cac thiet bi, neu thiet bi do co id bang voi number thi thao tac bat hoac tat
    for (size_t i = 0; i < listDevices->size(); i++) {
      FirebaseJsonData firebaseData;
      listDevices->get(firebaseData, i);

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, firebaseData.to<String>());
      JsonObject obj = doc.as<JsonObject>();
      
      int id = obj["id"];
      if (id == number) {
        int pin = obj["pin"];
        digitalWrite(pin, status == "on" ? HIGH : LOW); // for led mod only
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
  
  // dang nhap vao wifi
  WiFi.begin(wlan_ssid, wlan_password);

  // cho cho wifi duoc ket noi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" success.");
  Serial.print("IP: "); Serial.println(WiFi.localIP());
}

void connectToWebSocket() {
  // tao url connect to socket server
  // base URL for SockJS (websocket) connection
  // The complete URL will look something like this(cf. http://sockjs.github.io/sockjs-protocol/sockjs-protocol-0.3.3.html#section-36):
  // ws://<ws_host>:<ws_port>/<ws_baseurl>/<3digits>/<randomstring>/websocket
  // For the default config of Spring's SockJS/STOMP support, the default base URL is "/socketentry/".
  // VD: ws://192.168.1.2:8080/iot/999/laptrinhmang/websocket
  String socketUrl = ws_baseurl;
  socketUrl += random(0, 999);
  socketUrl += "/";
  socketUrl += random(0, 999999); // should be a random string, but this works (see )
  socketUrl += "/websocket";
  Serial.println(ws_host + String(ws_port) + socketUrl);

  // ket noi to websocket
  webSocket.begin(ws_host, ws_port, socketUrl);
  webSocket.setExtraHeaders();
  // dang ky mot trinh lang nghe su kien tu server
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

  // ket noi to firebase
  Firebase.begin(&config, &auth);
}

void fetchAllDevices() {
  Serial.println("Fetch all Devices...");
  // cho cho firebase duoc ketnoi
  while(!Firebase.ready()) {
    Serial.println("Wait for firebase connected...");
  }

  if (Firebase.ready()) {
    // lay mang du lieu device va gan cho bien fbdo
    if (Firebase.getArray(fbdo, "/device")) {

      if (fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_array) {
        listDevices = fbdo.to<FirebaseJsonArray *>();
        // duyet qua tung phan tu, sau do set tin hieu mac dinh cho den, vd: khi trang thai cua den so 1 tren server la on, thi mach se tu dong bat den so 1 khi khoi tao
        for (size_t i = 0; i < listDevices->size(); i++) {
          FirebaseJsonData firebaseData;
          listDevices->get(firebaseData, i);

          DynamicJsonDocument doc(1024);
          deserializeJson(doc, firebaseData.to<String>());
          JsonObject obj = doc.as<JsonObject>();
          
          int pin = obj["pin"];
          String status = obj["status"];
          pinMode(pin, OUTPUT);
          digitalWrite(pin, status == "on" ? HIGH : LOW);
        }
      } 

    } else {
      Serial.println(fbdo.errorReason());
    }
  }
}


