#ifndef __HCF_ADC_H
    #define __HCF_ADC_H
   
    #include "esp_err.h"

    // Inicializa o componente ADC
    esp_err_t hcf_adc_iniciar(void);

    // Lê o valor de uma entrada analógica
    esp_err_t hcf_adc_ler(uint32_t *value);

    esp_err_t hcf_adc_ler_3(uint32_t *value);

    // Finaliza o componente ADC
    void hcf_adc_limpar(void);

#endif