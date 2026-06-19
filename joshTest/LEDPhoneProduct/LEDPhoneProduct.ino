#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

// 配置参数
#define LED_COUNT    8        // WS2812B 数量（8颗）
#define DATA_PIN     15       // ATtiny1616 PA2 引脚 
#define POWER_EN     14       // ATtiny1616 PA1 引脚 
#define KEY_PIN      0        // ATtiny1616 PA4 引脚 
#define POWER_C      1        // ATtiny1616 PA5 引脚 
#define BRIGHTNESS   125      // 亮度（0-255，避免过亮导致供电不足）
#define MAXMODE      14        //工作模式最大值


volatile bool buttonPressed = false;         //按键按下
volatile unsigned long buttonPressTime = 0;  //按键按下时间
volatile bool longPressDetected  = false;    //检测到按键长按
volatile bool shortPressDetected = false;    //检测到按键短按
const unsigned long LONG_PRESS_TIME = 1000;  //长按时间2秒
const unsigned long SHORT_PRESS_TIME = 40;
unsigned char workMode  = 0;                 //工作模式
volatile bool isSleeping = true;             //系统是否睡眠状态
volatile bool systemWakeup = false;          //系统是否唤醒状态 

// 初始化 NeoPixel 对象
// NEO_GRB：WS2812B 颜色格式（绿-红-蓝）；NEO_KHZ800：通信频率 800KHz（标准）
Adafruit_NeoPixel strip(LED_COUNT, DATA_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // put your setup code here, to run once:
  pinMode(KEY_PIN, INPUT_PULLUP);  
  pinMode(POWER_EN, OUTPUT);   
  pinMode(POWER_C, OUTPUT);
  digitalWrite(POWER_EN, LOW);
  // 配置PA4引脚中断
  //setupPA4Interrupt();
  // 启用全局中断
  //sei();
  unsigned long previousMillis = millis();
  workMode = EEPROM.read(0); 
  workMode = (workMode>MAXMODE)?0:workMode;
  while(digitalRead(KEY_PIN) == LOW);
  if (millis() - previousMillis >= LONG_PRESS_TIME) 
  {
    //LED Power on
    digitalWrite(POWER_C, HIGH);
    isSleeping = false;
    systemWakeup = true;
    PORTA.INTFLAGS = (1 << 4);  
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;

    strip.begin();                   //初始化 WS2812B 控制
    strip.setBrightness(BRIGHTNESS); //设置亮度
    strip.show();                    //初始全灭（必须调用才会更新显示）
    //启用全局中断
    sei();
  }
  else
  {
    isSleeping = true;    
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (isSleeping)
  {
    enterSleepAndWakeup();
  }
  else 
  {
    runningMode();
  }
}
/**
 * @brief  进入休眠模式及唤醒判断
 * @param  无
 * @return 无
 */
void enterSleepAndWakeup()
{
  if (EEPROM.read(0) != workMode)
  {
    EEPROM.write(0, workMode);
    delay(10);     
  }
  digitalWrite(POWER_EN, HIGH);
  digitalWrite(POWER_C, LOW);
  power_all_disable();// 禁用不需要的外设
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  // 配置PA4引脚中断
  setupPA4Interrupt();
  // 启用全局中断
  sei();
  sleep_cpu();  //进入睡眠

  // ---- 代码在此处睡眠，直到按键唤醒 ----//
  // 系统唤醒处理
  sleep_disable();
  power_all_enable();

  digitalWrite(POWER_EN, LOW);
  digitalWrite(POWER_C, HIGH);
  PORTA.INTFLAGS = (1 << 4);
  PORTA.PIN4CTRL = PORT_PULLUPEN_bm; // 移除中断触发配置，只保留上拉
  if (!systemWakeup)
  {
    if (digitalRead(KEY_PIN) == LOW)
    {
      unsigned long previousMillis = millis();
      while(digitalRead(KEY_PIN) == LOW);
      if (millis() - previousMillis >= LONG_PRESS_TIME) 
      {
        isSleeping = false;
        systemWakeup = true;
        PORTA.INTFLAGS = (1 << 4);     
        PORTA.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // 配置PA4为双边沿触发（按下和释放都能触发）

        strip.begin();                   //初始化 WS2812B 控制
        strip.setBrightness(BRIGHTNESS); //设置亮度
        strip.show();                    //初始全灭（必须调用才会更新显示）
        sei();

      }
      else
      {
        isSleeping = true;
      }
    }
    else 
    {
      isSleeping = true;
    }
  }
}
/**
 * @brief  运行模式处理：按键处理及工作模式切换 
 * @param  无
 * @return 无
 */
void runningMode() {
  // 检查长按
  if (longPressDetected) {
    strip.clear();  // 清空所有LED
    longPressDetected = false;            
    isSleeping = true;
    systemWakeup = false;
    return;
  }
  // 检查短按
  if (shortPressDetected) 
  {
    shortPressDetected = false;       
    // 切换工作模式
    workMode = workMode + 1;
    if (workMode > MAXMODE)
    {
      workMode = 0;
    }
  } 
  switch (workMode)
  {
    case 0:
    allColor(255, 0, 0); // RED
    break;

    case 1:
    flash(255, 0, 0);
    break;

    case 2:
    allColor(255, 255, 0); // YELLOW
    break;

    case 3:
    flash(255, 255, 0);
    break;

    case 4:
    allColor(255, 105, 180); // PINK
    break;

    case 5:
    flash(255, 105, 180);
    break;

    case 6:
    allColor(0, 255, 0); // GREEN
    break;

    case 7:
    flash(0, 255, 0);
    break;

    case 8:
    allColor(255, 180, 0); // ORANGE
    break;

    case 9:
    flash(255, 180, 0);
    break;

    case 10:
    allColor(125, 0, 255); // PURPLE
    break;

    case 11:
    flash(125, 0, 255);
    break;

    case 12:
    allColor(0, 0, 255); // BLUE
    break;

    case 13:
    flash(0, 0, 255);
    break;

    default:
        // 都不匹配
        // 则执行这里的程序
        break;
  }
}
/**
 * @brief  配置PA4引脚中断(按键) 
 * @param  无
 * @return 无
 */
void setupPA4Interrupt() {
  // 清除PORTA的中断标志位
  PORTA.INTFLAGS = PORT_INT0_bm | PORT_INT1_bm;
  PORTA.INTFLAGS = (1 << 4);
  // 配置PA4引脚控制寄存器
  PORTA.PIN4CTRL = PORT_PULLUPEN_bm |     // 启用上拉电阻
                   PORT_ISC_LEVEL_gc;     // 低电平触发中断
}

/**
 * @brief  PORTA中断服务程序 
 * @param  无
 * @return 无
 */
ISR(PORTA_PORT_vect) {
  // 检查是否是PA4触发的中断
  if (PORTA.INTFLAGS & (1 << 4)) {
    // 清除PA4的中断标志
    PORTA.INTFLAGS = (1 << 4);   
    if (systemWakeup)
    {
      //bool currentState = digitalRead(KEY_PIN);
      if (digitalRead(KEY_PIN) == LOW) {
        // 按钮按下
        buttonPressed = true;
        buttonPressTime = millis();
      }
      else 
      {
        if (buttonPressed)
        {
          unsigned long pressDuration = millis() - buttonPressTime;
          buttonPressed = false;
          if (pressDuration >= LONG_PRESS_TIME) 
          {           
            longPressDetected = true;// 长按
          } 
          else if (pressDuration > SHORT_PRESS_TIME) // 消抖阈值
          {                   
            shortPressDetected = true; // 短按           
          } 
          else 
          {
            // 抖动短按，忽略
          }
        }
      }
    }
  }
}


/**
 * @brief 所有 LED 显示同一颜色
 * @param r: 红色值（0-255）
 * @param g: 绿色值（0-255）
 * @param b: 蓝色值（0-255）
 */
void allColor(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));  // 设置第 i 颗 LED 颜色
  }
  strip.show();  // 刷新显示
}

/**
 * @brief 流水灯效果（逐个点亮，前一个熄灭）
 * @param r: 颜色红分量
 * @param g: 颜色绿分量
 * @param b: 颜色蓝分量
 */
void runningLight(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.clear();  // 清空所有 LED
    strip.setPixelColor(i, strip.Color(r, g, b));  // 点亮当前 LED
    strip.show();
    delay(150);     // 每颗停留 150ms
    if (longPressDetected) {
      strip.clear();  // 清空所有LED
      break;
    }
  }
}

/**
 * @brief 彩虹渐变效果（循环所有颜色）
 * @param wait: 颜色切换间隔（毫秒，越小越流畅）
 * @param count: 多少种颜色循环
 */
void rainbow(uint8_t wait,uint8_t count) {
  uint16_t i, j;
  for (j = 0; j < count; j++) {  // 256 种颜色循环
    for (i = 0; i < LED_COUNT; i++) {
      // Wheel 函数将 Hue 值转为 RGB 颜色
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
    if (longPressDetected) {
      strip.clear();  // 清空所有LED
      break;
    }
  }
}

/**
 * @brief 辅助函数：Hue 值转 RGB 颜色（用于彩虹效果）
 * @param WheelPos: Hue 值（0-255）
 * @return 32 位 RGB 颜色值（兼容 NeoPixel 库）
 */
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void flash(uint8_t r, uint8_t g, uint8_t b) {
  // Turn all LEDs on
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b)); // Set to colour
    if (longPressDetected) {
      strip.clear();  // 清空所有LED
      break;
    }
  }
  strip.show();              // Update the ring
  delay(150);               // Keep lights on for 500 ms

  // Turn all LEDs off
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0)); // Set to off
    if (longPressDetected) {
      strip.clear();  // 清空所有LED
      break;
    }
  }
  strip.show();              // Update the ring
  delay(150);               // Keep lights off for 500 ms
}
