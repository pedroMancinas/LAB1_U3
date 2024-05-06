 // FileName:        HVAC.h
 // Dependencies:    None
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Incluye librerías, define ciertas macros y significados así como llevar un control de versiones.
 // Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 // Updated:         11/2018

#ifndef _hvac_h_
#define _hvac_h_

#pragma once

#define __MSP432P401R__
#define  __SYSTEM_CLOCK    48000000 // Frecuencias funcionales recomendadas: 12, 24 y 48 Mhz.

/* Archivos de cabecera importantes. */
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Archivos de cabecera POSIX. */
#include <pthread.h>
#include <semaphore.h>
#include <ti/posix/tirtos/_pthread.h>
#include <ti/sysbios/knl/Task.h>

/* Archivos de cabecera para RTOS. */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Event.h>

/* Archivos de cabecera de drivers de Objetos. */
#include "Drivers_obj/BSP.h"

// Definiciones del estado 'normal' de los botones externos a la tarjeta (solo hay dos botones).
#define GND 0
#define VCC 1
#define NORMAL_STATE_EXTRA_BUTTONS GND  // Aqui se coloca GND o VCC.

// Definiciones del sistema.
#define MAX_MSG_SIZE 64
#define MAX_ADC_VALUE 16383
#define MAIN_UART (uint32_t)(EUSCI_A0)
#define DELAY 400
#define ITERATIONS_TO_PRINT 49

// Definición del RTOS.
#define THREADSTACKSIZE1 1500

/* Enumeradores para la descripción del sistema. */

/*enum FAN        // Para el fan (abanico).
{
    On,
    Auto,
};

enum SYSTEM     // Para el sistema cuando FAN está en auto (cool, off y heat, o no considerar ninguno y usar fan only).
{
    Cool,
    Off,
    Heat,
    FanOnly,
};

struct EstadoEntradas       // Estructura que incluye el estado de las entradas.
{
    uint8_t  SystemState;
    uint8_t     FanState;
} EstadoEntradas;*/

//Propuestas/////////////////////////////////////////////////////////////////////////
enum ESTADO        // Para estados de los botones
{
    On,
    off,
};
enum ESTADO_PERSIANA
{
    DOWN,
    UP,
};

struct EstadoEntradas       // Estructura que incluye el estado de las entradas.
{
    uint8_t     State_BTN_ENC;
    uint8_t     State_BTN_MENU;
    uint8_t     State_SL;

} EstadoBotones;

struct BANDERAS      // Estructura que incluye el estado de las entradas.
{
    uint8_t    iniciar_opcion;


} EstadoBanderas;




////////////////////////////////////////////////////////////////////////////////////


/* Funciones. */

/* Función de interrupción para botones de setpoint. */
extern void INT_SWI(void);

/* Funciones de inicialización. */
extern boolean HVAC_InicialiceIO   (void);
extern boolean HVAC_InicialiceADC  (void);
extern boolean HVAC_InicialiceUART (void);

/* Funciones principales. */
extern void HVAC_ActualizarEntradas(void);
extern void HVAC_ActualizarSalidas(void);
extern void HVAC_Heartbeat(void);
extern void HVAC_PrintState(void);

// Funciones para los estados Heat y Cool.
extern void HVAC_Heat(void);
extern void HVAC_Cool(void);

// Funciones para incrementar o disminuir setpoint.
extern void HVAC_SetPointUp(void);
extern void HVAC_SetPointDown(void);

//Funciones agregadas
extern void opcion_default(void);
extern void persiana_1(void);
extern void persiana_2(void);
extern void secuencia_luces(void);
extern void HVAC_LUCES(void);

/* Función especial que imprime el mensaje asegurando que no habrá interrupciones y por ende,
 * un funcionamiento no óptimo.                                                             */
extern void print(char* message);

#endif
