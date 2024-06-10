#ifndef __MP_HCF_H
    #define __MP_HCF_H
   // #include "esp_err.h"
 

    void MP_init(void);
    void rotacionar_MP(int horario, float graus);

    void DRV_init(int Fim_de_curso_inicial, int Fim_de_curso_final);
    void rotacionar_DRV(int abrir, float graus, uint8_t saidas_atual);
#endif