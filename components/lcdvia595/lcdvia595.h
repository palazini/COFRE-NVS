#ifndef __LCDVIA595_H
    #define __LCDVIA595__H
    //#include "ioplaca.h"

    void lcd595_init(void);
    void lcd595_pulse(void);
    void lcd595_write(int linha, int coluna, const char *str);
    void lcd595_byte(uint8_t byte, uint8_t rs);
    void lcd595_clear(void);
    void Enviar_lcd595(uint8_t dado);

#endif