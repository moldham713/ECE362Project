/**
  ******************************************************************************
  * @file    main.c
  * @author  oldham2
  * @version V1.0
  * @date    23 October 2019
  * @brief   Main function for STM32 Slot machine
  ******************************************************************************
*/
/*PINS
 * A0: OEN
 * A1: CLK
 * A2: LATCH
 * A3: R1
 * A4: G1
 * A5: B1
 * A6: R2
 * A7: G2
 * A8: B2
 * A9: A
 * A10: B
 * A11: C
 *
 * LED MATRIX PINOUT
 * R1	G1
 * B1	GND
 * R2	G2
 * B2	GND
 * A 	B
 * C 	GND
 * CLK  LAT
 * OE   GND
 */

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

unsigned char  imagedata[32*3*16] = {0};



void init_picture()
{
	GPIOA->ODR |= 1; //OEN
	NVIC->ICER[0] |= 1 << (TIM6_DAC_IRQn);
	for(int x = 0; x < 32*3; x++)
	{
		for(int y = 0; y < 16; y++)
		{
			imagedata[(32*3*y)+x] = 1;
		}
	}
	NVIC->ISER[0] |= 1 << (TIM6_DAC_IRQn);
	GPIOA->ODR &= ~(1);
}

void picture_blank()
{
	for(int x = 0; x < 32*3; x++)
	{
		for(int y = 0; y < 16; y++)
		{
			imagedata[(32*3*y)+x] = 0;
		}
	}
}

void picture_red()
{
	GPIOA->ODR |= 1; //OEN
	NVIC->ICER[0] |= 1 << (TIM6_DAC_IRQn);
	for(int x = 0; x < 32*3; x++)
	{
		for(int y = 0; y < 16; y++)
		{
			if(x % 3 == 0) imagedata[(32*3*y)+x] = 1;
			else imagedata[(32*3*y)+x] = 0;
		}
	}
	NVIC->ISER[0] |= 1 << (TIM6_DAC_IRQn);
	GPIOA->ODR &= ~(1);
}

void init_GPIO()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= 0x555555;
}

void init_tim6()
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->PSC = 20-1;
    TIM6->ARR = 20-1;
    TIM6->CR1 |= 1;
    TIM6->DIER |= 1;
    NVIC->ISER[0] |= 1 << (TIM6_DAC_IRQn);
}


void TIM6_DAC_IRQHandler()
{
	static int imageline = 0;
	GPIOA->ODR &= ~(0xe00); //Clear bits 9-11 for ABC
    GPIOA->ODR |= 1; //OEN
	GPIOA->ODR |= (imageline & 1) << 9;
	GPIOA->ODR |= ((imageline & 2) >> 1) << 10;
	GPIOA->ODR |= ((imageline & 4) >> 2) << 11;
    for(int x = 0; x < 32; x++)
    {
    	GPIOA->ODR &= ~(0x1f8); //clear bits 3-8 for RGB
    	GPIOA->ODR |= (1 & (imagedata[(imageline *3 * 32)+x])) << 3;
    	GPIOA->ODR |= (1 & (imagedata[(imageline *3 * 32)+x+1])) << 4;
    	GPIOA->ODR |= (1 & (imagedata[(imageline *3 * 32)+x + 2])) << 5;
    	GPIOA->ODR |= (1 & (imagedata[((imageline+8) *3 * 32)+x])) << 6;
    	GPIOA->ODR |= (1 & (imagedata[((imageline+8) *3 * 32)+x+1])) << 7;
    	GPIOA->ODR |= (1 & (imagedata[((imageline+8) *3 * 32)+x + 2])) << 8;

    	GPIOA->ODR |= 0x2; //Clock high
    	GPIOA->ODR &= ~(0x2); //Clock low
    }
    GPIOA->ODR |= 0x4; //Latch high
    GPIOA->ODR &= ~(0x4); //Latch low
    GPIOA->ODR &= ~(1);
    imageline +=1;
    if(imageline > 7) imageline = 0;
    TIM6->SR &= !(1);
}

int main(void)
{
	init_picture();
	init_GPIO();
	init_tim6();

	while(1)
	{
		init_picture();
		for(int i = 0; i < 24000; i++)
		{
			asm("nop");
		}
		picture_red();
		for(int i = 0; i < 24000; i++)
		{
			asm("nop");
		}
	}

}