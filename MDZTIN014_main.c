/**
 * @file: main.c
 * @author: Tinomuda Madzore
 * @student-number: MDZTIN014
 * @ps-no: 1878246
 *
 * Student solution for practical 6.
 */

//Macros
#define STM32F051  //define target for header files, must be def'd before includes
#define True 1
#define False 0

//Includes
#include "stm32f0xx.h"											   
#include "lcd_stm32f0.h"
#include <stdint.h>

//Global Variables
volatile uint8_t SW0_PRESSED = False;

//Function Declerations
void ResetClockTo48Mhz(void);
void init_student(void);
void init_ADC(void);
void init_GPIOB(void);
void init_GPIOA(void);
void init_TIM3 (void);

//Main Function
int main (void){
    ResetClockTo48Mhz();
    init_student();
    init_GPIOA();
    init_GPIOB();
    init_ADC();
    while (1) {}
}						

//Function Definitions
/**
 * Function resets the STM32 Clocks to 48 MHz
 */
void ResetClockTo48Mhz(void)
{
    if ((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL)
    {
        RCC->CFGR &= (uint32_t) (~RCC_CFGR_SW);
        while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);
    }
    RCC->CR &= (uint32_t)(~RCC_CR_PLLON);
    while ((RCC->CR & RCC_CR_PLLRDY) != 0);
    RCC->CFGR = ((RCC->CFGR & (~0x003C0000)) | 0x00280000);
    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0);
    RCC->CFGR |= (uint32_t) (RCC_CFGR_SW_PLL);
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}

void init_student(void){
    init_LCD();
    lcd_command(CLEAR);
    lcd_putstring("MDZTIN014");
}

void init_ADC(void){
    // Sets up The ADC to read from channel 6 (PA6) in 8-bit resolution and discontinuous conversion mode

    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;  //Enable GPIOA clock
    GPIOA -> MODER |= GPIO_MODER_MODER6;    // Set PA6 to Analog
    
    RCC -> APB2ENR |= RCC_APB2ENR_ADCEN;    // Enable ADC clock
    ADC1 -> CR &= ~ADC_CR_ADEN;             // Ensure ADC is disabled before configuration

    ADC1 -> CFGR1 &= ~ADC_CFGR1_RES;          // Clear resolution bits
    ADC1 -> CFGR1 |= ADC_CFGR1_RES_1;     // Sets the ADC resolution to 8 bits   
    ADC1 -> CFGR1 |= ADC_CFGR1_ALIGN;     // Enable left align
    ADC1 -> CFGR1 |= ADC_CFGR1_WAIT;      // Enable wait mode
    ADC1 -> CFGR1 |= ADC_CFGR1_CONT; // Start continuous conversion
    ADC1 -> IER |= ADC_IER_EOCIE;           // Enable End of conversion interrupt

    ADC1 -> CHSELR |= ADC_CHSELR_CHSEL6;    // Select Channel 6 on ADC
    ADC1->CR |= ADC_CR_ADCAL;               // Perform ADC calibration
    while ((ADC1->CR & ADC_CR_ADCAL) != 0); // Wait until calibration finishes

    ADC1->CR |= ADC_CR_ADEN; // Set ADEN=1 in ADC_CR register, actually starts ADC
    while(!(ADC1 -> ISR & ADC_ISR_ADRDY)); // Wait for ADC to be ready to start converting

    NVIC_EnableIRQ(ADC1_COMP_IRQn);         // Enable Interrupt on ADC

    // Start ADC conversion
    ADC1->CR |= ADC_CR_ADSTART;
}

void init_GPIOB(void){
    
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;              // Enable clock for GPIOB (LEDs)
  
    GPIOB -> MODER &= ~(                            // Clear bits for PB0–PB7
        GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | 
        GPIO_MODER_MODER2 | GPIO_MODER_MODER3 |
        GPIO_MODER_MODER4 | GPIO_MODER_MODER5 |
        GPIO_MODER_MODER6 | GPIO_MODER_MODER7);

    GPIOB -> MODER |= (                             // Set PB0–PB7 as output
        GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0 | 
        GPIO_MODER_MODER2_0 | GPIO_MODER_MODER3_0 | 
        GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0 | 
        GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0);


}

void init_GPIOA(void){
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;            // Enable clock for GPIOA port

    GPIOA -> MODER &= ~GPIO_MODER_MODER0;         // Set PA0 as input

    GPIOA -> PUPDR &= ~GPIO_PUPDR_PUPDR0;           // Clear existing pull/push config
    GPIOA -> PUPDR |= GPIO_PUPDR_PUPDR0_0;        // Enable pull-up resistors for PA0 (UP because it can float)
}

void init_TIM3 (void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;  // Enable the TIM3 clock

    // Set PB0 & PB1 to alternate function mode, AF1 for TIM3 ch3 and ch4, (set to AF1 to connect to the PWM channels of TIM3)
    GPIOB -> MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1); //clear bits first
    GPIOB -> MODER |= (GPIO_MODER_MODER0_1 | GPIO_MODER_MODER1_1); // Set to AF mode
    GPIOB -> AFR[0] &= ~((0xF << GPIO_AFRL_AFRL0_Pos) | (0xF << GPIO_AFRL_AFRL1_Pos)); //clears the existing AF setting.
    GPIOB -> AFR[0] |= ((1 << GPIO_AFRL_AFRL0_Pos) | (1 << GPIO_AFRL_AFRL1_Pos)); // set both PB0 and PB1 to  AF1

    TIM3 -> CCMR2 &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_OC4M); //clear bits
    TIM3 -> CCMR2 |= (6 << TIM_CCMR2_OC3M_Pos) | (6 << TIM_CCMR2_OC4M_Pos); // PWM mode 1 for CH3 & CH4
    TIM3 -> CCMR2 |= TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE; // Preload enable

    TIM3->CCER |= (TIM_CCER_CC3E | TIM_CCER_CC4E); // Enable output on CH3 and CH4

    TIM3 -> PSC = 0;
    TIM3 -> ARR = 4799; // 48Mhz/(4799+1) = 10khz

    TIM3 -> CCR3 = 0; // PB0 Initial Duty (CH3)
    TIM3 -> CCR4 = TIM3->ARR; // PB1 Initial duty anti-phase (CH4)

    TIM3 -> CR1 |= TIM_CR1_CEN; // Start the timer
}

void ADC1_COMP_IRQHandler(void){
    if (ADC1 -> ISR & ADC_ISR_EOC){ // Check if its the end of a conversion
        uint8_t adc_val = (ADC1->DR  >> 8);            // Read 8-bit ADC result, need to shift down 8 bits as its left aligned from DR

        if (!SW0_PRESSED && !(GPIOA->IDR & GPIO_IDR_0)) { // check if button is pressed
            SW0_PRESSED = True;

            GPIOB -> ODR &= ~0xFF;  // Clear PB0–PB7

            // Reset PB2–PB7 to input mode
            GPIOB->MODER &= ~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3 |
                              GPIO_MODER_MODER4 | GPIO_MODER_MODER5 |
                              GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
            init_TIM3();  // Initialise timer/PWM
        }

        if (!SW0_PRESSED) {
            GPIOB->ODR = (GPIOB->ODR & ~0xFF) | adc_val;  // Show ADC value on PB0–PB7
        } else{

            TIM3 -> CCR3 = adc_val;        // PWM for PB0
            TIM3 -> CCR4 = 255 - adc_val;  // PB1 (anti-phase)
        }
        // Clear EOC flag
        ADC1 -> ISR |= ADC_ISR_EOC;

    }
}