//ตัวแปรที่จำเป็นสำหรับ Counting
//ตัวแปรแรกคือ include or import everything is important for this counting project
#include "M5StickCPlus.h" 

// Variable for this project
int Mode = 1;
float gyroX = 0;
float gyroY = 0;
float gyroZ = 0;

float accX = 0;
float accY = 0;
float accZ = 0;

float threshold = 1.5; // ค่าความแรงในการแกว่งแขน
float UserWeight = 0;
int StepWalk = 0;
int SwingArm = 0;
float CalorieWalkBurn = 0;
float CalorieArmBurn = 0;
float CalorieWalk = 0;
float CalorieArm = 0;
float TotalCalorie = 0;

void ResetWalkMode(){
  StepWalk = 0;
  CalorieWalkBurn = 0;
  CalorieWalk = 0;
  Blynk.virtualWrite(V4, StepWalk);
  Blynk.virtualWrite(V6, TotalCalorie);
  sendToBlynk();      
}

void ResetArmMode(){
  SwingArm = 0;
  CalorieArmBurn = 0;
  CalorieArm = 0;
  Blynk.virtualWrite(V5, SwingArm);
  Blynk.virtualWrite(V6, TotalCalorie);
  sendToBlynk();
}

void setup(){
//โค้ดที่จำเป็นต้องเซตอัปก่อนที่จะไปรันในโค้ดหลัก
    M5.Imu.Init(); //Super mega very important
}

void loop(){
//โค้ดทำงานหลักที่จะให้แสดงผลบนหน้าจอและส่งค่าไปยัง Blynk
    M5.update(); //เพื่อให้อ่านค่าจากการกดปุ่ม
   /*-----STEPCOUNTING&STEPARM-----*/
    M5.Imu.getGyroData(&gyroX, &gyroY, &gyroZ);
    M5.Imu.getAccelData(&accX, &accY, &accZ);
    Blynk.virtualWrite(V8, UserWeight);//รับค่าน้ำหนักผู้ใช้ที่ได้จาก Blynk โดยที่ Blynk รับค่าจาก Slider บน Smartphone หรือ web
    Blynk.virtualWrite(V7, Mode);//รับค่าโหมดจาก  Blynk โดยที่ Blynk รับค่าจาก Button switch mode บน Smartphone หรือ web
    float accMagnitude = sqrt(accX * accX + accY * accY + accZ * accZ); // คำนวณค่าแรงโน้มถ่วงของความเร่ง (magnitude) สูตรช่วยชีวิต
    if (Mode == 1) {
        if (M5.BtnA.wasReleased()) {
          ResetWalkMode();      //ใช้งานฟังก์ชัน ResetWalkMode ถ้าเงื่อนไขเป็นจริง
        } else if (accMagnitude > threshold) {
            StepWalk++;
            CalorieWalkBurn = StepWalk * (0.57 * UserWeight) / 1000;
            CalorieWalk = CalorieWalk + CalorieWalkBurn;
            TotalCalorie = CalorieWalk + CalorieArm;
            Blynk.virtualWrite(V4, StepWalk); //ส่งค่าจาก device(M5StickCPlus) ไปยัง Blynk Server เพื่อรอให้ไปแสดงผลบน Smartphone หรือ web เมื่อเรียกข้อข้อมูล
            Blynk.virtualWrite(V6, TotalCalorie);
            sendToBlynk();
        }
    } else if (Mode == 0) {
        if (M5.BtnA.wasReleased()) {
          ResetArmMode();
        } else if (accMagnitude > threshold) {
            SwingArm++;
            CalorieArmBurn = 0.0175 * UserWeight * SwingArm;
            CalorieArm = CalorieArm + CalorieArmBurn;
            TotalCalorie = CalorieWalk + CalorieArm;
            Blynk.virtualWrite(V5, SwingArm);
            Blynk.virtualWrite(V6, TotalCalorie);
            sendToBlynk();
        }
    }
    sendToBlynk();
}
