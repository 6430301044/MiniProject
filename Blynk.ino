//ตัวแปรที่จำเป็นสำหรับ Blynk
//ตัวแปรแรกคือ include or import everything is important for this Blynk&M5 project
#include "M5StickCPlus.h" //import libary ของ M5StickCPlus
#include "AXP192.h" //import libary ของ AXP192 ตัว manage power

//ประกาศตัวแปรคงที่ได้จากบนเว็บ Blynk
/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

/* Fill in information from Blynk Device Info here */
//#define BLYNK_TEMPLATE_ID           "TMPxxxxxx"
//#define BLYNK_TEMPLATE_NAME         "Device"
//#define BLYNK_AUTH_TOKEN            "YourAuthToken"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// WiFi 2.4 GH only
// Set auth to "" for connect networks.
// Your WiFi credentials.
// Set password to "" for open networks.
char auth[] = "YourAuthToken"
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";

// Variable for this project
int brightness = 8;

*-------------BLYNKCOMMARD-------------*/
BLYNK_WRITE(V0) {  // รับค่า Brightness จาก Widget Slider บนแอป Blynk IoT ผ่าน V0
        brightness = param.asInt();  // อ่านค่า Brightness ที่ส่งมาจากแอป Blynk IoT
        M5.Axp.ScreenBreath(brightness);  // กำหนดค่า Brightness ให้กับจอ M5StickC Plus
}

BLYNK_WRITE(V7) {
        Mode = param.asInt();
        if (Mode){
            Blynk.virtualWrite(V7, Mode);
        } else {
            Blynk.virtualWrite(V7, Mode);
        }
}

BLYNK_WRITE(V8) {
        UserWeight = param.asFloat();
        CalorieWalkBurn = StepWalk * (0.57 * UserWeight) / 1000;
        CalorieArmBurn = 0.0175 * UserWeight * SwingArm;
        CalorieWalk = CalorieWalk + CalorieWalkBurn;
        CalorieArm = CalorieArm + CalorieArmBurn;
        TotalCalorie = CalorieWalk + CalorieArm;
}

BLYNK_CONNECTED() { //ฟังก์ชั่นเช็คว่า Blynk เชื่อมต่อแล้วใช่ไหม
    Blynk.syncAll();//ทำการซิงค์ข้อมูลบนตัวบอร์ดกับ Smartphone หรือ web เพื่อให้ตรงกัน
}

void setup(){
//โค้ดที่จำเป็นต้องเซตอัปก่อนที่จะไปรันในโค้ดหลัก
    Serial.begin(9600);//เป็นตัวไว้เชื่อมกับ Serial moniter ใน Arduino IDE
    Blynk.begin(auth, ssid, pass);//Or Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass)
    //เตรียมข้อมูลให้ Blynk สำหรับ เชื่อมต่อ WiFi
    M5.begin(); //Super mega very important for everyting project with use M5Family
}

void loop(){
//โค้ดทำงานหลักที่จะให้แสดงผลบนหน้าจอและส่งค่าไปยัง Blynk
    /*--------------BLYNK--------------*/
    Blynk.run();//สั้งให้รัน Blynk เชื่อมต่อ WiFi
    float BattPower = M5.Axp.GetBatPower();//รับข้อมูล Battery
    Blynk.virtualWrite(V1, BattPower);//ส่งค่าจาก device(M5StickCPlus) ไปยัง Blynk Server เพื่อรอให้ไปแสดงผลบน Smartphone หรือ web เมื่อเรียกข้อข้อมูล
    float voltage = M5.Axp.GetBatVoltage();//รับข้อมูลแรงดัน
    float percentage = (voltage - 3.3) / 0.9 * 100;//ใช้สูตรทางคณิตศาสตร์เพื่อไปแสดงเป็นเปอร์เซนต์
    if (percentage > 100) {//สร้างเงื่อนไขให้กาะแสดงข้อมูลแบตเตอรี่
        percentage = 100;
    }
    if (percentage < 0) {
        percentage = 0;
    }
    Blynk.virtualWrite(V2, percentage);//ส่งค่าจาก device(M5StickCPlus) ไปยัง Blynk Server เพื่อรอให้ไปแสดงผลบน Smartphone หรือ web เมื่อเรียกข้อข้อมูล
}
