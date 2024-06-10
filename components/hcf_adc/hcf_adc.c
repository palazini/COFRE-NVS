#include "hcf_adc.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"

static const char *TAG = "HCF_ADC";

// Defina o canal ADC que você está usando para a entrada analógica
#define HCF_ADC_CANAL_0  ADC_CHANNEL_0
#define HCF_ADC_CANAL_3  ADC_CHANNEL_3

// Use esta atenuação para obter leituras precisas de uma entrada entre 0V e 3.3V
#define HCF_ADC_ATENUA    ADC_ATTEN_DB_11

#define HCF_ADC_AMOSTRAS  256  // Número de amostras para média

static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t calibration_handle = NULL;
static bool calibration_enabled = false;
static bool adc_calibra_inicializa(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void adc_calibra_finaliza(adc_cali_handle_t handle);

esp_err_t hcf_adc_iniciar(void) {
    // Configuração da unidade ADC

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };

    esp_err_t err = adc_oneshot_new_unit(&init_config, &adc_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao inicializar a unidade ADC");
        return err;
    }

    // Configuração do canal ADC
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = HCF_ADC_ATENUA,
    };
    
    err = adc_oneshot_config_channel(adc_handle, HCF_ADC_CANAL_0, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao configurar o canal ADC");
        return err;
    }

    //added
    err = adc_oneshot_config_channel(adc_handle, HCF_ADC_CANAL_3, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao configurar o canal ADC");
        return err;
    }
    // Inicializa a calibração se disponível
  ////// // calibration_enabled = adc_calibra_inicializa(ADC_UNIT_1, HCF_ADC_CANAL_0, HCF_ADC_ATENUA, &calibration_handle);

    return ESP_OK;
}

esp_err_t hcf_adc_ler(uint32_t *value) {
    int adc_value;
    uint32_t calibrated_value = 0;

    for (int i = 0; i < HCF_ADC_AMOSTRAS; i++) {
        esp_err_t err = adc_oneshot_read(adc_handle, HCF_ADC_CANAL_0, &adc_value);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Erro na leitura ADC");
            return err;
        }
        calibrated_value += adc_value;
    }

    calibrated_value /= HCF_ADC_AMOSTRAS;

    if (calibration_enabled) {
        int voltage_value;
        esp_err_t err = adc_cali_raw_to_voltage(calibration_handle, (int)calibrated_value, &voltage_value);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Erro na calibração");
            return err;
        }
        *value = (uint32_t)voltage_value;
    } else {
        *value = (uint32_t)calibrated_value;
    }

    return ESP_OK;
}

esp_err_t hcf_adc_ler_3(uint32_t *value) {
    int adc_value;
    uint32_t calibrated_value = 0;

    for (int i = 0; i < HCF_ADC_AMOSTRAS; i++) {
        esp_err_t err = adc_oneshot_read(adc_handle, HCF_ADC_CANAL_3, &adc_value);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Erro na leitura ADC");
            return err;
        }
        calibrated_value += adc_value;
    }

    calibrated_value /= HCF_ADC_AMOSTRAS;

    *value = (uint32_t)calibrated_value;

    return ESP_OK;
}

void hcf_adc_limpar(void) {
    adc_oneshot_del_unit(adc_handle);
    if (calibration_enabled) {
        adc_calibra_finaliza(calibration_handle);
    }
}

/*---------------------------------------------------------------
        ADC Calibration - Retirado do exemplo
---------------------------------------------------------------*/
static bool adc_calibra_inicializa(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

static void adc_calibra_finaliza(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}