#include <ESP8266WiFi.h>
#include <SocketIOClient.h>
 
SocketIOClient client;
const char* ssid = "Tang 1 Tro 198_2.4G";          //Tên mạng Wifi mà Socket server của bạn đang kết nối
const char* password = "khongbiet";  //Pass mạng wifi ahihi, anh em rãnh thì share pass cho mình với.
 
char host[] = "192.168.1.2";  //Địa chỉ IP dịch vụ, hãy thay đổi nó theo địa chỉ IP Socket server của bạn.
int port = 3484;                  //Cổng dịch vụ socket server do chúng ta tạo!
 
// từ khóa extern: dùng để #include các biến toàn cục ở một số thư viện khác. Trong thư viện SocketIOClient có hai biến toàn cục
// mà chúng ta cần quan tâm đó là
// RID: Tên hàm (tên sự kiện
// Rfull: Danh sách biến (được đóng gói lại là chuối JSON)
extern String RID;
extern String Rfull;
 
void setup()
{
    //Bật baudrate ở mức 115200 để giao tiếp với máy tính qua Serial
    Serial.begin(115200);
    delay(10);
 
    //Việc đầu tiên cần làm là kết nối vào mạng Wifi
    Serial.print("Ket noi vao mang ");
    Serial.println(ssid);
 
    //Kết nối vào mạng Wifi
    WiFi.begin(ssid, password);
 
    //Chờ đến khi đã được kết nối
    while (WiFi.status() != WL_CONNECTED) { //Thoát ra khỏi vòng 
        delay(500);
        Serial.printf("Dang ket noi wifi %s ...\n", ssid);
    }
 
    Serial.println();
    Serial.println(F("Da ket noi WiFi"));
    Serial.println(F("Dia chi IP cua ESP8266 (Socket Client ESP8266): "));
    Serial.println(WiFi.localIP());
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
    if (!client.connect(host, port)) {
        Serial.println(F("Ket noi den socket server that bai!"));
    }
 
    //Khi đã kết nối thành công
    if (client.connected()) {
        //Thì gửi sự kiện ("connection") đến Socket server ahihi.
        client.send("connection", "message", "Connected !!!!");
    }
}
 
void loop()
{
    //Khi bắt được bất kỳ sự kiện nào thì chúng ta có hai tham số:
    //  +RID: Tên sự kiện
    //  +RFull: Danh sách tham số được nén thành chuỗi JSON!
    if (client.monitor()) {
        Serial.println(RID);
        Serial.println(Rfull);

        // den led hoat dong nguoc low la bat, high la tat
        if (RID == "inactive_led") {
          digitalWrite(2, HIGH);
          Serial.println("Da tat led");
        }
        else if (RID == "active_led") {
          digitalWrite(2, LOW);
          Serial.println("Da bat led");
        }

    }
 
    //Kết nối lại!
    if (!client.connected()) {
      Serial.println("Ket noi lai socket server");
      client.reconnect(host, port);
    }
}