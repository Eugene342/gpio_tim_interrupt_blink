#include <stdint.h>
#define __IO volatile

// Структура регистров GPIO
typedef struct
{
    __IO uint32_t MODER;
    __IO uint32_t OTYPER;
    __IO uint32_t OSPEEDR;
    __IO uint32_t PUPDR;
    __IO uint32_t IDR;
    __IO uint32_t ODR;
    __IO uint32_t BSRR;
    __IO uint32_t LCKR;
    __IO uint32_t AFR[2];
} GPIO_TypeDef;

// Структура регистров таймера TIM2
typedef struct
{
    __IO uint32_t CR1;
    __IO uint32_t CR2;
    __IO uint32_t SMCR;
    __IO uint32_t DIER;
    __IO uint32_t SR;
    __IO uint32_t EGR;
    __IO uint32_t CCMR1;
    __IO uint32_t CCMR2;
    __IO uint32_t CCER;
    __IO uint32_t CNT;
    __IO uint32_t PSC;
    __IO uint32_t ARR;
} TIM_TypeDef;

// Базовые адреса периферии
#define RCC_BASE        0x40023800U
#define GPIOA_BASE      0x40020000U
#define TIM2_BASE       0x40000000U
#define NVIC_ISER0_ADDR 0xE000E100U

// Смещения регистров в RCC
#define AHB1ENR_OFFSET  0x30U
#define APB1ENR_OFFSET  0x40U

// Макросы прямого доступа к регистрам
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + AHB1ENR_OFFSET))
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + APB1ENR_OFFSET))

#define GPIOA ((GPIO_TypeDef *)GPIOA_BASE)
#define TIM2  ((TIM_TypeDef *)TIM2_BASE)
#define NVIC_ISER0 (*(volatile uint32_t *)NVIC_ISER0_ADDR)

static uint16_t tick_count = 0;  // Счётчик прерываний таймера

// Обработчик прерывания TIM2
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & (1U << 0U))                    // Флаг переполнения (UIF)
    {
        TIM2->SR &= ~(1U << 0U);                  // Сброс флага

        tick_count++;
        if (tick_count >= 1000)                    // Каждые 1000 прерываний (1000 мс)
        {
            // Переключить светодиод на PA5
            if (GPIOA->ODR & (1U << 5U))
            {
                GPIOA->BSRR = (1U << (5U + 16U)); // Выключить через BSRR
            }
            else
            {
                GPIOA->BSRR = (1U << 5U);         // Включить через BSRR
            }
            tick_count = 0;
        }
    }
}

int main(void)
{
    // Включаем тактирование: GPIOA (AHB1) и TIM2 (APB1)
    RCC_AHB1ENR |= (1U << 0U);
    RCC_APB1ENR |= (1U << 0U);

    // Настраиваем PA5 (LD2) как выход
    GPIOA->MODER   &= ~(0x3U << (5U * 2U));  // Сброс битов режима
    GPIOA->MODER   |=  (0x1U << (5U * 2U));  // Выход (01)
    GPIOA->OTYPER  &= ~(1U << 5U);           // Push-pull
    GPIOA->OSPEEDR &= ~(0x3U << (5U * 2U));  // Средняя скорость
    GPIOA->PUPDR   &= ~(0x3U << (5U * 2U));  // Без подтяжки

    // Настройка TIM2: 1 мс на одно прерывание
    TIM2->PSC = 15U;           // Предделитель: 16 МГц / 16 = 1 МГц
    TIM2->ARR = 999U;          // Перезагрузка: 1000 тиков = 1 мс
    TIM2->DIER |= (1U << 0U);  // Разрешить прерывание по переполнению
    NVIC_ISER0 |= (1U << 28U); // Разрешить TIM2 в NVIC (IRQ 28)
    TIM2->CR1 |= (1U << 0U);   // Запуск таймера (CEN)

    while (1)
    {
        // Работа идёт в прерывании — здесь ничего не делаем
    }
}










