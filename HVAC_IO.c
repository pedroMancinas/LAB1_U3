//Laboratorio unidad 3

// FileName:        HVAC_IO.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Funciones de control de HW a través de estados y objetos.
 // Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 // Updated:         11/2018

#include "HVAC.h"

char p1[5]="DOWN",p2[5]="DOWN",LUCES[5]="OFF"; //arrays char para definir el estado de la persiana 1 y 2

int opcion_menu=0;
int cuenta_apagar=0;
int cuenta=0;
int banderau=0;
int LED1=0,LED2=0,LED3=0;

_mqx_int val = 0;
 _mqx_int val2 = 0;
 _mqx_int val3 = 0;

/* Definición de botones. */
#define BOTON_ENC   BSP_BUTTON1     /* Botones de suma y resta al valor deseado, funcionan con interrupciones. */
#define BOTON_MENU  BSP_BUTTON2

#define BOTON_UP    BSP_BUTTON3     /* Botones para identificación del estado del sistema. */
#define BOTON_DOWN  BSP_BUTTON4
#define SYSTEM_COOL BSP_BUTTON5
#define SYSTEM_OFF  BSP_BUTTON6
#define SYSTEM_HEAT BSP_BUTTON7

/* Definición de leds. */
#define LEDROJO     BSP_LED1        /* Leds para denotar el estado de las salidas. */
#define HEAT_LED    BSP_LED2
#define HBeat_LED   BSP_LED3
#define LEDAZUL     BSP_LED4


 boolean flag = 0;

  static boolean bandera_inicial = 0;

/* Variables sobre las cuales se maneja el sistema. */

float TemperaturaActual = 20;  // Temperatura.
float SetPoint = 25.0;         // V. Deseado.

char state[MAX_MSG_SIZE];      // Cadena a imprimir.

bool toggle = 0;               // Toggle para el heartbeat.
_mqx_int delay;                // Delay aplicado al heartbeat.
bool event = FALSE;            // Evento I/O que fuerza impresión inmediata.

bool FAN_LED_State = 0;                                     // Estado led_FAN.
const char* SysSTR[] = {"Cool","Off","Heat","Only Fan"};    // Control de los estados.

/* Archivos sobre los cuales se escribe toda la información */
FILE _PTR_ input_port = NULL, _PTR_ output_port = NULL;                  // Entradas y salidas.
FILE _PTR_ fd_adc = NULL, _PTR_ fd_ch_H0 = NULL,_PTR_ fd_ch_H1 = NULL,_PTR_ fd_ch_H3 = NULL;

FILE _PTR_ fd_uart = NULL;                                               // Comunicación serial asíncrona.

// Estructuras iniciales.

const ADC_INIT_STRUCT adc_init =
{
    ADC_RESOLUTION_DEFAULT,                                                     // Resolución.
    ADC_CLKDiv8                                                                 // División de reloj.
};


const ADC_INIT_CHANNEL_STRUCT adc_ch_param0 =
{
    AN0,                                                                         // Fuente de lectura, 'ANx'.
    ADC_CHANNEL_MEASURE_LOOP | ADC_CHANNEL_START_NOW | ADC_INTERNAL_TEMPERATURE, // Banderas de inicialización (temperatura)
    50000,                                                                       // Periodo en uS, base 1000.
    ADC_TRIGGER_1                                                                // Trigger lógico que puede activar este canal.
};

const ADC_INIT_CHANNEL_STRUCT adc_ch_param1 =
{
    AN5,                                                                         // Fuente de lectura, 'ANx'.
    ADC_CHANNEL_MEASURE_LOOP,                                                    // Banderas de inicialización (pot).
    50000,                                                                       // Periodo en uS, base 1000.
    ADC_TRIGGER_2                                                                // Trigger lógico que puede activar este canal.
};
const ADC_INIT_CHANNEL_STRUCT adc_ch_param3 =
{
    AN3,                                                                         // Fuente de lectura, 'ANx'.
    ADC_CHANNEL_MEASURE_LOOP,                                                    // Banderas de inicialización (pot).
    50000,                                                                       // Periodo en uS, base 1000.
    ADC_TRIGGER_3                                                                // Trigger lógico que puede activar este canal.
};

static uint_32 botones[] =                                                          // Formato de las entradas.
{                                                                                // Se prefirió un solo formato.
     BOTON_ENC,
     BOTON_MENU,
     BOTON_UP,
     BOTON_DOWN,
     SYSTEM_COOL,
     SYSTEM_OFF,
     SYSTEM_HEAT,

     GPIO_LIST_END
};

static const uint_32 fan[] =                                                    // Formato de los leds, uno por uno.
{
     LEDROJO,
     GPIO_LIST_END
};
static const uint_32 blue[] =                                                    // Formato de los leds, uno por uno.
{
     LEDAZUL,
     GPIO_LIST_END
};





/**********************************************************************************
 * Function: INT_SWI
 * Preconditions: Interrupción habilitada, registrada e inicialización de módulos.
 * Overview: Función que es llamada cuando se genera
 *           la interrupción del botón SW1 o SW2.
 * Input: None.
 * Output: None.
 **********************************************************************************/
void INT_SWI(void)
{
    Int_clear_gpio_flags(input_port);
    return;
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceIO
* Returned Value   : boolean; inicialización correcta.
* Comments         :
*    Abre los archivos e inicializa las configuraciones deseadas de entrada y salida GPIO.
*
*END***********************************************************************************/

boolean HVAC_InicialiceIO(void)
{
    // Estructuras iniciales de entradas y salidas.
    EstadoBotones.State_BTN_ENC=off;
    EstadoBotones.State_BTN_MENU=off;
    EstadoBotones.State_SL=off;
    EstadoBanderas.iniciar_opcion=off;





    const uint_32 output_set[] =
    {
         LEDROJO   | GPIO_PIN_STATUS_0,
         HEAT_LED  | GPIO_PIN_STATUS_0,
         HBeat_LED | GPIO_PIN_STATUS_0,
         LEDAZUL  | GPIO_PIN_STATUS_0,
         GPIO_LIST_END
    };

    const uint_32 input_set[] =
    {
        BOTON_ENC,
        BOTON_MENU,
        BOTON_UP,
        BOTON_DOWN,
        SYSTEM_COOL,
        SYSTEM_OFF,
        SYSTEM_HEAT,

        GPIO_LIST_END
    };

    // Iniciando GPIO.
    ////////////////////////////////////////////////////////////////////

    output_port =  fopen_f("gpio:write", (char_ptr) &output_set);
    input_port =   fopen_f("gpio:read", (char_ptr) &input_set);

    if (output_port) { ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, NULL); }   // Inicialmente salidas apagadas.
    ioctl (input_port, GPIO_IOCTL_SET_IRQ_FUNCTION, INT_SWI);               // Declarando interrupción.

    return (input_port != NULL) && (output_port != NULL);
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceADC
* Returned Value   : boolean; inicialización correcta.
* Comments         :
*    Abre los archivos e inicializa las configuraciones deseadas para
*    el módulo general ADC y dos de sus canales; uno para la temperatura, otro para
*    el heartbeat.
*
*END***********************************************************************************/
boolean HVAC_InicialiceADC (void)
{

       fd_adc   = fopen_f("adc:",  (const char*) &adc_init);               // Módulo.
       fd_ch_H0 = fopen_f  ("adc:0", (const char*) &adc_ch_param0);
       fd_ch_H1 = fopen_f("adc:1",(const char*) &adc_ch_param1);
       fd_ch_H3 = fopen_f("adc:3",(const char*) &adc_ch_param3);



        return (fd_adc != NULL) && (fd_ch_H0 != NULL) && (fd_ch_H1 != NULL) && (fd_ch_H3 != NULL); ;  // VALIDA QUE SE CREARON LOS ARCHIVOS.
}



/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceUART
* Returned Value   : boolean; inicialización correcta.
* Comments         :
*    Abre los archivos e inicializa las configuraciones deseadas para
*    configurar el modulo UART (comunicación asíncrona).
*
*END***********************************************************************************/
boolean HVAC_InicialiceUART (void)
{
    // Estructura inicial de comunicación serie.
    const UART_INIT_STRUCT uart_init =
    {
        /* Selected port */        1,
        /* Selected pins */        {1,2},
        /* Clk source */           SM_CLK,
        /* Baud rate */            BR_115200,

        /* Usa paridad */          NO_PARITY,
        /* Bits protocolo  */      EIGHT_BITS,
        /* Sobremuestreo */        OVERSAMPLE,
        /* Bits de stop */         ONE_STOP_BIT,
        /* Dirección TX */         LSB_FIRST,

        /* Int char's \b */        NO_INTERRUPTION,
        /* Int char's erróneos */  NO_INTERRUPTION
    };

    // Inicialización de archivo.
    fd_uart = fopen_f("uart:", (const char*) &uart_init);

    return (fd_uart != NULL); // Valida que se crearon los archivos.
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_ActualizarEntradas
* Returned Value   : None.
* Comments         :
*    Actualiza los variables indicadores de las entradas sobre las cuales surgirán
*    las salidas.
*
*END***********************************************************************************/
void HVAC_ActualizarEntradas(void)
{

    ioctl(input_port, GPIO_IOCTL_READ, &botones); //INICIALIZA LOS BOTONES

    if((botones[0] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS) /////////SI SE  PRESIONA  BTN_ENC P1.1//////////////////////
    {
        cuenta_apagar++;
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &fan);
        EstadoBotones.State_BTN_ENC=On; // BTN_ENC CAMBIA A ESTADO PRENDIDO


    }

    if((botones[1] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS)////SI SE PRESIONA BTN_MENU P1.4////////////////////
    {
        EstadoBotones.State_BTN_ENC=off; // BTN_ENC CAMBIA A ESTADO APAGADO
        EstadoBotones.State_BTN_MENU=On; //BTN_MENU CA,BIA A ESPADO PRENDIDO
        EstadoBanderas.iniciar_opcion=On;
        opcion_menu++;


        while(((botones[1] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS))// EVITAR CAMBIOS INESPERADOS P1.1
        {
            ioctl(input_port, GPIO_IOCTL_READ, &botones);
        }

    }

    if(((botones[0] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS)&&(cuenta_apagar==2)) /////PARA APAGAR SISTEMA///////////////
    {
        while(((botones[0] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS))// EVITAR CAMBIOS INESPERADOS P1.1
        {
            ioctl(input_port, GPIO_IOCTL_READ, &botones);
        }

        cuenta_apagar=1;
        print("PRESIONE DE NUEVO PARA APAGAR\n\r");

        while ((cuenta < 50)&&(EstadoBotones.State_BTN_ENC==On))// 5 ITERACIONES DE 1 SEGUNDO, 5 SEGUNDOS TOTALES
        {
           usleep(100000); // Espera 1 segundo
           ioctl(input_port, GPIO_IOCTL_READ, &botones); //INICIALIZA LOS BOTONES

           if((botones[0] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS)
           {
              cuenta_apagar=0;
              print("SISTEMA APAGADO!!!\n\r");
              ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &fan);//APAGA LED ROJO
              EstadoBotones.State_BTN_MENU=off; // BTN_ENC CAMBIA A ESTADO APAGADO
              EstadoBotones.State_BTN_ENC=off;

              while(((botones[0] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS))// EVITAR CAMBIOS INESPERADOS P1.1
              {
                  ioctl(input_port, GPIO_IOCTL_READ, &botones);
              }

         }
           cuenta++;
       }
        cuenta=0;
     }

}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_ActualizarSalidas
* Returned Value   : None.
* Comments         :
*    Decide a partir de las entradas actualizadas las salidas principales,
*    y en ciertos casos, en base a una cuestión de temperatura, la salida del 'fan'.
*
*END***********************************************************************************/
void HVAC_ActualizarSalidas(void)
{

}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_Heat
* Returned Value   : None.
* Comments         :
*    Decide a partir de la temperatura actual y la deseada, si se debe activar el fan.
*    (La temperatura deseada debe ser mayor a la actual). El estado del fan debe estar
*    en 'auto' y este modo debe estar activado para entrar a la función.
*
*END***********************************************************************************/
void HVAC_Heat(void)
{

}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_Cool
* Returned Value   : None.
* Comments         :
*    Decide a partir de la temperatura actual y la deseada, si se debe activar el fan.
*    (La temperatura deseada debe ser menor a la actual). El estado del fan debe estar
*    en 'auto' y este modo debe estar activado para entrar a la función.
*
*END***********************************************************************************/
void HVAC_Cool(void)
{

}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_Heartbeat
* Returned Value   : None.
* Comments         :
*    Función que prende y apaga una salida para notificar que el sistema está activo.
*    El periodo en que se hace esto depende de una entrada del ADC en esta función.
*
*END***********************************************************************************/
void HVAC_Heartbeat(void)               // Función de 'alive' del sistema.
{

         // ENTRADO POR PRIMERA VEZ, empieza a correr el canal de heartbeat.

    if(bandera_inicial == 0)
     {
         // ENTRADO POR PRIMERA VEZ, empieza a correr el canal de heartbeat.
         ioctl (fd_ch_H0, IOCTL_ADC_RUN_CHANNEL, NULL);
         ioctl (fd_ch_H1, IOCTL_ADC_RUN_CHANNEL, NULL);
         ioctl (fd_ch_H3, IOCTL_ADC_RUN_CHANNEL, NULL);
         bandera_inicial = 1;
     }

      flag =  (fd_adc && fread_f(fd_ch_H0, &val, sizeof(val))) ? 1 : 0;
      if(flag != TRUE)
      {
          printf("Error al leer archivo. Cierre del programa\r\n");
          exit(1);
      }

      flag =  (fd_adc && fread_f(fd_ch_H1, &val2, sizeof(val2))) ? 1 : 0;
         if(flag != TRUE)
         {
             printf("Error al leer archivo. Cierre del programa\r\n");
             exit(1);
         }
       flag =  (fd_adc && fread_f(fd_ch_H3, &val3, sizeof(val3))) ? 1 : 0;
                  if(flag != TRUE)
                  {
                      printf("Error al leer archivo. Cierre del programa\r\n");
                      exit(1);
                  }

       usleep(100000);

           if (val < 1638) {  //Obtencion en una escala de 0 a 10 de forma proporcional
               LED1= 0;
           } else if (val >= 1638 && val < 3276) {
               LED1= 1;
           } else if (val >= 3276 && val < 4914) {
               LED1 = 2;
           } else if (val >= 4914 && val < 6552) {
               LED1 = 3;
           } else if (val >= 6552 && val < 8190) {
               LED1 = 4;
           } else if (val >= 8190 && val < 9828) {
               LED1 = 5;
           } else if (val >= 9828 && val < 11466) {
               LED1 = 6;
           } else if (val >= 11466 && val < 13104) {
               LED1= 7;
           } else if (val >= 13104 && val < 14742) {
               LED1 = 8;
           } else if (val >= 14742 && val < 16380) {
               LED1 = 9;
           } else {
               LED1 = 10;
           }

           if (val2 < 1638) {  //Obtencion en una escala de 0 a 10 de forma proporcional
                   LED2= 0;
               } else if (val2 >= 1638 && val2 < 3276) {
                   LED2= 1;
               } else if (val >= 3276 && val2 < 4914) {
                   LED2 = 2;
               } else if (val2 >= 4914 && val2 < 6552) {
                   LED2 = 3;
               } else if (val2 >= 6552 && val2 < 8190) {
                   LED2 = 4;
               } else if (val2 >= 8190 && val2 < 9828) {
                   LED2 = 5;
               } else if (val2 >= 9828 && val2 < 11466) {
                   LED2 = 6;
               } else if (val2 >= 11466 && val2 < 13104) {
                   LED2= 7;
               } else if (val2 >= 13104 && val2 < 14742) {
                   LED2 = 8;
               } else if (val2 >= 14742 && val2 < 16380) {
                   LED2 = 9;
               } else {
                   LED2 = 10;
               }

           if (val3 < 1638) {  //Obtencion en una escala de 0 a 10 de forma proporcional
                   LED3= 0;
               } else if (val3 >= 1638 && val3 < 3276) {
                   LED3= 1;
               } else if (val3 >= 3276 && val3 < 4914) {
                   LED3 = 2;
               } else if (val3 >= 4914 && val3 < 6552) {
                   LED3 = 3;
               } else if (val3 >= 6552 && val3 < 8190) {
                   LED3 = 4;
               } else if (val3 >= 8190 && val3 < 9828) {
                   LED3 = 5;
               } else if (val3 >= 9828 && val3 < 11466) {
                   LED3 = 6;
               } else if (val3 >= 11466 && val3 < 13104) {
                   LED3= 7;
               } else if (val3 >= 13104 && val3 < 14742) {
                   LED3 = 8;
               } else if (val3 >= 14742 && val3 < 16380) {
                   LED3 = 9;
               } else {
                   LED3 = 10;
               }
       return;
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_PrintState
* Returned Value   : None.
* Comments         :
*    Imprime via UART la situación actual del sistema en términos de temperaturas
*    actual y deseada, estado del abanico, del sistema y estado de las entradas.
*    Imprime cada cierto número de iteraciones y justo despues de recibir un cambio
*    en las entradas, produciéndose un inicio en las iteraciones.
*END***********************************************************************************/
void HVAC_PrintState(void)
{

    if((EstadoBotones.State_BTN_ENC==On )&&(EstadoBotones.State_BTN_MENU==off))//AL PRENDER SISTEMA
    {
        EstadoBotones.State_BTN_MENU=On;
        print("SISTEMA ENCENDIDO!!!\n\r");
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &fan);//PRENDE LED ROJO
        sleep(1); //  ESPERA UN SEGUNDO
    }

    if(EstadoBotones.State_BTN_MENU==On) //ENTRA AL MENU UNA VEZ EL SISTEMA SE PRENDIO
    {

        switch (opcion_menu) {
                case 0: opcion_default(); break;
                case 1: persiana_1(); break;
                case 2: persiana_2(); break;
                case 3: secuencia_luces(); break;
                case 4: opcion_menu = 0; break;
                default: break;
            }

    }

}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_SetPointUp
* Returned Value   : None.
* Comments         :
*    Sube el valor deseado (set point). Llamado por interrupción a causa del SW1.
*
*END***********************************************************************************/
void HVAC_SetPointUp(void)
{

}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_SetPointDown
* Returned Value   : None.
* Comments         :
*    Baja el valor deseado (set point). Llamado por interrupción a causa del SW2.
*
*END***********************************************************************************/
void HVAC_SetPointDown(void)
{

}

void opcion_default(void)//funcion para imprimir el  menu
{
  ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &fan);//PRENDE LED ROJO4

  sprintf(state," LED_1= %d, ",LED1);
  print(state);

  sprintf(state," LED_2= %d, ",LED2);
  print(state);

  sprintf(state," LED_3= %d, ",LED3);
  print(state);

  sprintf(state," P1= %s", p1);
  print(state);

  sprintf(state," P2= %s", p2);
  print(state);//imprime el estado de la persiana 2

  sprintf(state,"  SL= %s\n\r", LUCES);
  print(state);
}

void persiana_1(void)
{   ioctl(input_port, GPIO_IOCTL_READ, &botones);
    ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &fan);//PRENDE LED ROJO
    if(EstadoBanderas.iniciar_opcion==On)
    {
        EstadoBanderas.iniciar_opcion=off;
        sprintf(state,"P1_SELECTED STATUS: %s\n\r", p1);
        print(state);
    }

    if((botones[2] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS)//SI SE PRESIONA BTN_UP P2.3
    {
        print("OPEN\n\r");
        sleep(5); //RETARDO 5 SEGUNDOS
        print("P1_SELECTED STATUS: UP\n\r");
        strcpy(p1, "UP");

        while(((botones[2] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS))// EVITAR CAMBIOS INESPERADOS P2.1
        {
             ioctl(input_port, GPIO_IOCTL_READ, &botones);
        }
    }

    if((botones[3] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS)//SI SE PRESIONA BTN_DOWN P2.4
      {
          print("CLOSE\n\r");
          sleep(5); //RETARDO 5 SEGUNDOS
          print("P1_SELECTED STATUS: DOWN\n\r");
          strcpy(p1, "DOWN");

          while(((botones[3] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS))// EVITAR CAMBIOS INESPERADOS P2.1
          {
               ioctl(input_port, GPIO_IOCTL_READ, &botones);
          }
      }
}

void persiana_2(void)
{    ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &fan);//PRENDE LED ROJO
    ioctl(input_port, GPIO_IOCTL_READ, &botones);
    if(EstadoBanderas.iniciar_opcion==On)
      {    EstadoBanderas.iniciar_opcion=off;
          sprintf(state,"P2_SELECTED STATUS: %s\n\r", p1);
          print(state);
      }

      if((botones[2] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS)//SI SE PRESIONA BTN_UP P2.3
      {
          print("OPEN\n\r");
          sleep(5); //RETARDO 5 SEGUNDOS
          print("P2_SELECTED STATUS: UP\n\r");
          strcpy(p2, "UP");

          while(((botones[2] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS))// EVITAR CAMBIOS INESPERADOS P2.1
          {
               ioctl(input_port, GPIO_IOCTL_READ, &botones);
          }
      }

      if((botones[3] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS)//SI SE PRESIONA BTN_DOWN P2.4
        {
            print("CLOSE\n\r");
            sleep(5); //RETARDO 5 SEGUNDOS
            print("P2_SELECTED STATUS: DOWN\n\r");
            strcpy(p2, "DOWN");

            while(((botones[3] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS))// EVITAR CAMBIOS INESPERADOS P2.1
            {
                 ioctl(input_port, GPIO_IOCTL_READ, &botones);
            }
        }
}


void secuencia_luces(void)
{    ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &fan);//PRENDE LED ROJO
    ioctl(input_port, GPIO_IOCTL_READ, &botones);

    if(EstadoBanderas.iniciar_opcion==On)
     {
         EstadoBanderas.iniciar_opcion=off;
         sprintf(state,"SL_SELECTED STATUS: %s\n\r", LUCES);
         print(state);
     }


    if((botones[2] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS)//SI SE PRESIONA BTN_UP P2.3
    {
        strcpy(LUCES, "ON");
        print("ON\n\r");
        EstadoBotones.State_SL=On;

       while((botones[2] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS)
       {
           ioctl(input_port, GPIO_IOCTL_READ, &botones);
       }
    }

    if((botones[3] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS)//SI SE PRESIONA BTN_DOWN P2.4
    {   strcpy(LUCES, "OFF");
        print("OFF\n\r");
        EstadoBotones.State_SL=off;
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &fan);  // APAGA ROJO
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &blue); // APAGA AZUL

       while((botones[3] & GPIO_PIN_STATUS)== NORMAL_STATE_EXTRA_BUTTONS)
       {
           ioctl(input_port, GPIO_IOCTL_READ, &botones);
       }
    }
}


void HVAC_LUCES(void)
{
    if (EstadoBotones.State_SL == On)
    {
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &fan);  // PRENDE ROJO
        usleep(50000);
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &fan);  // APAGA ROJO
        usleep(50000);
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &blue); // PRENDE AZUL
        usleep(50000);
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &blue); // APAGA AZUL
    }
    else
    {
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &fan);  // APAGA ROJO
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &blue); // APAGA AZUL
    }
}



