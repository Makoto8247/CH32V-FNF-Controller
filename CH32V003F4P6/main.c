/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2022/08/08
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *Multiprocessor communication mode routine:
 *Master:USART1_Tx(PD5)\USART1_Rx(PD6).
 *This routine demonstrates that USART1 receives the data sent by CH341 and inverts
 *it and sends it (baud rate 115200).
 *
 *Hardware connection:PD5 -- Rx
 *                     PD6 -- Tx
 *
 */

#include "debug.h"


/* Global define */
#define LEFT_ARROW_PIN  GPIO_Pin_7
#define DOWN_ARROW_PIN  GPIO_Pin_6
#define UP_ARROR_PIN    GPIO_Pin_5
#define RIGHT_ARROR_PIN GPIO_Pin_4
#define ENTER_KEY_PIN   GPIO_Pin_3
#define ESC_KEY_PIN     GPIO_Pin_2
#define SPACE_KEY_PIN   GPIO_Pin_1

/* Global Variable */

uint16_t send_data[14] = {
        0x57, 0xAB, // HEAD
        0x00,       // ADDR
        0x02,       // CMD
        0x08,       // LEN
        0x00,       // SPECIAL KEY
        0x00,       // SPACE
        0x00,       // LEFT KEY
        0x00,       // DOWN KEY
        0x00,       // UP KEY
        0x00,       // RIGHT KEY
        0x00,       // ENTER KEY
        0x00,       // ESC KEY or SPACE KEY
        0x00        // CHECK SUM
};
uint16_t old_send_data[14] = {
        0x57, 0xAB, // HEAD
        0x00,       // ADDR
        0x02,       // CMD
        0x08,       // LEN
        0x00,       // SPECIAL KEY
        0x00,       // SPACE
        0x00,       // LEFT KEY
        0x00,       // DOWN KEY
        0x00,       // UP KEY
        0x00,       // RIGHT KEY
        0x00,       // ENTER KEY
        0x00,       // ESC KEY or SPACE KEY
        0x00        // CHECK SUM
};


void USARTx_CFG(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);

    /* USART1 TX-->D.5   RX-->D.6 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

void BUTTON_CFG(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

uint16_t CHECK_SUM(void)
{
    uint sum = 0;
    for (int i=0;i<13;i++) sum += (uint)send_data[i];
    return (uint16_t)sum;
}

void WRITE_DATA(void)
{
    send_data[13] = CHECK_SUM();
    for(int i=0;i<14;i++){
        USART_SendData(USART1, send_data[i]);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    }
    while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

}

uint8_t DATA_EQUAL(void)
{
    for(int i=7;i<13;i++){
       if(old_send_data[i] != send_data[i]) return 1;
    }
    return 0;
}

void DATA_CPY(void)
{
    for(int i=7;i<13;i++) old_send_data[i] = send_data[i];
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    USARTx_CFG();
    BUTTON_CFG();
    DATA_CPY();

    while(1)
    {
        /* input key scan */
        send_data[7]  = (1-GPIO_ReadInputDataBit(GPIOC, LEFT_ARROW_PIN))  * 0x50;
        send_data[8]  = (1-GPIO_ReadInputDataBit(GPIOC, DOWN_ARROW_PIN))  * 0x51;
        send_data[9]  = (1-GPIO_ReadInputDataBit(GPIOC, UP_ARROR_PIN))    * 0x52;
        send_data[10] = (1-GPIO_ReadInputDataBit(GPIOC, RIGHT_ARROR_PIN)) * 0x4F;
        send_data[11] = (1-GPIO_ReadInputDataBit(GPIOC, ENTER_KEY_PIN))   * 0x28;
        send_data[12] = (1-GPIO_ReadInputDataBit(GPIOC, ESC_KEY_PIN))     * 0x58;
        if (send_data[12] != 0x58) send_data[12] = (1-GPIO_ReadInputDataBit(GPIOC, SPACE_KEY_PIN))   * 0x2C;

        if (DATA_EQUAL() == 1) WRITE_DATA();

        DATA_CPY();
    }
}
