#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <stdio.h>
#include "esp_system.h"
//#include "ioplaca.h"

// LCD -> fazer o aterramento dos pinos flutuantes, 2 bits menos significativos inutilizados
#define LCD_DT_WR   GPIO_NUM_18
#define LCD_SH_LD   GPIO_NUM_5
//#ifndef LCD_CK
#define LCD_CK      GPIO_NUM_17
//#endif
#define portTICK_RATE_MS portTICK_PERIOD_MS

uint8_t dado_atual;

void Enviar_lcd595(uint8_t dado)
{
    int i;    
   // ESP_LOGI(TAG_LCD, "%x", dado);
    gpio_set_level(LCD_CK, 0);
    for(i = 7; i >= 0; i--)
    { 
        gpio_set_level(LCD_DT_WR, (dado >> i) & 1 );
        vTaskDelay(1 / portTICK_RATE_MS);
        gpio_set_level(LCD_CK, 1);
        vTaskDelay(1 / portTICK_RATE_MS);
        gpio_set_level(LCD_CK, 0);
        vTaskDelay(1 / portTICK_RATE_MS);
    }
    gpio_set_level(LCD_SH_LD, 1);
    vTaskDelay(1 / portTICK_RATE_MS);
    gpio_set_level(LCD_SH_LD, 0);
}

void lcd595_pulse(void)
{
    Enviar_lcd595(dado_atual | 0b00001000);
    vTaskDelay(2 / portTICK_RATE_MS);

    Enviar_lcd595(dado_atual & 0b11110111);
    vTaskDelay(2 / portTICK_RATE_MS);
}

void lcd595_byte(uint8_t byte, uint8_t rs)
{
    dado_atual &= 0b11111011;
    dado_atual |= (rs * 0b100);                   // pino RS
    Enviar_lcd595(dado_atual);
    dado_atual &= 0x0F;                         // limpa dados
    dado_atual |= (byte & 0xF0);                // nibble alto
    Enviar_lcd595(dado_atual);
    lcd595_pulse();
    dado_atual &= 0x0F;
    dado_atual |= ((byte & 0x0F) * 0b10000);    // nibble baixo
    Enviar_lcd595(dado_atual); 
    lcd595_pulse();

    vTaskDelay(1 / portTICK_RATE_MS);
}


void lcd595_write(int linha, int coluna, const char *str)
{
    if(linha == 1)
    {
        lcd595_byte(0x80 + coluna, 0);
    } else {
        lcd595_byte(0xC0 + coluna, 0);
    }

    vTaskDelay(1 / portTICK_RATE_MS);
    
    while (*str)
    {
        lcd595_byte(*str++, 1);
    }
}

void lcd595_init(void)
{
    vTaskDelay(200 / portTICK_RATE_MS); 
    dado_atual = 0x00;

    vTaskDelay(15 / portTICK_RATE_MS); 

    vTaskDelay(15 / portTICK_RATE_MS);

    dado_atual = 0x30;
    Enviar_lcd595(dado_atual);
    lcd595_pulse();
    vTaskDelay(5 / portTICK_RATE_MS);

    dado_atual = 0x30;
    Enviar_lcd595(dado_atual);
    lcd595_pulse();
    vTaskDelay(1 / portTICK_RATE_MS);

    dado_atual = 0x30;
    Enviar_lcd595(dado_atual);
    lcd595_pulse();
    vTaskDelay(1 / portTICK_RATE_MS);

    dado_atual = 0x20;
    Enviar_lcd595(dado_atual);
    lcd595_pulse();
    vTaskDelay(1 / portTICK_RATE_MS);

    lcd595_byte(0x28, 0);
    lcd595_byte(0x0C, 0);
    //lcd595_byte(0x06, 0);
    lcd595_byte(0x01, 0);
    vTaskDelay(2 / portTICK_RATE_MS);
    dado_atual = 0x00;
    vTaskDelay(200 / portTICK_RATE_MS); 
}

void lcd595_clear(void)
{
    lcd595_byte(0x01, 0);
    vTaskDelay(20 / portTICK_RATE_MS);
}
