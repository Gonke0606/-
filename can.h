#ifndef CAN_H
#define CAN_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

class CANController {
public:
    CANController();
    void init();
    void update();
    void setCurrentValue(int32_t current_mA);
    
private:
    CAN_HandleTypeDef hcan1;
    CAN_TxHeaderTypeDef TxHeader;
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t TxData[8];
    uint8_t RxData[8];
    uint32_t TxMailbox;
    int32_t current_value_mA;
    bool data_received;
    
    void processReceivedMessage();
    static void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
};

extern CANController canController;

#endif // CAN_H

