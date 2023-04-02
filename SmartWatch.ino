#include "M5StickCPlus.h" //import libary ของ M5StickCPlus
#include "AXP192.h" //import libary ของ AXP192 ตัว manage power
#include "7seg70.h" //import ข้อมูลต่างๆจากไฟล์ 7seg70.h
#include "ani.h"  
#include <driver/i2s.h> //import libary ของ I2S (Inter-IC Sound) Microphone

#define grey 0x65DB // ประกาศว่า grey คือสี 0x65DB สีเทานะแหละ
#define CLK_PIN 0 // ประกาศว่า CLK_PIN ใช้พอร์ต 0
#define MIC_PIN 34 
#define BTNB_PIN 39 
#define READ_LEN    (2 * 256)
#define GAIN_FACTOR 3
uint8_t BUFFER[READ_LEN] = {0};

uint16_t oldy[160];
int16_t *adcBuffer = NULL;

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

int brightness = 8;

//Format ของการเซตเวลาและวันเดือนปี
RTC_TimeTypeDef RTC_TimeStruct;//ประกาศตัวแปร เซตเวลา ชื่อ RTC_TimeStruct
RTC_DateTypeDef RTC_DateStruct;//ประกาศตัวแปร เซตวันเดือนปี ชื่อ RTC_DateStruct
int H = 0;
int M = 0;

String ho = "";
String mi = "";
String se = "";

String days[7] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

bool spavanje = 0;
bool inv = 0;

int frame = 0;

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

int y;
float Vref = 1.0; // แรงดันอ้างอิงเป็น 1 พาวเปอร์
float dB = 0;

/*-------------BLYNKCOMMARD-------------*/
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

BLYNK_CONNECTED() {
    Blynk.syncAll();
}

unsigned long sendToBlynkPremillis = 0;

void sendToBlynk() {
    // current time - previous time >= 300;
    // previous = current tim
    if (millis() - sendToBlynkPremillis >= 300) {
        sendToBlynkPremillis = millis();
    }
}

void i2sInit() {
    i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
            .sample_rate = 44100,
            .bits_per_sample =
            I2S_BITS_PER_SAMPLE_16BIT,  // is fixed at 12bit, stereo, MSB
            .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(4, 1, 0)
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
#else
            .communication_format = I2S_COMM_FORMAT_I2S,
#endif
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count    = 2,
            .dma_buf_len      = 128,
    };

    i2s_pin_config_t pin_config;

#if (ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(4, 3, 0))
    pin_config.mck_io_num = I2S_PIN_NO_CHANGE;
#endif

    pin_config.bck_io_num = I2S_PIN_NO_CHANGE;
    pin_config.ws_io_num = CLK_PIN;
    pin_config.data_out_num = I2S_PIN_NO_CHANGE;
    pin_config.data_in_num = MIC_PIN;

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

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

void setup() {
    // put your setup code here, to run once:
// Debug console
    Serial.begin(9600);
    Blynk.begin(auth, ssid, pass);//Or Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass)

    M5.begin();
    M5.Imu.Init();
    i2sInit();
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setSwapBytes(true);
    M5.Lcd.setTextSize(1);

    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Axp.EnableCoulombcounter();

    RTC_TimeTypeDef TimeStruct;
    TimeStruct.Hours = 4;
    TimeStruct.Minutes = 27;
    TimeStruct.Seconds = 0;
    //M5.Rtc.SetTime(&TimeStruct);
    RTC_DateTypeDef DateStruct;
    DateStruct.WeekDay = 7;
    DateStruct.Month = 4;
    DateStruct.Date = 2;
    DateStruct.Year = 2023;
    //M5.Rtc.SetData(&DateStruct);
}

void loop() {
    // put your main code here, to run repeatedly:
    M5.update(); //เพื่อให้อ่านค่าจากการกดปุ่ม
    /*-----------WATCH-----------*/
    spavanje = 0;
    M5.Rtc.GetTime(&RTC_TimeStruct);
    M5.Rtc.GetData(&RTC_DateStruct);
    M5.Lcd.setCursor(0, 15);
    M5.Lcd.setTextFont(0);

    M5.Lcd.drawString(String(M5.Axp.GetBatVoltage()) + " V  ", 114, 3);
    M5.Lcd.setFreeFont(&DSEG7_Classic_Regular_64);

    if (H != int(RTC_TimeStruct.Hours) || M != int(RTC_TimeStruct.Minutes)) {
        ho = String(RTC_TimeStruct.Hours);
        if (ho.length() < 2)
            ho = "0" + ho;

        mi = String(RTC_TimeStruct.Minutes);
        if (mi.length() < 2)
            mi = "0" + mi;
        M5.Lcd.drawString(ho + ":" + mi, 2, 56);
        H = int(RTC_TimeStruct.Hours);
        M = int(RTC_TimeStruct.Minutes);
    }

    // M5.Lcd.printf("Week: %d\n",RTC_DateStruct.WeekDay);
    se = String(RTC_TimeStruct.Seconds);
    if (se.length() < 2)
        se = "0" + se;
    M5.Lcd.drawString(se, 180, 0, 4);

    M5.Lcd.drawString(days[RTC_DateStruct.WeekDay - 1] + "    ", 4, 0, 2);
    M5.Lcd.setTextColor(grey, TFT_BLACK);

    M5.Lcd.drawString(String(RTC_DateStruct.Date) + "/" + String(RTC_DateStruct.Month), 4, 20, 4);
    M5.Lcd.drawString(String(RTC_DateStruct.Year), 70, 28, 2);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

    if (M5.BtnB.wasReleased()) {
        M5.Lcd.invertDisplay(inv);
        inv = !inv;
    }

    M5.Lcd.pushImage(112, 12, 40, 40, ani[frame]);
    frame++;
    if (frame == 132)
        frame = 0;

    delay(12);
    /*--------------BLYNK--------------*/
    Blynk.run();
    float BattPower = M5.Axp.GetBatPower();
    Blynk.virtualWrite(V1, BattPower);
    float voltage = M5.Axp.GetBatVoltage();
    float percentage = (voltage - 3.3) / 0.9 * 100;
    if (percentage > 100) {
        percentage = 100;
    }
    if (percentage < 0) {
        percentage = 0;
    }
    Blynk.virtualWrite(V2, percentage);
    /*-----STEPCOUNTING&STEPARM-----*/
    M5.Imu.getGyroData(&gyroX, &gyroY, &gyroZ);
    M5.Imu.getAccelData(&accX, &accY, &accZ);
    Blynk.virtualWrite(V8, UserWeight);
    Blynk.virtualWrite(V7, Mode);
    float accMagnitude = sqrt(accX * accX + accY * accY + accZ * accZ); // คำนวณค่าแรงโน้มถ่วงของความเร่ง (magnitude)
    if (Mode == 1) {
        if (M5.BtnA.wasReleased()) {
          ResetWalkMode();      
        } else if (accMagnitude > threshold) {
            StepWalk++;
            CalorieWalkBurn = StepWalk * (0.57 * UserWeight) / 1000;
            CalorieWalk = CalorieWalk + CalorieWalkBurn;
            TotalCalorie = CalorieWalk + CalorieArm;
            Blynk.virtualWrite(V4, StepWalk);
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
    /*-----------MICROPHONE-----------*/
    size_t bytesread;

    i2s_read(I2S_NUM_0, (char *) BUFFER, READ_LEN, &bytesread,
             (100 / portTICK_RATE_MS));
    adcBuffer = (int16_t *) BUFFER;
    for (int n = 0; n < 160; n++) {
        y = adcBuffer[n] * GAIN_FACTOR;
        y = map(y, INT16_MIN, INT16_MAX, 10, 70);
    }
    dB = 20 * log10((float) y / Vref); // แปลงค่า y เป็น dB
    Blynk.virtualWrite(V3, dB); // ส่งค่า dB ไปยังแอป Blynk
    vTaskDelay(100 / portTICK_RATE_MS);

}