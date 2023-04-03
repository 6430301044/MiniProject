//ตัวแปรที่จำเป็นสำหรับ Counting
//ตัวแปรแรกคือ include or import everything is important for this counting project
#include "M5StickCPlus.h" //import libary ของ M5StickCPlus
#include <driver/i2s.h> //import libary ของ I2S (Inter-IC Sound) Microphone

//ประกาศให้ตัวแปรเป็นค่าคงที่
#define CLK_PIN 0 // ประกาศว่า CLK_PIN ใช้พอร์ต 0
#define MIC_PIN 34 
#define BTNB_PIN 39 
#define READ_LEN    (2 * 256)
#define GAIN_FACTOR 3
uint8_t BUFFER[READ_LEN] = {0};

uint16_t oldy[160];
int16_t *adcBuffer = NULL;

// Variable for this project
int y;
float Vref = 1.0; // แรงดันอ้างอิงเป็น 1 พาวเปอร์
float dB = 0;

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

void setup(){
//โค้ดที่จำเป็นต้องเซตอัปก่อนที่จะไปรันในโค้ดหลัก
    M5.begin(); //Super mega very important for everyting project with use M5Family
    i2sInit(); //Super mega very important for this project
}

void loop(){
//โค้ดทำงานหลักที่จะให้แสดงผลบนหน้าจอและส่งค่าไปยัง Blynk
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
