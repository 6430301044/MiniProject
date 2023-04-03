//ตัวแปรที่จำเป็นสำหรับ Watch
//ตัวแปรแรกคือ include or import everything is important for this watch project
#include "M5StickCPlus.h" 
#include "AXP192.h" 
#include "7seg70.h" 
#include "ani.h"

//ประกาศให้ตัวแปรเป็นค่าคงที่
#define grey 0x65DB // ประกาศว่า grey คือสี 0x65DB สีเทานะแหละ

// Variable for this project
RTC_TimeTypeDef RTC_TimeStruct;//ประกาศตัวแปร เซตเวลา ชื่อ RTC_TimeStruct ชนิดตัวแปรเป็น RTC_TimeTypeDef
RTC_DateTypeDef RTC_DateStruct;//ประกาศตัวแปร เซตวันเดือนปี ชื่อ RTC_DateStruct ชนิดตัวแปรเป็น RTC_DateTypeDef
int H = 0;
int M = 0;

String ho = "";
String mi = "";
String se = "";

String days[7] = {"Monday", "Tuesday", "Wednesday"
  , "Thursday", "Friday", "Saturday", "Sunday"};

bool inv = 0;

int frame = 0;

void setup(){
//โค้ดที่จำเป็นต้องเซตอัปก่อนที่จะไปรันในโค้ดหลัก
    M5.begin();//สั่งให้ M5 เริ่มทำงาน [Super mega very important for everything project with use M5Family]
    M5.Lcd.setRotation(3); // สั้งให้หมุนตำแหน่ง x,y ไป 270 องศา
    M5.Lcd.fillScreen(BLACK); //สั่งให้พื้นหลังสีดำ
    M5.Lcd.setSwapBytes(true); //สั่งให้สลับ byte ในโปรเจกต์นี้ใช้สำหรับกำหนดสี เพื่อให้สีมีความถูกต้องเพิ่มมากขึ้นเพื่อไปแสดงบนจอ LCD
    M5.Lcd.setTextSize(1); //สั้งให้เซตขนาดข้อความ

    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);//เซตสีหลักของโปรเจกต์
    M5.Axp.EnableCoulombcounter();
    /*เพื่อให้สารารถตรวจจับกระแสภายใขของชิปAXP192 ที่มีหน้าที่ในการควบคุมพลังงานภายในบอร์ด 
    เพื่อติดตามการใช้พลังงานของบอร์ดเพื่อนำไปใช้ประโยชน์ต่อเช่นบันทึกการใช้พลังงานในแต่ละรอบ*/

    RTC_TimeTypeDef TimeStruct;
    TimeStruct.Hours = 4;
    TimeStruct.Minutes = 27;
    TimeStruct.Seconds = 0;
    //M5.Rtc.SetTime(&TimeStruct);  //สำหรับตั้งค่าเวลาลงบนตัว RTC
    RTC_DateTypeDef DateStruct;
    DateStruct.WeekDay = 7;
    DateStruct.Month = 4;
    DateStruct.Date = 2;
    DateStruct.Year = 2023;
    //M5.Rtc.SetData(&DateStruct); //สำหรับตั้งค่าวันเดือนปีลงบนตัว RTC

}

void loop(){
//โค้ดทำงานหลักที่จะให้แสดงผลบนหน้าจอและส่งค่าไปยัง Blynk
    M5.update(); //เพื่อให้อ่านค่าจากการกดปุ่ม
   /*-----------WATCH-----------*/
    M5.Rtc.GetTime(&RTC_TimeStruct);//รับค่าเวลา
    M5.Rtc.GetData(&RTC_DateStruct);
    M5.Lcd.setCursor(0, 15);//เซตตำแหน่ง
    M5.Lcd.setTextFont(0);//เซตฟอนต์

    M5.Lcd.drawString(String(M5.Axp.GetBatVoltage()) + " V  ", 114, 3); //แสดงแรงดัน
    M5.Lcd.setFreeFont(&DSEG7_Classic_Regular_64);//เซตฟอนต์ให้แรงดัน

    //กำหนดเงื่อนไขในการแสดงผลบนจอ LCD ของเวลา
    if (H != int(RTC_TimeStruct.Hours) || M != int(RTC_TimeStruct.Minutes)) {
        ho = String(RTC_TimeStruct.Hours);//กำหนดค่าที่ได้จาก RTC ให้เป็นรูปแบบสตริง
        if (ho.length() < 2) //ถ้าความยาวสตริงเกิน 2 เป็นจริงให้รีเซ็ตค่ากลับไปที่ 0
            ho = "0" + ho;

        mi = String(RTC_TimeStruct.Minutes);
        if (mi.length() < 2)
            mi = "0" + mi;
        M5.Lcd.drawString(ho + ":" + mi, 2, 56);
        H = int(RTC_TimeStruct.Hours); //กำหนดค่าที่ได้จากสตริง ho ให้เป็นรูปแบบ Integer
        M = int(RTC_TimeStruct.Minutes);
    }

    se = String(RTC_TimeStruct.Seconds);
    if (se.length() < 2)
        se = "0" + se;
    M5.Lcd.drawString(se, 180, 0, 4);// X y font

    M5.Lcd.drawString(days[RTC_DateStruct.WeekDay - 1] + "    ", 4, 0, 2);
    M5.Lcd.setTextColor(grey, TFT_BLACK);

    M5.Lcd.drawString(String(RTC_DateStruct.Date) + "/" + String(RTC_DateStruct.Month), 4, 20, 4);
    M5.Lcd.drawString(String(RTC_DateStruct.Year), 70, 28, 2);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

    if (M5.BtnB.wasReleased()) { //กำหนให้เมื่อกดปุ่ม B แล้วปล่อย ทำการสลับสีบนจอ LCD เป็นสีตรงข้าม
        M5.Lcd.invertDisplay(inv);
        inv = !inv;
    }

    M5.Lcd.pushImage(112, 12, 40, 40, ani[frame]); //แสดงภาพที่ได้จากไฟล์ ani.h
    frame++;
    if (frame == 132)
        frame = 0;

    delay(12);//สั่งให้สลีปเป็นเวลา 12 มิลลิวินาที ก่อนทำงานอีกรอบ

}
