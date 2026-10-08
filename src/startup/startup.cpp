#include <cstdint>

#include "src/startup/startup.hpp"
#include "src/hal/arm_core.hpp"

extern uint32_t _estack;

extern uint32_t _sdata;
extern uint32_t _edata;

extern uint32_t _sidata;

extern uint32_t _sbss;
extern uint32_t _ebss;

extern uint32_t _init_array_start;
extern uint32_t _init_array_end;

// NOTE: Empty handler for all unhandled interrupts
extern "C" void default_handler(void) { while(1); }

// Placeholder weak handlers for specific interrupts until we actually fill them in
extern "C" __attribute__((weak)) void systick_handler(void)    { while(1); }
extern "C" __attribute__((weak)) void pendsv_handler(void)     { while(1); }
extern "C" __attribute__((weak)) void spi1_irq_handler(void)   { while(1); }
extern "C" __attribute__((weak)) void usart2_irq_handler(void) { while(1); }

__attribute__((section(".isr_vector"), used))
const uint32_t vector_table[] = {
    // ── Core ARM exceptions ─────────────────────────────────────────
    (uint32_t)&_estack,               // [0]  Initial SP
    (uint32_t)&reset_handler,         // [1]  Reset
    (uint32_t)&default_handler,       // [2]  NMI
    (uint32_t)&default_handler,       // [3]  HardFault
    (uint32_t)&default_handler,       // [4]  MemManage
    (uint32_t)&default_handler,       // [5]  BusFault
    (uint32_t)&default_handler,       // [6]  UsageFault
    0,                                // [7]  Reserved
    0,                                // [8]  Reserved
    0,                                // [9]  Reserved
    0,                                // [10] Reserved
    (uint32_t)&default_handler,       // [11] SVCall
    (uint32_t)&default_handler,       // [12] DebugMon
    0,                                // [13] Reserved
    (uint32_t)&pendsv_handler,        // [14] PendSV  — context switcher
    (uint32_t)&systick_handler,       // [15] SysTick — scheduler tick

    // ── IRQ0–IRQ88: peripheral interrupts (IRQ position = index - 16) ──
    (uint32_t)&default_handler,       // [16] IRQ0  WWDG
    (uint32_t)&default_handler,       // [17] IRQ1  PVD
    (uint32_t)&default_handler,       // [18] IRQ2  TAMP_STAMP
    (uint32_t)&default_handler,       // [19] IRQ3  RTC_WKUP
    (uint32_t)&default_handler,       // [20] IRQ4  FLASH
    (uint32_t)&default_handler,       // [21] IRQ5  RCC
    (uint32_t)&default_handler,       // [22] IRQ6  EXTI0
    (uint32_t)&default_handler,       // [23] IRQ7  EXTI1
    (uint32_t)&default_handler,       // [24] IRQ8  EXTI2
    (uint32_t)&default_handler,       // [25] IRQ9  EXTI3
    (uint32_t)&default_handler,       // [26] IRQ10 EXTI4
    (uint32_t)&default_handler,       // [27] IRQ11 DMA1_Stream0
    (uint32_t)&default_handler,       // [28] IRQ12 DMA1_Stream1
    (uint32_t)&default_handler,       // [29] IRQ13 DMA1_Stream2
    (uint32_t)&default_handler,       // [30] IRQ14 DMA1_Stream3
    (uint32_t)&default_handler,       // [31] IRQ15 DMA1_Stream4
    (uint32_t)&default_handler,       // [32] IRQ16 DMA1_Stream5
    (uint32_t)&default_handler,       // [33] IRQ17 DMA1_Stream6
    (uint32_t)&default_handler,       // [34] IRQ18 ADC
    (uint32_t)&default_handler,       // [35] IRQ19 CAN1_TX
    (uint32_t)&default_handler,       // [36] IRQ20 CAN1_RX0
    (uint32_t)&default_handler,       // [37] IRQ21 CAN1_RX1
    (uint32_t)&default_handler,       // [38] IRQ22 CAN1_SCE
    (uint32_t)&default_handler,       // [39] IRQ23 EXTI9_5
    (uint32_t)&default_handler,       // [40] IRQ24 TIM1_BRK_TIM9
    (uint32_t)&default_handler,       // [41] IRQ25 TIM1_UP_TIM10
    (uint32_t)&default_handler,       // [42] IRQ26 TIM1_TRG_COM_TIM11
    (uint32_t)&default_handler,       // [43] IRQ27 TIM1_CC
    (uint32_t)&default_handler,       // [44] IRQ28 TIM2
    (uint32_t)&default_handler,       // [45] IRQ29 TIM3
    (uint32_t)&default_handler,       // [46] IRQ30 TIM4
    (uint32_t)&default_handler,       // [47] IRQ31 I2C1_EV
    (uint32_t)&default_handler,       // [48] IRQ32 I2C1_ER
    (uint32_t)&default_handler,       // [49] IRQ33 I2C2_EV
    (uint32_t)&default_handler,       // [50] IRQ34 I2C2_ER
    (uint32_t)&spi1_irq_handler,       // [51] IRQ35 SPI1  — IMU
    (uint32_t)&default_handler,       // [52] IRQ36 SPI2
    (uint32_t)&default_handler,       // [53] IRQ37 USART1
    (uint32_t)&usart2_irq_handler,     // [54] IRQ38 USART2 — telemetry
    (uint32_t)&default_handler,       // [55] IRQ39 USART3
    (uint32_t)&default_handler,       // [56] IRQ40 EXTI15_10
    (uint32_t)&default_handler,       // [57] IRQ41 RTC_Alarm
    (uint32_t)&default_handler,       // [58] IRQ42 OTG_FS_WKUP
    (uint32_t)&default_handler,       // [59] IRQ43 TIM8_BRK_TIM12
    (uint32_t)&default_handler,       // [60] IRQ44 TIM8_UP_TIM13
    (uint32_t)&default_handler,       // [61] IRQ45 TIM8_TRG_COM_TIM14
    (uint32_t)&default_handler,       // [62] IRQ46 TIM8_CC
    (uint32_t)&default_handler,       // [63] IRQ47 DMA1_Stream7
    (uint32_t)&default_handler,       // [64] IRQ48 FSMC
    (uint32_t)&default_handler,       // [65] IRQ49 SDIO
    (uint32_t)&default_handler,       // [66] IRQ50 TIM5
    (uint32_t)&default_handler,       // [67] IRQ51 SPI3
    (uint32_t)&default_handler,       // [68] IRQ52 UART4
    (uint32_t)&default_handler,       // [69] IRQ53 UART5
    (uint32_t)&default_handler,       // [70] IRQ54 TIM6_DAC
    (uint32_t)&default_handler,       // [71] IRQ55 TIM7
    (uint32_t)&default_handler,       // [72] IRQ56 DMA2_Stream0
    (uint32_t)&default_handler,       // [73] IRQ57 DMA2_Stream1
    (uint32_t)&default_handler,       // [74] IRQ58 DMA2_Stream2
    (uint32_t)&default_handler,       // [75] IRQ59 DMA2_Stream3
    (uint32_t)&default_handler,       // [76] IRQ60 DMA2_Stream4
    (uint32_t)&default_handler,       // [77] IRQ61 ETH
    (uint32_t)&default_handler,       // [78] IRQ62 ETH_WKUP
    (uint32_t)&default_handler,       // [79] IRQ63 CAN2_TX
    (uint32_t)&default_handler,       // [80] IRQ64 CAN2_RX0
    (uint32_t)&default_handler,       // [81] IRQ65 CAN2_RX1
    (uint32_t)&default_handler,       // [82] IRQ66 CAN2_SCE
    (uint32_t)&default_handler,       // [83] IRQ67 OTG_FS
    (uint32_t)&default_handler,       // [84] IRQ68 DMA2_Stream5
    (uint32_t)&default_handler,       // [85] IRQ69 DMA2_Stream6
    (uint32_t)&default_handler,       // [86] IRQ70 DMA2_Stream7
    (uint32_t)&default_handler,       // [87] IRQ71 USART6
    (uint32_t)&default_handler,       // [88] IRQ72 I2C3_EV
    (uint32_t)&default_handler,       // [89] IRQ73 I2C3_ER
    (uint32_t)&default_handler,       // [90] IRQ74 OTG_HS_EP1_OUT
    (uint32_t)&default_handler,       // [91] IRQ75 OTG_HS_EP1_IN
    (uint32_t)&default_handler,       // [92] IRQ76 OTG_HS_WKUP
    (uint32_t)&default_handler,       // [93] IRQ77 OTG_HS
    (uint32_t)&default_handler,       // [94] IRQ78 DCMI
    (uint32_t)&default_handler,       // [95] IRQ79 CRYP
    (uint32_t)&default_handler,       // [96] IRQ80 HASH_RNG
    (uint32_t)&default_handler,       // [97] IRQ81 FPU
    // ── STM32F429-specific (Table 62 additions) ──────────────────────
    (uint32_t)&default_handler,       // [98]  IRQ82 UART7
    (uint32_t)&default_handler,       // [99]  IRQ83 UART8
    (uint32_t)&default_handler,       // [100] IRQ84 SPI4
    (uint32_t)&default_handler,       // [101] IRQ85 SPI5
    (uint32_t)&default_handler,       // [102] IRQ86 SPI6
    (uint32_t)&default_handler,       // [103] IRQ87 SAI1
    (uint32_t)&default_handler,       // [104] IRQ88 LTDC
};

static void enable_fpu() {
    constexpr uint32_t CPACR_ADDR = 0xE000ED88u;
    constexpr uint32_t FPU_ENABLE_MASK = (0xFu << 20);

    volatile uint32_t *cpacr = (volatile uint32_t*)CPACR_ADDR;
    *cpacr |= FPU_ENABLE_MASK;

    arm::dsb();
    arm::isb();
}

static void copy_data_section() {
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    uint32_t *end = &_edata;

    while (dst < end) {
        *dst++ = *src++;
    }
}

static void zero_bss_section() {
    uint32_t *start = &_sbss;
    uint32_t *end = &_ebss;

    while (start < end) {
        *start++ = 0u;
    }
}

static void call_ctors() {
    using ctor_fn = void (*)();

    ctor_fn *start = reinterpret_cast<ctor_fn*>(&_init_array_start);
    ctor_fn *end = reinterpret_cast<ctor_fn*>(&_init_array_end);

    while (start < end) {
        (*start++)();
    }
}

int main(void);

extern "C" void reset_handler(void) {
    enable_fpu();
    copy_data_section();
    zero_bss_section();
    call_ctors();

    main();
    while(1);
}
