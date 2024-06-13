//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                       _              //
//               _    _       _      _        _     _   _   _    _   _   _        _   _  _   _          //
//           |  | |  |_| |\| |_| |\ |_|   |\ |_|   |_| |_| | |  |   |_| |_| |\/| |_| |  |_| | |   /|    //    
//         |_|  |_|  |\  | | | | |/ | |   |/ | |   |   |\  |_|  |_| |\  | | |  | | | |_ | | |_|   _|_   //
//                                                                                       /              //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
--     _____    ____    _____    _____    _____    ____       _____    ____    ______   _____    ______   --
--    / ____|  / __ \  |  __ \  |_   _|  / ____|  / __ \     / ____|  / __ \  |  ____| |  __ \  |  ____|  --
--   | |      | |  | | | |  | |   | |   | |  __  | |  | |   | |      | |  | | | |__    | |__) | | |__     --
--   | |      | |  | | | |  | |   | |   | | |_ | | |  | |   | |      | |  | | |  __|   |  _  /  |  __|    --
--   | |____  | |__| | | |__| |  _| |_  | |__| | | |__| |   | |____  | |__| | | |      | | \ \  | |____   --
--    \_____|  \____/  |_____/  |_____|  \_____|  \____/     \_____|  \____/  |_|      |_|  \_\ |______|  --
--                                                                                                        --                                                                                                
*/

//Área de inclusão de bibliotecas
//-----------------------------------------------------------------------------------------------------------------------

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ioplaca.h"  
#include "lcdvia595.h"
#include "driver/adc.h"
#include "hcf_adc.h"
#include "MP_hcf.h"  
#include "esp_err.h"
#include "nvs_flash.h"
#include "string.h"

// Área das macros
//-----------------------------------------------------------------------------------------------------------------------


#define TECLA_1 le_teclado() == '1'
#define TECLA_2 le_teclado() == '2'
#define TECLA_3 le_teclado() == '3'
#define TECLA_4 le_teclado() == '4'
#define TECLA_5 le_teclado() == '5'
#define TECLA_6 le_teclado() == '6'
#define TECLA_7 le_teclado() == '7'
#define TECLA_8 le_teclado() == '8'
#define TECLA_0 le_teclado() == '0'

#define STORAGE_NAMESPACE "storage"
#define SENHA_KEY "senha"

// Área de declaração de variáveis e protótipos de funções
//-----------------------------------------------------------------------------------------------------------------------

static const char *TAG = "Placa";
static uint8_t entradas, saidas = 0; //variáveis de controle de entradas e saídas

int ctrl = 0;
int n1 = 0;
int qdig = 0;
int coluna = 0;
int resul = 0;
char operador;
char tecla;
char mostra[40];
uint32_t adcvalor = 0;
int erros = 0;
int senha;
int adm2 = 0;
int adm3 = 0;
int adm4 = 0;
int interrompe = 14;
nvs_handle_t handle_algo;

// Funções e ramos auxiliares
//-----------------------------------------------------------------------------------------------------------------------

//OBS: CASO SEJA VISTO UMA DAS FUNÇÕES ABAIXO NO DECORRER DO CÓDIGO BASTAR APERTAR CTRL + BOTÃO DIREITO SOBRE A FUNÇÃO E VOCÊ SERÁ DIRECIONADO PARA CÁ

void setup() 
{
  // inicializa o NVS

  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  
  ESP_ERROR_CHECK(err);

  // abre o namespace de armazenamento

  nvs_handle_t handle_algo;
  err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle_algo);
  if (err != ESP_OK) 
  {
    lcd595_clear();
    lcd595_write(1,0, "erro ao abrir");
    lcd595_write(2,0, "namespace");
  }

  // lê a senha do NVS
  err = nvs_get_i32(handle_algo, SENHA_KEY, &senha);
  if (err != ESP_OK) 
  {
    senha = 1510; // Define a senha padrão como inteiro
  }
}

void ajuste ()
{
    // caso o cofre esta com a tampa aberta ou fechada demais antes inicar a execução do código principal será ajustado a posição ideal
    // são utilizados valor com uma leve distância para evitar que o movimento de um acione o outro implicando em um looping 

    hcf_adc_ler(&adcvalor);

    if(adcvalor > 300)
    {
        hcf_adc_ler(&adcvalor);
        while(adcvalor > 300)
        {
            hcf_adc_ler(&adcvalor);
            rotacionar_DRV(0, 11, saidas);
        }
    }

    if(adcvalor < 200)
    {
        hcf_adc_ler(&adcvalor);
        while(adcvalor < 200)
        {
            hcf_adc_ler(&adcvalor);
            rotacionar_DRV(1, 11, saidas);
        }
    }

}

void limpa ()
{
    if(tecla == 'C') // caso a tecla 'C' seja pressionada limpará o valor das principais variáveis para o funcionamento do código
    {
        n1 = 0;
        qdig = 0;
        ctrl = 0;
    }
}

void senhadisplay ()
{
    switch (qdig) 
    {
        case 0: // caso inicial da variável "qdig", quando ela é 0, exibindo no display todos espaços vazios
            lcd595_write(2, 0, "[ ] [ ] [ ] [ ]");
            break;
        case 1: // caso a variável "qdig" tenha valor 1, ou seja, somenta uma tecla foi pressionada, o display exibirá um espaço ocupado
            sprintf(&mostra[0], "[*] [ ] [ ] [ ]");
            lcd595_write(2, 0, &mostra[0]);
            break;
        case 2: // funciona de maneira análoga à de cima
            sprintf(&mostra[0], "[*] [*] [ ] [ ]");
            lcd595_write(2, 0, &mostra[0]);
            break;
        case 3: // funciona de maneira análoga à de cima
            sprintf(&mostra[0], "[*] [*] [*] [ ]");
            lcd595_write(2, 0, &mostra[0]);
            break;
        case 4: // funciona de maneira análoga à de cima
            sprintf(&mostra[0], "[*] [*] [*] [*]");
            lcd595_write(2, 0, &mostra[0]);
            vTaskDelay(250 / portTICK_PERIOD_MS);
            break;

        default: // em um caso de falha de código que a variável "qdig" assuma outro valor ela voltará a seu estado inicial
            qdig = 0;
            break;
    }
}

void abremotor()
{
    while(adcvalor <= 2400 && interrompe > 0) // enquanto valor do potenciômetor for menor que 2400 ou interrompe maior que 0, irá girar o motor de 11 em 11 graus
    {
        hcf_adc_ler(&adcvalor); // coleta o valor do potenciômetro sempre antes da execução

        lcd595_clear(); // limpa o display de antigas mensagens
        lcd595_write(1,0, "COFRE ABERTO!"); // informa pelo display "cofre aberto" tendo em vista que a senha está correta

        rotacionar_DRV(1, 11, saidas); // rotaciona o motor no sentido anti-horário de 11 em 11 graus através das saídas dos transistores
                
        interrompe--; // decresce de 1 em 1 o valor da variável interrompe para que o motor pare ao chegar a 0 caso o potenciômetro não chegue ao seu limite
    }
}

void fechamotor()
{
    while(adcvalor >= 450 && interrompe < 14) // enquanto o valor do potenciômetro for maior que 450 ou interrompe menor que 11, irá girar o motor de 11 em 11 graus
    {  
        hcf_adc_ler(&adcvalor); // coleta o valor do potenciômetro novamente para saber até quando se deve fechar

        lcd595_clear(); // limpa o display de antigas mensagens
        lcd595_write(1,0, "COFRE FECHANDO!"); // informa no display que o cofre está iniciando seu fechamento

        rotacionar_DRV(0, 11, saidas); // rotaciona o motor no sentido horário de 11 em 11 graus através das saídas dos transistores

        interrompe++; // aumenta de 1 em 1 o valor da variável interrompe para que o motor pare ao chegar a 11 caso o potenciômetro não chegue ao seu limite
    }
}

void espera()
{
    lcd595_clear(); // limpa o display de antigas mensagens
    lcd595_write(1,0, "MTS TENTATIVAS"); // informa no display que houveram muitas tentivas erradas
    lcd595_write(2,0, "AGUARDE 20s"); // informa no display que se deve aguardar 20 segundos
    vTaskDelay(20000 / portTICK_PERIOD_MS); // ativa uma pausa de 20s
    erros = 0; // zera a variável erros para que caso a pessoa erre novamente 3 vezes o mesmo aconteça
}

// Programa Principal
//-----------------------------------------------------------------------------------------------------------------------

void app_main(void)
{
    MP_init(); // configura pinos do motor
    // a seguir, apenas informações de console, aquelas notas verdes no início da execução
    ESP_LOGI(TAG, "Iniciando...");
    ESP_LOGI(TAG, "Versão do IDF: %s", esp_get_idf_version());

    /////////////////////////////////////////////////////////////////////////////////////   Inicializações de periféricos (manter assim)
    
    // inicializar os IOs e teclado da placa
    ioinit();      
    entradas = io_le_escreve(saidas); // Limpa as saídas e lê o estado das entradas

    // inicializar o display LCD 
    lcd595_init();
    lcd595_write(1,1,"    Jornada 1   ");
    lcd595_write(2,1," Programa Basico");
    
    // Inicializar o componente de leitura de entrada analógica
    esp_err_t init_result = hcf_adc_iniciar();
    if (init_result != ESP_OK) {
        ESP_LOGE("MAIN", "Erro ao inicializar o componente ADC personalizado");
    }

    // inica motor
    DRV_init(6, 7);

    //delay inicial
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
    lcd595_clear();

    /////////////////////////////////////////////////////////////////////////////////////   Periféricos inicializados
    
    lcd595_clear();
    lcd595_write(1,0, "© Gab Palazini"); // apresentação 
    lcd595_write(2,2, "CODIGO COFRE"); // " "

    vTaskDelay(1500 / portTICK_PERIOD_MS);

    lcd595_clear();
    ajuste();

    setup();
    
    /////////////////////////////////////////////////////////////////////////////////////   Início do ramo principal                    
    while(1)
    {
        hcf_adc_ler(&adcvalor); // coleta o valor do poteciômetro, que será necessário em varios momentos do código
    
        tecla = le_teclado(); // atribui à variável tecla o valor pressionado no teclado através da função le_teclado

        limpa();
       
        if(tecla == '+') // caso a tecla '+' seja pressionada iremos para o menu de administrador, onde será possível alterar a senha do cofre
        {
            ctrl = 1; // atribui o valor 1 para a variável ctrl para que a if abaixo se torne verdadeira e então entrar-se no menu de administrador
            lcd595_clear(); // limpa as informações exibidas no display
        }
            
        if(ctrl == 1)
        {
            tecla = le_teclado(); // atribui à variável tecla o valor pressionado no teclado através da função le_teclado

            limpa();

            lcd595_write(1,0, "ADMIN?"); // exibe no display a mensagem "admin?" ao entrar no menu de administrador

            if(tecla>='0' && tecla <='9') // condição para que somente números sejam aceitos na construção da variál n1, que será nossa senha
            {
                n1 = n1 * 10 + tecla - '0'; // faz que n1 se torne o valor anterior de n1 (inicialmente 0) vezes 10 mais a tecla pressionada no teclado
                qdig = qdig + 1; // essa variável irá auxiliar na exibição da senha no display conforme a quantida de teclas pressionadas (qdig = quantidade de digitos)
            }

            if (ctrl == 1) // verifica se ctrl é 1 antes de executar a função do switch
            { 
                senhadisplay();
            }

            if(qdig == 4) // quando qdig == 4, deste modo todas as casas foram preenchidas é analisado se a senha está correta
            {
                if(n1 == 9900) // caso a senha esteja correta será iniciado uma sequência de execuções que leverá ao painel de administrdor
                {
                    lcd595_clear(); // limpa as informações exibidas no display
                    adm2 = 1; // atribui o valor 1 para a variável auxiliar "adm2" que será útil futuramente
                } 
                else // casso a senha esteja errada será executado o código abaixo
                {
                    lcd595_clear(); // limpa as informações exibidas no display
                    lcd595_write(1,0, "ACESSO NEGADO"); // exibe no display a mensagem "acesso negado" visto que a senha está errada
                    
                    vTaskDelay(2000 / portTICK_PERIOD_MS); // delay de 2 segundos até voltar para a página inicial 
                    
                    lcd595_clear(); // limpa as informações exibidas no display
                    qdig = 0; // atribui o valor 0 para a variável "qdig" para retornarmos ao painel inicial 
                    n1 = 0; // atribui o valor 0 para a variável "n1" para retornarmos ao painel inicial 
                }
            }
        }

        if(adm2 == 1) // caso a senha esteja correta será executado o código abaixo
        {
            lcd595_write(1,0, "PAINEL ADM"); // exibe no display a mensagem "painel adm"
            lcd595_write(2,0, "PRESSIONE 1"); // exibe no display a mensagem "pressione 1"
            n1 = 0; // atribui o valor 0 para a variável "n1" pois iremos utilizá-la abaixo
            qdig = 0; // atribui o valor 0 para a variável "qdig" pois iremos utilizá-la abaixo

            while(tecla != '1' && adm3 != 1)
            {
                tecla = le_teclado(); // atribui à variável tecla o valor pressionado no teclado através da função le_teclado
                if(tecla == '1')
                {
                    adm3 = 1; // atribui o valor 1 para a variável "adm3" para se direcionar à próxima if]
                    adm2 = 0;
                }
            }
        }

        if(adm3 == 1)
        {
            adm4 = 1;
            vTaskDelay(500 / portTICK_PERIOD_MS);
            adm3 = 0;
            qdig = 0;
        }
        
        if(adm4 == 1 && qdig <4) // caso a tecla 1 seja pressionada será executado o código abaxio
        {
            tecla = le_teclado(); // atribui à variável tecla o valor pressionado no teclado através da função le_teclado
            lcd595_clear(); // limpa as informações exibidas no display
            lcd595_write(1,0, "NOVA SENHA"); // exibe no display a mensagem "nova senha" onde então o usuário poderá atribuir uma nova senha ao cofre 
            
            senhadisplay();

            while(qdig <4)
            {   
                tecla = le_teclado(); // atribui à variável tecla o valor pressionado no teclado através da função le_teclado

                senhadisplay();
                
                if(tecla>='0' && tecla <='9')
                {
                n1 = n1 * 10 + tecla - '0'; // faz que n1 se torne o valor anterior de n1 (inicialmente 0) vezes 10 mais a tecla pressionada no teclado
                qdig = qdig +1; // aumenta a variável "qdig" para saber a quantidade de digitos já registrados
                }
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
        }
            
        if(adm4 == 1 && qdig == 4)
        {   
            n1 = senha; // atribui à variável "senha" o valor digitado pelo usuário
            esp_err_t err = nvs_set_i32(handle_algo, SENHA_KEY, senha);
            
            if (err != ESP_OK) 
            {
                lcd595_clear();
                lcd595_write(1,0, "erro ao salvar");
                lcd595_write(2,0, "nova senha");
            }

            nvs_commit(handle_algo);

            vTaskDelay(1000 / portTICK_PERIOD_MS);

            lcd595_clear();
            lcd595_write(1,0, "senha salva");

            vTaskDelay(2500 / portTICK_PERIOD_MS); // delay de 1 segundo para assegurar que todos dados foram atualizados
                        
            qdig = 0; // atribui o valor 0 para a variável para retornarmos ao estado inicial do cofre
            n1 = 0; // " "
            adm2 = 0; // " "
            adm3 = 0; // " "
            adm4 = 0; // " "
            ctrl = 0; // " "

        }

        if(ctrl == 0) // enquanto a tecla '+' não for pressionada, a variáve ctrl será 0, então a princípio o código é executado aqui
        {
            lcd595_write(1,0, "Digite a senha!"); // escreve no display a frase "digite a senha"
        }

        if(tecla>='0' && tecla <='9') // condição para que somente números sejam aceitos na construção da variál n1, que será nossa senha
        {
          if(ctrl == 0) // redundânica da if ctrl == 0 pois a outro comando semelhante anteriormente quando ctrl == 1, evitando sobreposições
            {
                n1 = n1 * 10 + tecla - '0'; // faz que n1 se torne o valor anterior de n1 (inicialmente 0) vezes 10 mais a tecla pressionada no teclado
                qdig = qdig + 1; // essa variável irá auxiliar na exibição da senha no display conforme a quantida de teclas pressionadas (qdig = quantidade de digitos)
            }
        }

        if (ctrl == 0) // Verifica se ctrl é 0 antes de entrar no switch
        { 
            senhadisplay();
        }

        if(qdig == 4 && ctrl == 0) // quando qdig == 4, deste modo todas as casas foram preenchidas é analisado se a senha está correta
        {
            if(n1 == senha) // caso a senha não tenha sido trocada no painel ADM, no topo do código, ela terá seu valor padrão "1510"
            {
                hcf_adc_ler(&adcvalor); // coleta o valor do potenciômetro 
            
                abremotor();
                
                if(adcvalor == 2400 || interrompe == 0) // quando o valor do potênciometro for máximo ou interrompe chegar a 0, se inicia o processo de fechamento
                {   
                    lcd595_clear();
                    lcd595_write(1,2, "COFRE ABERTO");

                    for (int i = 15; i >= 0; i--) // esse ramo fará uma contagem regressiva de 15 segundos, tempo que o cofre ficará aberto
                    {
                        char contagem[17];
                        sprintf(contagem, "Fechando em %2d s", i); 
                        lcd595_write(2, 0, contagem);              
                        vTaskDelay(1000 / portTICK_PERIOD_MS);
                    }

                    hcf_adc_ler(&adcvalor); // coleta o valor do potenciômetro novamente para saber até quando se deve fechar

                    fechamotor();

                    qdig = 0; // após a conclusão de tudo a variável qdig volta ser 0 para que se possa iniciar novamente o processo
                    n1 = 0; // após a conclusão de tudo a variável n1 volta ser 0 para que se possa iniciar novamente o processo
                }

            }

            else // caso a pessoa erre a senha da if(n1 == senha) será executado o comando else
            {
                lcd595_clear(); // limpa o display de antigas mensagens
                lcd595_write(1,0, "SENHA ERRADA!"); // informa no display que a senha digitada está errada
              
                qdig = 0; // após isso a variável qdig volta ser 0 para que se possa iniciar novamente o processo
                n1 = 0; // após isso a variável n1 volta ser 0 para que se possa iniciar novamente o processo
                erros = erros + 1; // aumenta o valor da variável erros
            }

            if(erros == 3) // caso a variável erros alcance o valor 3 será executado um código de espera de segurança, evitando bruteforce
            {
                espera();
            }
        
        vTaskDelay(5000 / portTICK_PERIOD_MS); //delay para encerramento
        }
           
        vTaskDelay(100 / portTICK_PERIOD_MS); // delay para a while
       
    }     
        
    hcf_adc_limpar(); // caso erro no programa, desliga o módulo ADC
}
