// xiao_laba_cn@yahoo.com, 2018
// STM8S003F3, 10-bit ADC, low cost ADC demo/ framework.
// ie. voltage/currrent sensor / metering
// software oversampling could be extension of 14bit or 16bit, 
// trade-off, MCU speed and application no constraint.
// 7-seg LED with dot enabled, 3 digits, PN MY3631AH-1 or equlivent
#define demo


#include "stm8s.h"
#include <stdio.h>

#ifndef demo
/*
		IIRFilter(adcValue); //omit if open source
		QuickRollingFilter(adcValue); //omit if open source
		oversampling(adcValue); //omit if open source
*/
#include "iirf.h"
#include "qrf.h"
#include "precision.h"
#endif

// Tune your hardware if common common A/K 7-seg LED used
// 定義7段顯示器連接 (根據您的硬體調整)
#define SEG_A_PORT   GPIOD    // 段A GPIO端口
#define SEG_A_PIN    GPIO_PIN_0  // 段A引腳
#define SEG_B_PORT   GPIOD    // 段B GPIO端口
#define SEG_B_PIN    GPIO_PIN_1  // 段B引腳
#define SEG_C_PORT   GPIOD    // 段C GPIO端口
#define SEG_C_PIN    GPIO_PIN_2  // 段C引腳
#define SEG_D_PORT   GPIOD    // 段D GPIO端口
#define SEG_D_PIN    GPIO_PIN_3  // 段D引腳
#define SEG_E_PORT   GPIOD    // 段E GPIO端口
#define SEG_E_PIN    GPIO_PIN_4  // 段E引腳
#define SEG_F_PORT   GPIOD    // 段F GPIO端口
#define SEG_F_PIN    GPIO_PIN_5  // 段F引腳
#define SEG_G_PORT   GPIOD    // 段G GPIO端口
#define SEG_G_PIN    GPIO_PIN_6  // 段G引腳
#define DP_PORT      GPIOD    // 小數點 GPIO端口
#define DP_PIN       GPIO_PIN_7  // 小數點引腳

// Tune your hardware if common A/K 7-seg LED used

// Common cathod 7-seg LED
// 數位選擇引腳 (共陰極)
#define DIGIT1_PORT  GPIOB    // 第一位數 GPIO端口
#define DIGIT1_PIN   GPIO_PIN_4  // 第一位數引腳
#define DIGIT2_PORT  GPIOB    // 第二位數 GPIO端口
#define DIGIT2_PIN   GPIO_PIN_5  // 第二位數引腳
#define DIGIT3_PORT  GPIOB    // 第三位數 GPIO端口
#define DIGIT3_PIN   GPIO_PIN_6  // 第三位數引腳

// Volt divider ratio
// 電壓分壓器常數 (根據您的硬體調整)
#define VOLTAGE_DIVIDER_RATIO 20.0f  // R1=95k, R2=5k 比例為20:1
#define ADC_REF_VOLTAGE 5.0f         // ADC參考電壓
#define ADC_MAX_VALUE 1023.0f        // 10位元ADC最大值

// map of 7-seg digit/alphabet
// 數字0-9的7段顯示模式 (共陰極)
const uint8_t digitPatterns[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
	// n, A~F, for HEX, n00 = 0x00, nFE = 0xFE, omit for Voltage meter
};

volatile uint16_t adcValue = 0;      // 儲存ADC讀取值
volatile float measuredVoltage = 0.0f; // 儲存測量到的電壓值
uint8_t displayDigits[3] = {0, 0, 0}; // 儲存要顯示的3位數字
uint8_t currentDigit = 0;            // 當前顯示的位數

void GPIO_Setup(void) {
    // 配置7段顯示器段為輸出
    GPIO_Init(SEG_A_PORT, SEG_A_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(SEG_B_PORT, SEG_B_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(SEG_C_PORT, SEG_C_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(SEG_D_PORT, SEG_D_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(SEG_E_PORT, SEG_E_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(SEG_F_PORT, SEG_F_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(SEG_G_PORT, SEG_G_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(DP_PORT, DP_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    
    // 配置數位選擇引腳為輸出
    GPIO_Init(DIGIT1_PORT, DIGIT1_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(DIGIT2_PORT, DIGIT2_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    GPIO_Init(DIGIT3_PORT, DIGIT3_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
    
    // 初始關閉所有數位
    GPIO_WriteHigh(DIGIT1_PORT, DIGIT1_PIN);
    GPIO_WriteHigh(DIGIT2_PORT, DIGIT2_PIN);
    GPIO_WriteHigh(DIGIT3_PORT, DIGIT3_PIN);
}


void ADC_Setup(void) {
    // 在通道0(A0引腳)初始化ADC
    ADC1_Init(ADC1_CONVERSIONMODE_SINGLE, // 單次轉換模式
              ADC1_CHANNEL_0,             // 通道0
              ADC1_PRESSEL_FCPU_D2,       // 預分頻
              ADC1_EXTTRIG_GPIO,          // 外部觸發
              DISABLE,                    // 禁用外部觸發
              ADC1_ALIGN_RIGHT,           // 右對齊
              ADC1_SCHMITTTRIG_CHANNEL0,  // 施密特觸發
              DISABLE);                   // 禁用施密特觸發
    
    ADC1_Cmd(ENABLE); // 啟用ADC
}


uint16_t ADC_Read(void) {
    ADC1_StartConversion(); // 開始ADC轉換
    while(ADC1_GetFlagStatus(ADC1_FLAG_EOC) == RESET); // 等待轉換完成
    return ADC1_GetConversionValue(); // 返回轉換值
}

void TIM4_Setup(void) {
    // 設置TIM4用於顯示多工(1ms中斷)
    TIM4_TimeBaseInit(TIM4_PRESCALER_128, 125); // 16MHz/128 = 125kHz, 125 ticks = 1ms
    TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE); // 啟用更新中斷
    TIM4_Cmd(ENABLE); // 啟用定時器
}

void updateDisplayDigits(float voltage) {
    // 格式化電壓為XX.X (04.5表示4.5V)
    uint16_t displayValue = (uint16_t)(voltage * 10.0f);
    
    displayDigits[0] = displayValue / 100;          // 十位數
    displayDigits[1] = (displayValue / 10) % 10;    // 個位數
    displayDigits[2] = displayValue % 10;           // 十分位數
}

void displayDigit(uint8_t digit, uint8_t value) {
    // 首先關閉所有段
    GPIO_WriteLow(SEG_A_PORT, SEG_A_PIN);
    GPIO_WriteLow(SEG_B_PORT, SEG_B_PIN);
    GPIO_WriteLow(SEG_C_PORT, SEG_C_PIN);
    GPIO_WriteLow(SEG_D_PORT, SEG_D_PIN);
    GPIO_WriteLow(SEG_E_PORT, SEG_E_PIN);
    GPIO_WriteLow(SEG_F_PORT, SEG_F_PIN);
    GPIO_WriteLow(SEG_G_PORT, SEG_G_PIN);
    GPIO_WriteLow(DP_PORT, DP_PIN);
    
    // 關閉所有數位
    GPIO_WriteHigh(DIGIT1_PORT, DIGIT1_PIN);
    GPIO_WriteHigh(DIGIT2_PORT, DIGIT2_PIN);
    GPIO_WriteHigh(DIGIT3_PORT, DIGIT3_PIN);
    
    // 根據數字模式設置段
    uint8_t pattern = digitPatterns[value];
    if(pattern & 0x01) GPIO_WriteHigh(SEG_A_PORT, SEG_A_PIN);
    if(pattern & 0x02) GPIO_WriteHigh(SEG_B_PORT, SEG_B_PIN);
    if(pattern & 0x04) GPIO_WriteHigh(SEG_C_PORT, SEG_C_PIN);
    if(pattern & 0x08) GPIO_WriteHigh(SEG_D_PORT, SEG_D_PIN);
    if(pattern & 0x10) GPIO_WriteHigh(SEG_E_PORT, SEG_E_PIN);
    if(pattern & 0x20) GPIO_WriteHigh(SEG_F_PORT, SEG_F_PIN);
    if(pattern & 0x40) GPIO_WriteHigh(SEG_G_PORT, SEG_G_PIN);
    
    // 處理第二位數的小數點
    if(digit == 1) {
        GPIO_WriteHigh(DP_PORT, DP_PIN);
    }
    
    // 啟動當前數位
    switch(digit) {
        case 0: GPIO_WriteLow(DIGIT1_PORT, DIGIT1_PIN); break; // 第一位數
        case 1: GPIO_WriteLow(DIGIT2_PORT, DIGIT2_PIN); break; // 第二位數
        case 2: GPIO_WriteLow(DIGIT3_PORT, DIGIT3_PIN); break; // 第三位數
    }
}

INTERRUPT_HANDLER(TIM4_UPD_OVF_IRQHandler, 23) {
    TIM4_ClearITPendingBit(TIM4_IT_UPDATE); // 清除中斷掛起位
    
    // 多工顯示
    displayDigit(currentDigit, displayDigits[currentDigit]);
    currentDigit = (currentDigit + 1) % 3; // 循環切換位數
}


void main(void) {
    // 初始化外設
    GPIO_Setup();  // GPIO初始化
    ADC_Setup();   // ADC初始化
    TIM4_Setup();  // 定時器初始化
    
    // 啟用中斷
    enableInterrupts();
    
    while(1) {
        // 讀取ADC值
        adcValue = ADC_Read();
#ifndef demo	
		IIRFilter(adcValue); //omit if open source
		QuickRollingFilter(adcValue); //omit if open source
		oversampling(adcValue); //omit if open source
#endif  
        // 轉換為電壓(0-100V範圍)
        float adcVoltage = (adcValue / ADC_MAX_VALUE) * ADC_REF_VOLTAGE;
        measuredVoltage = adcVoltage * VOLTAGE_DIVIDER_RATIO;
        
        // 更新顯示數字
        updateDisplayDigits(measuredVoltage);

#ifdef demo        
        // 測量間簡單延遲
        for(uint16_t i = 0; i < 1000; i++);
#endif
		
    }
}