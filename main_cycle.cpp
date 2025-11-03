#include "main_cycle.h"
#include "clock.h"
#include "sensor.h"
#include "can.h"
#include "stm32f4xx_hal.h"

static bool initialized = false;

void main_cycle_init()
{
    if (initialized)
    {
        return;
    }
    
    // クロック設定
    SystemClock_Config();
    
    // 電流センサの初期化
    currentSensor.init();
    
    // CAN通信の初期化
    canController.init();
    
    initialized = true;
}

void main_cycle_update()
{
    // 各クラスの更新処理
    currentSensor.update();
    canController.update();
}

