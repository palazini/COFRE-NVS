#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>
//#include <string.h>
//#include "esp_system.h"

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

#define portTICK_RATE_MS portTICK_PERIOD_MS

char le_teclado ()
{        
    int i,j;
    uint8_t linha = 0b1000;
    uint8_t coluna = 0; 
    uint8_t mostra = 0;
    char tecla = '_';

    for (j = 0; j < 4; j++)
    {
        gpio_set_level(TEC_SH_LD,1);

        for(i = 7; i >= 0; i--)
        {    
            if(i < 4 )
            {
                if(gpio_get_level(TEC_DT_RD) == 1)
                {
                    coluna = i;
                    mostra = linha * 10 + coluna;
                    //  ESP_LOGI("captura", "L %i - C %i", linha, coluna);
                    vTaskDelay(100 / portTICK_RATE_MS);
                }
            }

            gpio_set_level(TEC_DT_WR, (linha >> i) & 1 );
            vTaskDelay(2 / portTICK_RATE_MS);
            gpio_set_level(TEC_CK, 1);
            vTaskDelay(2 / portTICK_RATE_MS);
            gpio_set_level(TEC_CK, 0);
            
        }
        gpio_set_level(TEC_SH_LD,0);

        if(linha <= 1)
        {
            linha = 0b1000;
        }
        else
        {
            linha >>= 1;
        }
    }
    switch (mostra)
    {
    case 10:
        tecla = '1';
        break;
    case 11:
        tecla = '2';
        break;
    case 12:
        tecla = '3';
        break;
    case 13:
        tecla = '-';
        break;
    case 20:
        tecla = 'C';
        break;
    case 21:
        tecla = '0';
        break;
    case 22:
        tecla = '=';
        break;
    case 23:
        tecla = '+';
        break;
    case 40:
        tecla = '7';
        break;
    case 41:
        tecla = '8';
        break;
    case 42:
        tecla = '9';
        break;
    case 43:
        tecla = '/';
        break;
    case 80:
        tecla = '4';
        break;
    case 81:
        tecla = '5';
        break;
    case 82:
        tecla = '6';
        break;
    case 83:
        tecla = 'x';
        break;
    default:
        tecla = '_';
        break;
    }
    return tecla;

}

uint8_t exp_le_escreve (uint8_t enviar)//>26 microssegundos
{
    uint8_t recebido = 0;
    int j;
    gpio_set_level(EXP_SH_LD,1);
    //ets_delay_us(1);  
    for (j = 7; j >= 0; j--)
    {
        recebido <<= 1;
        recebido += gpio_get_level(EXP_DT_RD);
        gpio_set_level(EXP_DT_WR, ( enviar >> j ) & 1);
       // ets_delay_us(1);  
        gpio_set_level(EXP_CK,1);
      //  ets_delay_us(1);  
        gpio_set_level(EXP_CK,0);
     //   ets_delay_us(1);  
    } 
    gpio_set_level(EXP_SH_LD,0);
    //ets_delay_us(1);  

    return recebido;
}

uint8_t io_le_escreve(uint8_t saidas)
{
    int j;
    uint8_t entradas = 0;
    gpio_set_level(IO_SH_LD,1);
   // ets_delay_us(10); //vTaskDelay(10 / portTICK_RATE_MS);  
    for (j = 7; j >= 0; j--)
    {
        entradas <<= 1;
        entradas += gpio_get_level(IO_DT_RD);
        gpio_set_level(IO_DT_WR, ( saidas >> j ) & 1);
      //  ets_delay_us(10); //vTaskDelay(10 / portTICK_RATE_MS);
        gpio_set_level(IO_CK,1);
      //  ets_delay_us(10); //vTaskDelay(10 / portTICK_RATE_MS);
        gpio_set_level(IO_CK,0);
       // ets_delay_us(10); //vTaskDelay(10 / portTICK_RATE_MS);
    } 
    gpio_set_level(IO_SH_LD,0);
   // ets_delay_us(10); //vTaskDelay(10 / portTICK_RATE_MS); 
    gpio_set_level(IO_SH_LD,1);//testar
    gpio_set_level(IO_SH_LD,0);
    return entradas;

}

void ioinit(void)
{
    gpio_reset_pin(LCD_DT_WR);
    gpio_reset_pin(LCD_CK);
    gpio_reset_pin(LCD_SH_LD);

    gpio_set_direction(LCD_DT_WR, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_SH_LD, GPIO_MODE_OUTPUT);
    gpio_set_direction(LCD_CK, GPIO_MODE_OUTPUT);

    gpio_set_level(LCD_DT_WR, 0);
    gpio_set_level(LCD_SH_LD, 0);
    gpio_set_level(LCD_CK, 0);

    gpio_reset_pin(TEC_DT_WR);
    gpio_reset_pin(TEC_CK);
    gpio_reset_pin(TEC_SH_LD);
    gpio_reset_pin(TEC_DT_WR);
    gpio_set_direction(TEC_DT_WR, GPIO_MODE_OUTPUT);
    gpio_set_direction(TEC_SH_LD, GPIO_MODE_OUTPUT);
    gpio_set_direction(TEC_CK, GPIO_MODE_OUTPUT);
    gpio_set_direction(TEC_DT_RD, GPIO_MODE_INPUT);
    gpio_set_level(TEC_DT_WR,0);
    gpio_set_level(TEC_CK,0);
    gpio_set_level(TEC_SH_LD,0);

    gpio_reset_pin(EXP_DT_WR);
    gpio_reset_pin(EXP_CK);
    gpio_reset_pin(EXP_SH_LD);
    gpio_reset_pin(EXP_DT_WR);
    gpio_set_direction(EXP_DT_WR, GPIO_MODE_OUTPUT);
    gpio_set_direction(EXP_SH_LD, GPIO_MODE_OUTPUT);
    gpio_set_direction(EXP_CK, GPIO_MODE_OUTPUT);
    gpio_set_direction(EXP_DT_RD, GPIO_MODE_INPUT);
    gpio_set_level(EXP_DT_WR,0);
    gpio_set_level(EXP_CK,0);
    gpio_set_level(EXP_SH_LD,0);
    
    gpio_reset_pin(IO_DT_WR);
    gpio_reset_pin(IO_CK);
    gpio_reset_pin(IO_SH_LD);
    gpio_set_direction(IO_DT_WR, GPIO_MODE_OUTPUT);
    gpio_set_direction(IO_SH_LD, GPIO_MODE_OUTPUT);
    gpio_set_direction(IO_CK, GPIO_MODE_OUTPUT);
    gpio_set_direction(IO_DT_RD, GPIO_MODE_INPUT);
    gpio_set_level(IO_DT_WR,0);
    gpio_set_level(IO_CK,0);
    gpio_set_level(IO_SH_LD,0);


}



