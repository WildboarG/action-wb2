#ifndef __RD_04_H__
#define __RD_04_H__

void IIC_WriteData(uint8_t reg_addr, uint8_t Buff);

uint8_t IIC_ReadData(uint8_t reg_addr);

void RD_04_Init(void);

#endif // !__RD_04_H__
