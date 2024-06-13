idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES hcf_adc
                    REQUIRES ioplaca
                    REQUIRES lcdvia595
                    REQUIRES mp_hcf
                    REQUIRES nvs_flash
                    )

set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common
    )
