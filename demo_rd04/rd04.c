#include "FreeRTOS.h"
#include "hosal_i2c.h"
#include "task.h"
#include <rd04.h>
#include <stdint.h>
#include <stdio.h>

#define RD_ADDR 0x71

extern hosal_i2c_dev_t i2c0;

void IIC_WriteData(uint8_t reg_addr, uint8_t Buff) {
  uint8_t Data[2];
  uint8_t ret = -1;
  Data[0] = reg_addr;
  Data[1] = Buff;
  ret = hosal_i2c_master_send(&i2c0, RD_ADDR, Data, sizeof(Data),
                              HOSAL_WAIT_FOREVER); // 发送数据
  if (ret != 0) {
    printf("err:%c\n", Buff);
  }
}

uint8_t IIC_ReadData(uint8_t reg_addr) {
  uint8_t read_data[1];
  read_data[0] = reg_addr;
  hosal_i2c_master_recv(&i2c0, RD_ADDR, read_data, 1, HOSAL_WAIT_FOREVER);
  return read_data;
}

void RD_04_Init(void) {
  printf("init radar\r\n");
  uint8_t value;

  for (uint8_t i = 0; i < 5; i++) {
    IIC_WriteData(0x13, 0x9B);
    vTaskDelay(1);
    value = IIC_ReadData(0x13);
    printf("READ VALUE =0x%02X\r\n", value);
  }

  IIC_WriteData(0x24, 0x03);

  IIC_WriteData(0x04, 0x20);

  IIC_WriteData(0x10, 0x20);

  IIC_WriteData(0x03, 0x40);

  IIC_WriteData(0x1C, 0x21);

  // IIC_WriteData(0x11, 0x10);

  IIC_WriteData(0x18, 0x6a);
  IIC_WriteData(0x19, 0x00);
  IIC_WriteData(0x1A, 0x55);
  IIC_WriteData(0x1B, 0x01);

  IIC_WriteData(0x1D, 0x80);
  IIC_WriteData(0x1E, 0x0C);
  IIC_WriteData(0x1F, 0x00);

  IIC_WriteData(0x20, 0x00);
  IIC_WriteData(0x21, 0x7D);
  IIC_WriteData(0x22, 0x00);

  IIC_WriteData(0x23, 0x0C);
  vTaskDelay(3000);
  printf("init radar successfuly\r\n");
}
