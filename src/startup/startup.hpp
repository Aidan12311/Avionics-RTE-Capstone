#ifndef AVIONICS_RTE_STARTUP_HPP
#define AVIONICS_RTE_STARTUP_HPP

extern "C" {
    void reset_handler(void);
    void default_handler(void);
    
    void systick_handler(void);
    void pendsv_handler(void);

    void usart1_irq_handler(void);
    void usart2_irq_handler(void);
    
    void spi1_irq_handler(void);
}

#endif // AVIONICS_RTE_STARTUP_HPP
