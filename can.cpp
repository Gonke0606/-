#include "can.h"
#include "sensor.h"

// グローバルインスタンス
CANController canController;

// 外部ハンドル宣言（CubeMXで生成されるファイルで定義される）
extern CAN_HandleTypeDef hcan1;

// Error_Handlerの外部宣言
extern void Error_Handler(void);

// CAN ID設定
#define CAN_SLAVE_RX_ID    0x100  // 受信するCAN ID
#define CAN_SLAVE_TX_ID    0x200  // 送信するCAN ID

CANController::CANController() : current_value_mA(0), data_received(false)
{
    // 送信ヘッダの初期設定
    TxHeader.StdId = CAN_SLAVE_TX_ID;
    TxHeader.ExtId = 0x01;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = 8;  // 8バイト送信
    TxHeader.TransmitGlobalTime = DISABLE;
}

void CANController::init()
{
    // CAN初期化（CubeMXで生成されることを想定）
    // ここではフィルタと受信開始のみ記述
    
    // CANフィルタ設定（受信するIDを設定）
    CAN_FilterTypeDef sFilterConfig;
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = CAN_SLAVE_RX_ID << 5;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0xFFE0;  // IDの下位ビットを無視
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;
    
    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    
    // CAN開始
    if (HAL_CAN_Start(&hcan1) != HAL_OK)
    {
        Error_Handler();
    }
    
    // 受信割り込みを有効化
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void CANController::update()
{
    // 電流値を更新
    current_value_mA = currentSensor.getCurrent_mA();
    
    // 受信データを処理
    if (data_received)
    {
        processReceivedMessage();
        data_received = false;
    }
    
    // 8バイトのデータとして電流値を送信（mA、32bit符号付き整数として）
    TxData[0] = (uint8_t)(current_value_mA & 0xFF);
    TxData[1] = (uint8_t)((current_value_mA >> 8) & 0xFF);
    TxData[2] = (uint8_t)((current_value_mA >> 16) & 0xFF);
    TxData[3] = (uint8_t)((current_value_mA >> 24) & 0xFF);
    // 残りの4バイトは0で埋める
    TxData[4] = 0;
    TxData[5] = 0;
    TxData[6] = 0;
    TxData[7] = 0;
    
    // CANメッセージ送信
    if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
    {
        Error_Handler();
    }
}

void CANController::setCurrentValue(int32_t current_mA)
{
    current_value_mA = current_mA;
}

void CANController::processReceivedMessage()
{
    // 0bitのデータを受け取った場合の処理
    // ここでは何もする必要がないが、必要に応じて処理を追加可能
}

void CANController::HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    if (hcan->Instance == CAN1)
    {
        // 受信メッセージを読み取り
        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &canController.RxHeader, canController.RxData) == HAL_OK)
        {
            canController.data_received = true;
        }
    }
}

// HALコールバック関数
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CANController::HAL_CAN_RxFifo0MsgPendingCallback(hcan);
}

