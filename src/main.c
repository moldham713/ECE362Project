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
 * R1   G1
 * B1   GND
 * R2   G2
 * B2   GND
 * A    B
 * C    GND
 * CLK  LAT
 * OE   GND
 */
#include <string.h>
#include <stdlib.h>
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include "imagedata.h"
#define WIN_CHANCE 5
#define NUM_SPRITES 6

unsigned char  imagedata[32*3*16] = {0};
int scrolling = 0;
int won = 0;
int winsprite = 0;
int losesprite1 = 0;
int losesprite2 = 0;
int buttondown = 0;


void load_bg()
{
    memcpy(imagedata, bgimagedata, 32*16*3);
}

void printSprite(int spritenum, int x, int y)
{
    switch(spritenum)
    {
    case 0:

        for(int i = 0; i < 8; i++)
        {
            memcpy(&imagedata[(32*3*(y+i))+x], &heartsprite[8*3*i], 8*3);
        }

        break;

    case 1:
        for(int i = 0; i < 8; i++)
        {
            memcpy(&imagedata[(32*3*(y+i))+x], &coinsprite[8*3*i], 8*3);
        }
        break;

    case 2:
        for(int i = 0; i < 8; i++)
        {
            memcpy(&imagedata[(32*3*(y+i))+x], &lemonsprite[8*3*i], 8*3);
        }
        break;


    case 3:
        for(int i = 0; i < 8; i++)
        {
            memcpy(&imagedata[(32*3*(y+i))+x], &sevensprite[8*3*i], 8*3);
        }
        break;


    case 4:
        for(int i = 0; i < 8; i++)
        {
            memcpy(&imagedata[(32*3*(y+i))+x], &snowsprite[8*3*i], 8*3);
        }
        break;


    case 5:
        for(int i = 0; i < 8; i++)
        {
            memcpy(&imagedata[(32*3*(y+i))+x], &targetsprite[8*3*i], 8*3);
        }
        break;

    }
}


void init_GPIO()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER |= 0x555555;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    GPIOC->PUPDR |= GPIO_PUPDR_PUPDR0_1;
}

void init_tim6() //display timer
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->PSC =50-1;
    TIM6->ARR = 20-1;
    TIM6->CR1 |= 1;
    TIM6->DIER |= 1;
    NVIC->ISER[0] |= 1 << (TIM6_DAC_IRQn);
    NVIC->IP[4]= 0x0000ff00; //Timer 6 has lower priority than timer 3
}

void init_tim3() //picture update timer
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3->PSC = 6000-1;
    TIM3->ARR = 100-1;
    TIM3->CR1 |= 1;
    TIM3->DIER |= 1;
    NVIC->ISER[0] |= 1 << (TIM3_IRQn);
}

void init_tim2() //state timer
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 0xafff;
    TIM2->ARR = 0x0fff;
    //TIM2->CR1 |= 1;
    TIM2->DIER |= 1;
    NVIC->ISER[0] |= 1 << (TIM2_IRQn);
}

/*
 * The timer 6 interrupt handler updates the display 2 lines at a time using
 * the data in imagedata
 */
void TIM6_DAC_IRQHandler()
{

    //Line by line
    static int imageline = 0;
    GPIOA->ODR &= ~(0xe00); //Clear bits 9-11 for ABC
    GPIOA->ODR |= 1; //OEN
    GPIOA->ODR |= (imageline & 1) << 9;
    GPIOA->ODR |= ((imageline & 2) >> 1) << 10;
    GPIOA->ODR |= ((imageline & 4) >> 2) << 11;
    for(int x = 0; x < 32*3; x+=3)
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
    TIM6->SR &= !1;
}

void TIM2_IRQHandler()
{
    scrolling = !scrolling;
    won = rand() % WIN_CHANCE == 1;
    winsprite = rand() % NUM_SPRITES;
    losesprite1 = (winsprite + 1 + rand() % (NUM_SPRITES- 2))%NUM_SPRITES;
    losesprite2 = (winsprite + 1 + rand() % (NUM_SPRITES - 2))% NUM_SPRITES;
    TIM2->CR1 &= ~TIM_CR1_CEN;
    TIM2->SR &= ~TIM_SR_UIF;
}

void TIM3_IRQHandler()
{
    //Handles state of the screen picture
    if(scrolling)
    {
        int sprite1 = rand() % NUM_SPRITES;
        int sprite2 = rand() % NUM_SPRITES;
        int sprite3 = rand() % NUM_SPRITES;
        static int yshift = 4;


        load_bg();
        printSprite(sprite1, 2*3, yshift);
        printSprite(sprite2, 12*3, yshift);
        printSprite(sprite3, 22*3, yshift);

        yshift += 1;

        if(yshift > 7) yshift = 1;
    }

    else
    {
        load_bg();
        if(won)
        {
            printSprite(winsprite, 2*3, 4);
            printSprite(winsprite, 12*3, 4);
            printSprite(winsprite, 22*3, 4);

        }
        else
        {
            printSprite(winsprite, 2*3, 4);
            printSprite(losesprite1, 12*3, 4);
            printSprite(losesprite2, 22*3, 4);
        }
    }

    //also used to check push button
    buttondown = GPIOC->IDR & GPIO_IDR_0;
    if(buttondown && !scrolling)
    {
        TIM2->CNT = 0;
        TIM2->CR1 |= TIM_CR1_CEN;
        scrolling = 1;
    }

    TIM3->SR &= ~(1);



}


int main(void)
{
    init_GPIO();
    init_tim6();
    init_tim3();
    init_tim2();

    while(1);
}
