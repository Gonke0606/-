#ifndef SENSOR_H
#define SENSOR_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

class CurrentSensor {
public:
    CurrentSensor();
    void init();
    void update();
    int32_t getCurrent_mA() const { return current_mA; }
    
private:
    DFSDM_Filter_HandleTypeDef hdfsdm1_filter0;
    DMA_HandleTypeDef hdma_dfsdm1_flt0;
    int32_t current_mA;
    int16_t raw_data_buffer[32];
    bool data_ready;
    
    void convertToCurrent_mA();
    static void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdma_filter);
    static void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdma_filter);
};

extern CurrentSensor currentSensor;

#endif // SENSOR_H

