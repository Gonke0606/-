#include "sensor.h"
#include <stdio.h>

// グローバルインスタンス
CurrentSensor currentSensor;

// AMC1204の設定
// 入力レンジ: ±250mV (通常の設定)
// DFSDM分解能: 24bit
// 変換係数: 250mV / 2^23 (MSBを符号ビットとして使用)
// 電流変換係数: センサのゲインによって決まる (例: ゲイン=100の場合)
#define AMC1204_INPUT_RANGE_mV  250.0f
#define DFSDM_FULL_SCALE        8388608.0f  // 2^23
#define SENSOR_GAIN             100.0f      // 電流センサのゲイン（使用センサに応じて調整）
#define CURRENT_RANGE_mA        2500.0f     // 測定可能な最大電流（例: ±2500mA）

// 外部ハンドル宣言（CubeMXで生成されるファイルで定義される）
extern DFSDM_Filter_HandleTypeDef hdfsdm1_filter0;
extern DMA_HandleTypeDef hdma_dfsdm1_flt0;

// Error_Handlerの外部宣言
extern void Error_Handler(void);

CurrentSensor::CurrentSensor() : current_mA(0), data_ready(false)
{
}

void CurrentSensor::init()
{
    // DFSDM Filter 0 の初期化（CubeMXで生成されることを想定）
    // ここでは基本設定のみ記述
    
    // DMAを開始して連続的にデータを取得
    if (HAL_DFSDM_FilterRegularStart_DMA(&hdfsdm1_filter0, 
                                         (int32_t*)raw_data_buffer, 
                                         32) != HAL_OK)
    {
        Error_Handler();
    }
}

void CurrentSensor::update()
{
    if (data_ready)
    {
        convertToCurrent_mA();
        
        // 電流値をprintfで出力
        printf("Current: %ld mA\r\n", current_mA);
        
        data_ready = false;
    }
}

void CurrentSensor::convertToCurrent_mA()
{
    // バッファの平均値を計算
    int64_t sum = 0;
    for (int i = 0; i < 32; i++)
    {
        sum += raw_data_buffer[i];
    }
    int32_t average = (int32_t)(sum / 32);
    
    // DFSDMの生データを電圧に変換 (mV)
    // DFSDMは24bitデータを32bit符号付き整数として返す
    float voltage_mV = (float)average * (AMC1204_INPUT_RANGE_mV / DFSDM_FULL_SCALE);
    
    // 電圧を電流に変換 (mA)
    // 電流センサのゲインを考慮
    // voltage_mV = I_mA * (1 / GAIN) * 負荷抵抗
    // ここでは簡略化して、直接変換係数を使用
    float current_float = voltage_mV * (CURRENT_RANGE_mA / AMC1204_INPUT_RANGE_mV);
    current_mA = (int32_t)current_float;
}

void CurrentSensor::HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdma_filter)
{
    if (hdfsdma_filter->Instance == DFSDM1_Filter0)
    {
        currentSensor.data_ready = true;
    }
}

void CurrentSensor::HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdma_filter)
{
    // ハーフバッファも処理する場合はここで実装
}

// HALコールバック関数（HALライブラリから呼ばれる）
void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdma_filter)
{
    CurrentSensor::HAL_DFSDM_FilterRegConvCpltCallback(hdfsdma_filter);
}

void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdma_filter)
{
    CurrentSensor::HAL_DFSDM_FilterRegConvHalfCpltCallback(hdfsdma_filter);
}

