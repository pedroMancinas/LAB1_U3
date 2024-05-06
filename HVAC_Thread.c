 // FileName:        HVAC_Thread.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Definici�n de las funciones de los threads de HVAC_Thread().
 // Authors:         Jos� Luis Chac�n M. y Jes�s Alejandro Navarro Acosta.
 // Updated:         11/2018

#include "HVAC.h"                           // Incluye definici�n del sistema.


/*********************************THREAD******************************************
 * Function: HVAC_Thread
 * Preconditions: None.
 * Overview: Realiza la lectura de la temperatura y controla salidas actualizando
 *           a su vez entradas. Imprime estados. Tambi�n contiene el heartbeat.
 * Input:  Apuntador vac�o que puede apuntar cualquier tipo de dato.
 * Output: None.
 *
 ********************************************************************************/
void *HVAC_Thread(void *arg0)
{
   bool flag = TRUE;

   SystemInit();
   flag &= HVAC_InicialiceIO();
   flag &= HVAC_InicialiceADC();
   flag &= HVAC_InicialiceUART();

   if(flag != TRUE)
   {
       printf("Error al crear archivo. Cierre del programa.\r\n");
       exit(1);
   }

   printf("Presione BOTON_ENC para encender el sistema.\r\n");

   while(TRUE)
   {
       HVAC_ActualizarEntradas();
       HVAC_LUCES();
       HVAC_ActualizarSalidas();
       HVAC_PrintState();
       HVAC_Heartbeat();
   }
}
