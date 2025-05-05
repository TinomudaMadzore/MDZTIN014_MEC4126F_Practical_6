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

    GPIOA -> MODER |= GPIO_MODER_MODER6;    // Set PA6 to Analog
    
    RCC -> APB2ENR |= RCC_APB2ENR_ADCEN;    // Enable ADC clock
    ADC1 -> CR &= ~ADC_CR_ADEN;             // Ensure ADC is disabled before configuration

    
    
    ADC1 -> CHSELR |= ADC_CHSELR_CHSEL6;    // Select Channel 6 on ADC
    ADC1->CFGR1 &= ~ADC_CFGR1_RES;          // Clear resolution bits
    ADC1 -> CFGR1 |= (ADC_CFGR1_RES_1 |     // Sets the ADC resolution to 8 bits
                      ADC_CFGR1_WAIT |      // Enable wait mode
                      ADC_CFGR1_ALIGN);     // Enable left align

    ADC1 -> IER |= ADC_IER_EOCIE;           // Enable End of conversion interrupt

    NVIC_EnableIRQ(ADC1_COMP_IRQn);         // Enable Interrupt on ADC

    ADC1->CR |= ADC_CR_ADCAL;               // Perform ADC calibration
    while ((ADC1->CR & ADC_CR_ADCAL) != 0); // Wait until calibration finishes
    
    ADC1->CR |= ADC_CR_ADEN; // Set ADEN=1 in ADC_CR register, actually starts ADC
    while(!(ADC1 -> ISR & ADC_ISR_ADRDY)); // Wait for ADC to be ready to start converting

    ADC1->CFGR1 |= ADC_CFGR1_CONT; // Start continuous conversion
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
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    GPIOB -> MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1);
    GPIOB -> MODER |= (GPIO_MODER_MODER0_1 | GPIO_MODER_MODER1_1);
    GPIOB -> AFR[0] &= ~((0x0F << GPIO_AFRL_AFRL0_Pos) | (0x0F << GPIO_AFRL_AFRL1_Pos));
    GPIOB -> AFR[0] |= ((0x01 << GPIO_AFRL_AFRL0_Pos) | (0x01 << GPIO_AFRL_AFRL1_Pos));

    TIM3 -> CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M); // Corrected: Use CCMR1 for channels 1 & 2
    TIM3 -> CCMR1 |= (6 << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE |
                     (6 << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE; // PWM mode 1 for CH1 & CH2

    TIM3->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E); // Enable output on CH1 and CH2 (PB0 & PB1 are usually CH1 & CH2 for TIM3)
    TIM3->CR1 |= TIM_CR1_ARPE;

    TIM3 -> PSC = 47;
    TIM3 -> ARR = 99;

    TIM3 -> CCR1 = 0; // PB0 Initial Duty (CH1)
    TIM3 -> CCR2 = TIM3->ARR; // PB1 Initial duty anti-phase (CH2)

    TIM3 -> CR1 |= TIM_CR1_CEN;
}

void ADC1_COMP_IRQHandler(void){
    if (ADC1 -> ISR & ADC_ISR_EOC){
        uint8_t adc_val = ADC1 -> DR;            // Read 8-bit ADC result

        if (SW0_PRESSED == False && !(GPIOA->IDR & GPIO_IDR_0)) { // check if button is pressed
            SW0_PRESSED = True;

            GPIOB -> ODR &= ~0xFF;  // Clear PB0–PB7

            // Reset PB2–PB7 to input mode
            GPIOB->MODER &= ~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3 |
                              GPIO_MODER_MODER4 | GPIO_MODER_MODER5 |
                              GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
            init_TIM3();  // Initialise timer
        }

        if (!SW0_PRESSED) {
            GPIOB->ODR = (GPIOB->ODR & 0xFF) | adc_val;  // Show ADC value on PB0–PB7
        } else{
            TIM3 -> CCR3 = adc_val;        // PB0 = CH3
            TIM3 -> CCR4 = 255 - adc_val;  // PB1 = CH4 (anti-phase)
        }

        ADC1 -> ISR |= ADC_ISR_EOC;

    }
}