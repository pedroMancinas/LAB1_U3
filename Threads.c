 // FileName:        Threads.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Activa las funciones del hilo HVAC_thread.
 //                  Main del proyecto.
 // Authors:         Jos� Luis Chac�n M. y Jes�s Alejandro Navarro Acosta.
 // Updated:         11/2018

#include "HVAC.h"

extern void *HVAC_Thread(void *arg0);   // Thread que arrancar� inicialmente.
int main(void){
    pthread_t           hvac_thread;
    pthread_attr_t      pAttrs;
    struct sched_param  priParam;
    int                 retc;
    int                 detachState;


   pthread_attr_init(&pAttrs);                                  /* Reinicio de par�metros. */

   detachState = PTHREAD_CREATE_DETACHED;                       // Los recursos se liberar�n despu�s del t�rmino del thread.
   retc = pthread_attr_setdetachstate(&pAttrs, detachState);    // Adem�s, al hilo no se le puede unir otro (join).
   if (retc != 0) { while (1); }

   /**********************
   ** Thread principal. **
   **********************/

   priParam.sched_priority = 1;                                     // Mayor prioridad a la tarea principal.
   retc |= pthread_attr_setstacksize(&pAttrs, THREADSTACKSIZE1);    // As� se determinar�a el tama�o del stack.
   if (retc != 0) { while (1); }
   pthread_attr_setschedparam(&pAttrs, &priParam);
   retc = pthread_create(&hvac_thread, &pAttrs, HVAC_Thread, NULL); // Creaci�n del thread.
   if (retc != 0) { while (1); }

   /* Arranque del sistema. */
   BIOS_start();

   return (0);
}
