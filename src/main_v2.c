#include "stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "event_groups.h"
/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"

// Declaratii periferice pentru gestionarea switch-urilor si afisajelor 7 segmente
XGpio SwitchGpio;
XGpio Bcd, Bcd_st; // Referinte pentru afisajele 7 segmente (Bcd si Bcd_st)

TickType_t x500ms;
TickType_t x050ms;

//=======================================================================//
// CONSTANTE
//=======================================================================//
#define TIMER_ID        1
#define DELAY_500_MS    500UL   // Timp de 500ms pentru temporizator
#define DELAY_050_MS    50UL    // Timp de 50ms pentru verificarea starii switch-urilor

#define EVENT_SWITCH_CHANGED (1 << 0)   // Eveniment care semnaleaza modificarea starii switch-urilor
#define EVENT_VALUE_UPDATED  (1 << 1)   // Eveniment care semnaleaza actualizarea valorii de afisat

//=======================================================================//
// VARIABILE
//=======================================================================//
static TimerHandle_t xTimer = NULL;   // Timer pentru actualizarea afisajului
static TaskHandle_t xTxTask;          // Task-ul pentru trimiterea datelor
static TaskHandle_t xRxTask;          // Task-ul pentru receptionarea datelor
static TaskHandle_t xProc1;           // Task-ul pentru logica switch-urilor (Proc1)
static QueueHandle_t xQueue = NULL;   // Coada pentru trimiterea si receptionarea valorilor intre task-uri
static EventGroupHandle_t xEventGroup = NULL;  // Grupul de evenimente pentru sincronizarea task-urilor

unsigned int txUnsignedInt;   // Valoare care va fi transmisa de catre TxTask
unsigned int dspUnsignedInt;  // Valoare care va fi afisata pe afisajele 7 segmente
unsigned int switchState;     // Starea switch-urilor

// Codificarea cifrelor pentru afisajele 7 segmente (common cathode)
const uint32_t segmentEncoding[10] = { 0x400, 0x790, 0x240, 0x300, 0x190, 0x120, 0x020, 0x780, 0x000, 0x100 };

//=======================================================================//
// PROTOTIPURI DE FUNCTII
//=======================================================================//
static void vTimerCallback(TimerHandle_t pxTimer);  // Functie callback pentru temporizator
static void prvProc1(void *pvParameters);           // Functia asociata procesului Proc1
static void prvTxTask(void *pvParameters);         // Functia pentru task-ul Tx
static void prvRxTask(void *pvParameters);         // Functia pentru task-ul Rx
void init_perif(void);  // Functia pentru initializarea perifericelor

/*-----------------------------------------------------------*/
int main(void) {
    init_perif(); // Initializarea perifericelor (GPIO-uri pentru switch-uri si afisaje)

    // Creare task pentru trimiterea valorii (TxTask)
    xTaskCreate(prvTxTask,                 // Functia care implementeaza procesul.
                 (const char *) "Tx",      // Numele task-ului
                 configMINIMAL_STACK_SIZE, // Memoria alocata pentru task
                 NULL,                     // Parametru suplimentar pentru task
                 tskIDLE_PRIORITY,         // Prioritate pentru task
                 &xTxTask);                // Referinta pentru task-ul creat

    // Creare task pentru receptionarea valorii (RxTask)
    xTaskCreate(prvRxTask,
                 (const char *) "Rx",
                 configMINIMAL_STACK_SIZE,
                 NULL,
                 tskIDLE_PRIORITY + 1,   // Prioritate mai mare decat TxTask
                 &xRxTask);

    // Creare task pentru logica switch-urilor (Proc1)
    xTaskCreate(prvProc1,
                 (const char *) "Proc1",
                 configMINIMAL_STACK_SIZE,
                 NULL,
                 tskIDLE_PRIORITY + 1,   // Prioritate mai mare decat TxTask
                 &xProc1);

    // Creare coada utilizata pentru trimiterea si receptionarea valorilor intre task-uri
    xQueue = xQueueCreate(1, sizeof(txUnsignedInt)); // Coada va contine un singur element

    // Verificare daca coada a fost creata cu succes
    configASSERT(xQueue);

    // Convertirea valorilor de timp in ticks (unitati de masura FreeRTOS)
    x500ms = pdMS_TO_TICKS(DELAY_500_MS);
    x050ms = pdMS_TO_TICKS(DELAY_050_MS);

    xil_printf("Pornire aplicatie temporizator pentru afisaje 7 segs\r\n");

    // Creare temporizator cu expirare de 500ms
    xTimer = xTimerCreate((const char *) "Timer1", x500ms, pdTRUE, (void *) TIMER_ID, vTimerCallback);

    // Verificare daca temporizatorul a fost creat cu succes
    configASSERT(xTimer);

    // Pornire temporizator
    xTimerStart(xTimer, 0);

    // Pornire planificator FreeRTOS
    vTaskStartScheduler();

    // Daca ajunge aici, inseamna ca schedulerul nu a inceput corect
    for (;;) {
    }
}

/*-----------------------------------------------------------*/
static void vTimerCallback(TimerHandle_t pxTimer) { // Functia callback pentru temporizatorul de 500ms
    configASSERT(pxTimer);
    // Seteaza evenimentul pentru actualizarea valorii afisajului
    xEventGroupSetBits(xEventGroup, EVENT_VALUE_UPDATED);
}

/*-----------------------------------------------------------*/
static void prvProc1(void *pvParameters) { // Functia pentru logica switch-urilor (Proc1)
	xil_printf("--- citesc valoarea switch-ului SW0 --- \r\n");
	unsigned int prevSwitchState = 0; // Starea anterioara a switch-ului

    while (1) {
        switchState = XGpio_DiscreteRead(&SwitchGpio, 1); // Citire starea switch-urilor
        // Verifica daca starea s-a schimbat
        if (switchState != prevSwitchState) {
            prevSwitchState = switchState;
            // Daca s-a schimbat, seteaza evenimentul de schimbare a switch-ului
            xEventGroupSetBits(xEventGroup, EVENT_SWITCH_CHANGED);
        }
        vTaskDelay(x050ms); // Intarziere de 50 ms pentru evitarea fluctuatiilor (debounce)
    }
}

/*-----------------------------------------------------------*/
static void prvTxTask(void *pvParameters) { // Functia pentru task-ul Tx (trimiterea datelor)
    for (;;) {
        vTaskDelay(x500ms);  // Asteapta 500ms intre trimiterea valorilor

        // Genereaza o valoare aleatorie intre 0 si 99
        txUnsignedInt = rand() % 100;

        // Trimite valoarea in coada
        xQueueSend(xQueue, &txUnsignedInt, 0UL);

        xil_printf("Update la 500ms\r\n"); // Mesaj pentru depanare
    }
}

/*-----------------------------------------------------------*/
static void prvRxTask(void *pvParameters) { // Functia pentru task-ul Rx (receptionarea datelor)
    extern XGpio Bcd, Bcd_st;
    unsigned int rxUnsignedInt;

    for (;;) {
    	// Asteapta evenimentul de actualizare a valorii
    	xEventGroupWaitBits(xEventGroup, EVENT_VALUE_UPDATED, pdTRUE, pdFALSE, portMAX_DELAY);

    	// Asteapta sa primeasca date din coada
        xQueueReceive(  xQueue,             /* Coada din care se citeste. */
                        &rxUnsignedInt,    /* Datele sunt citite in aceasta adresa. */
                        portMAX_DELAY );   /* Asteapta fara timp limita pentru date. */

        // Afisare valoare primita pentru depanare
        xil_printf( "RxTask a primit numarul de la TxTask: '%d'\r\n", rxUnsignedInt );
        dspUnsignedInt = rxUnsignedInt; // Actualizare contor afisaj

        // Calcularea cifrelor pentru afisaj
        int digit1 =  dspUnsignedInt % 10;       // Cifra unitatilor - BCD dreapta
        int digit2 = (dspUnsignedInt / 10) % 10; // Cifra zecilor - BCD stanga

        // Actualizare afisaje cu cifrele calculate
        XGpio_DiscreteWrite(&Bcd,    1, segmentEncoding[digit1] | (switchState >> 0 & 0xF));
        XGpio_DiscreteWrite(&Bcd_st, 1, segmentEncoding[digit2] | (switchState >> 4 & 0xF));
        }
    }

/*-----------------------------------------------------------*/
// Functia pentru initializarea perifericelor
void init_perif() {
    extern XGpio Bcd, Bcd_st;

    // Initializare pentru afisajele 7 segmente
    XGpio_Initialize(&Bcd_st, XPAR_AXI_GPIO_4_DEVICE_ID);
    XGpio_Initialize(&Bcd, XPAR_AXI_GPIO_5_DEVICE_ID);
    XGpio_SetDataDirection(&Bcd, 1, 0x0000);  // Seteaza directia de iesire pentru Bcd
    XGpio_SetDataDirection(&Bcd_st, 1, 0x0000); // Seteaza directia de iesire pentru Bcd_st
    XGpio_DiscreteWrite(&Bcd, 1, 0x00000000);  // Initializeaza afisajele cu 0
    XGpio_DiscreteWrite(&Bcd_st, 1, 0x00000000);

    // Initializare GPIO pentru switch-uri
    int status = XGpio_Initialize(&SwitchGpio, XPAR_AXI_GPIO_0_DEVICE_ID);
    if (status != XST_SUCCESS) {
        xil_printf("Eroare la initializarea GPIO pentru switch-uri.\r\n");
        return;
    }
    XGpio_SetDataDirection(&SwitchGpio, 1, 0xFFFFFFFF);  // Seteaza toti pinul ca intrare

    // Creare grup de evenimente
    xEventGroup = xEventGroupCreate();
    configASSERT(xEventGroup); // Asigura-te ca grupul de evenimente a fost creat cu succes
}
