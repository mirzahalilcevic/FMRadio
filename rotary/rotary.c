#include "rotary.h"

volatile uint8_t interrupt_state = 0;
volatile uint8_t interrupt_state_btn = 0;

volatile uint8_t button_state = 0;
volatile uint8_t LastA = 1;
volatile int32_t RE_Count = 0;
volatile int32_t previous_RE_Count = 0;
volatile uint8_t Mode = 1;

void RE_Proccessing(void);
void initGPIO_PB5(void);
void initEXTI3_PB3(void);
void initEXTI0_PA0(void);
void EXTI3_IRQHandler(void);

void init_rotary_encoder(void) {
  initEXTI3_PB3();
  initGPIO_PB5();
  initEXTI0_PA0();
}

void RE_Proccessing(void) {
  uint8_t now_a;
  uint8_t now_b;

  // Read inputs
  now_a = (GPIOB->IDR & GPIO_IDR_IDR_3) >> 3;
  now_b = (GPIOB->IDR & GPIO_IDR_IDR_5) >> 5;

  /* Check difference */
  if (now_a != LastA) {
    LastA = now_a;

    if (LastA == 0) {
      /* Check mode */
      if (!Mode) {
        previous_RE_Count = RE_Count;
        if (now_b == 1) {
          RE_Count--;
        } else {
          RE_Count++;
        }
      } else {
        previous_RE_Count = RE_Count;
        if (now_b == 1) {
          RE_Count++;
        } else {
          RE_Count--;
        }
      }
    }
  }
}

void initGPIO_PB5(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  GPIOB->MODER &= ~GPIO_MODER_MODER5_1;
  GPIOB->MODER &= ~GPIO_MODER_MODER5_0;
  GPIOB->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR5_1;
  GPIOB->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR5_0;
  GPIOB->PUPDR &= ~GPIO_PUPDR_PUPDR5_1;
  GPIOB->PUPDR |= GPIO_PUPDR_PUPDR5_0;
}

void initEXTI0_PA0(void) {
  // enable PA0
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  GPIOA->MODER &= ~GPIO_MODER_MODER0_1;
  GPIOA->MODER &= ~GPIO_MODER_MODER0_0;
  GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR0_1;
  GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR0_0;
  GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR0_1;
  GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_0;

  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // enable clock on SYSCFG register
  NVIC_EnableIRQ(EXTI0_IRQn);
  SYSCFG->EXTICR[0] |=
      SYSCFG_EXTICR1_EXTI0_PA; // select PA 0 as interrupt source p259
  EXTI->IMR |= EXTI_IMR_MR0;   // enable interrupt on EXTI_Line0
  EXTI->EMR &= ~EXTI_EMR_MR0;  // disable event on EXTI_Line0
  // EXTI->RTSR |= EXTI_RTSR_TR0; // enable rising
  // edge trigger
  EXTI->FTSR |= EXTI_FTSR_TR0; // enable falling edge trigger
}

void EXTI0_IRQHandler(void) {
  if ((EXTI->PR & EXTI_PR_PR0) == EXTI_PR_PR0) // EXTI_Line0 interrupt pending?
  {
    interrupt_state_btn = 1;
    EXTI->PR = EXTI_PR_PR0; // clear EXTI_Line0 interrupt flag
  }
}

void initEXTI3_PB3(void) {
  // enable PB3 pin B
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  GPIOB->MODER &= ~GPIO_MODER_MODER3_1;
  GPIOB->MODER &= ~GPIO_MODER_MODER3_0;
  GPIOB->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR3_1;
  GPIOB->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR3_0;
  GPIOB->PUPDR &= ~GPIO_PUPDR_PUPDR3_1;
  GPIOB->PUPDR |= GPIO_PUPDR_PUPDR3_0;

  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // enable clock on SYSCFG register
  NVIC_EnableIRQ(EXTI3_IRQn);
  SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PB;
  EXTI->IMR |= EXTI_IMR_MR3;   // enable interrupt on EXTI_Line0
  EXTI->EMR &= ~EXTI_EMR_MR3;  // disable event on EXTI_Line0
  EXTI->RTSR |= EXTI_RTSR_TR3; // enable rising edge trigger
  EXTI->FTSR |= EXTI_FTSR_TR3; // enable falling edge trigger
}

void EXTI3_IRQHandler(void) {
  if ((EXTI->PR & EXTI_PR_PR3) == EXTI_PR_PR3) // EXTI_Line0 interrupt pending?
  {
    interrupt_state = 1;
    EXTI->PR = EXTI_PR_PR3; // clear EXTI_Line0 interrupt flag
  }
}

void serviceIRQA(void) {
  if (interrupt_state == 1) {
    RE_Proccessing();
    interrupt_state = 0;
  }
  if (interrupt_state_btn == 1) {
    button_state = 1;
    interrupt_state_btn = 0;
  }
}
