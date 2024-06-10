#ifndef __IOPLACA_H
    #define __IOPLACA_H
   // #include "esp_err.h"
    //void Enviar_lcd595(uint8_t dado);


    // LCD -> fazer o aterramento dos pinos flutuantes, 2 bits menos significativos inutilizados
    #define LCD_DT_WR   GPIO_NUM_18
    #define LCD_SH_LD   GPIO_NUM_5
    #define LCD_CK      GPIO_NUM_17
    // IOs -> 7 a 4 BJT coletor, 3 e 2 Triac, 1 e 0 Relé
    #define IO_DT_WR    GPIO_NUM_27
    #define IO_SH_LD    GPIO_NUM_14
    #define IO_CK       GPIO_NUM_12
    #define IO_DT_RD    GPIO_NUM_13
    // Teclado -> Linhas são saídas e colunas são entradas
    #define TEC_DT_WR   GPIO_NUM_16
    #define TEC_CK      GPIO_NUM_4
    #define TEC_SH_LD   GPIO_NUM_2
    #define TEC_DT_RD   GPIO_NUM_15
    // Motor DRV8825 / A4988 -> fazer o aterramento de pinos flutuantes
    #define MP_SLP      GPIO_NUM_22
    #define MP_STEP     GPIO_NUM_21
    #define MP_DIR      GPIO_NUM_19
    // Expansor 
    #define EXP_CK      GPIO_NUM_32
    #define EXP_SH_LD   GPIO_NUM_33
    #define EXP_DT_WR   GPIO_NUM_25
    #define EXP_DT_RD   GPIO_NUM_26

    void ioinit(void);
    uint8_t io_le_escreve (uint8_t saidas);
    uint8_t exp_le_escreve (uint8_t enviar);
    char le_teclado ();
#endif