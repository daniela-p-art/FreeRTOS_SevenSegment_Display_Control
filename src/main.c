#include "stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"

XGpio SwitchGpio, Bcd, Bcd_st, led;

//=======================================================================//
// VARIABILE GLOBALE
//=======================================================================//
static TaskHandle_t xProces1, xProces2, xProces3, xProces4;
static QueueHandle_t xQueue1 = NULL, xQueue2 = NULL, xQueue3 = NULL;

// Structuri pentru starea switch-urilor si LED-urilor
typedef struct {
    uint8_t switchState[5]; // starile celor 5 switch-uri
} SwitchStates;

typedef struct {
    uint8_t ledState[3]; // starea celor 3 LED-uri
} LDStates;

// Variabile pentru starile switch-urilor, LED-urilor si cifrele pentru afisajul 7 segmente
SwitchStates switchStates;
LDStates LD_States;
uint8_t digit1, digit2;

// Codificarea cifrelor pentru afisajele 7 segmente (common cathode)
const uint32_t segmentEncoding[10] = { 0x400, 0x790, 0x240, 0x300, 0x190, 0x120, 0x020, 0x780, 0x000, 0x100 };

//=======================================================================//
// PROTOTIPURI DE FUNCTII
//=======================================================================//
static void Proces1(void *pvParameters);
static void Proces2(void *pvParameters);
static void Proces3(void *pvParameters);
static void Proces4(void *pvParameters);
void init_perif(void);

int main(void) {
    init_perif(); // Initializeaza perifericele

    // Creeaza cele 4 procese
    xTaskCreate(Proces1, "Proces1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &xProces1);
    xTaskCreate(Proces2, "Proces2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &xProces2);
    xTaskCreate(Proces3, "Proces3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &xProces3);
    xTaskCreate(Proces4, "Proces4", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &xProces4);

    // Crearea si configurarea coadelor pentru comunicarea intre procese
    xQueue1 = xQueueCreate(1, sizeof(SwitchStates));
    xQueue2 = xQueueCreate(1, sizeof(LDStates));
    xQueue3 = xQueueCreate(1, sizeof(uint8_t));

    // Verifica daca coada a fost creata cu succes
    configASSERT(xQueue1);
    configASSERT(xQueue2);
    configASSERT(xQueue3);

    xil_printf("Pornire aplicatie temporizator pentru afisaje 7 segs\r\n");

    // Pornirea planificatorului FreeRTOS
    vTaskStartScheduler();

    // Daca ajunge aici, inseamna ca ceva nu a functionat corect
    for (;;) {
    }
}

//=======================================================================//
// PROCESELE
//=======================================================================//

// Procesul 1: Citeste starile switch-urilor si le trimite intr-o coada
static void Proces1(void *pvParameters) {
    xil_printf("--- Citesc valorile switch-urilor --- \r\n");
    SwitchStates switchStates_local;
    uint32_t switchRawState;

    while (1) {
        // Citirea valorilor switch-urilor de la GPIO
        switchRawState = XGpio_DiscreteRead(&SwitchGpio, 1);

        // Procesarea starilor fiecarui switch
        for (int i = 0; i < 5; i++) {
            switchStates_local.switchState[i] = (switchRawState >> i) & 0x1;
        }

        // Actualizarea LED-urilor in functie de starile switch-urilor
        for (int i = 2; i < 5; i++) {
            XGpio_DiscreteWrite(&led, XPAR_AXI_GPIO_1_DEVICE_ID, (switchStates_local.switchState[i] << i));
            vTaskDelay(pdMS_TO_TICKS(50)); // Asteapta 50ms
        }

        // Trimiterea starilor switch-urilor catre procesul 2 prin coada
        xQueueSend(xQueue1, &switchStates_local, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(50)); // Asteapta 50ms inainte de urmatoarea citire
    }
}

// Procesul 2: Citeste starea LED-urilor si o trimite intr-o coada
static void Proces2(void *pvParameters) {
    LDStates LD_States_local;
    uint8_t ledState;

    while (1) {
        // Citirea starii LED-urilor de la GPIO
        ledState = XGpio_DiscreteRead(&led, XPAR_AXI_GPIO_1_DEVICE_ID);

        // Procesarea starii fiecarui LED
        for (int i = 2; i < 5; i++) {
            LD_States_local.ledState[i-2] = (ledState & (1 << i)) ? 1 : 0;
        }

        // Trimiterea starii LED-urilor catre procesul 3 prin coada
        xQueueSend(xQueue2, &LD_States_local, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(50)); // Asteapta 50ms inainte de actualizare
    }
}

// Procesul 3: Trimite valorile BCD pe afisajul 7 segmente
static void Proces3(void *pvParameters) {
    xil_printf("--- Trimit valori BCD --- \r\n");
    SwitchStates switchStatesLocal;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(50));

        // Primeste starile switch-urilor de la Procesul 1
        xQueueReceive(xQueue1, &switchStatesLocal, portMAX_DELAY);

        // Selectia valorilor pentru afisajul 7 segmente (cifrele 1 si 2)
        digit1 = switchStatesLocal.switchState[0];
        digit2 = switchStatesLocal.switchState[1];

        // Scrierea valorilor BCD pe porturile GPIO pentru afisaje 7 segmente
        XGpio_DiscreteWrite(&Bcd, 1, segmentEncoding[digit1]);
        XGpio_DiscreteWrite(&Bcd_st, 1, segmentEncoding[digit2]);
    }
}

// Procesul 4: Afiseaza starile la consola seriala
static void Proces4(void *pvParameters) {
    SwitchStates switchStatesLocal;
    LDStates LD_States_local;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1500));

        xil_printf("Acest proces transmite informatii de la toate Procesele  \r\n");

        // Primeste starile switch-urilor si LED-urilor
        xQueueReceive(xQueue1, &switchStatesLocal, portMAX_DELAY);
        xQueueReceive(xQueue2, &LD_States_local, portMAX_DELAY);

        // Afiseaza starile switch-urilor pe consola seriala
        xil_printf("Starile intrerupatoarelor sunt:\r\n");
        for (int i = 0; i < 5; i++) {
            xil_printf("SW%d = %d\r\n", i, switchStatesLocal.switchState[i]);
        }

        // Afiseaza starile LED-urilor pe consola seriala
        xil_printf("Starile LED-urilor 2,3,4 sunt:\r\n");
        for (int i = 2; i < 5; i++) {
            xil_printf("LD%d = %d\r\n", i, switchStatesLocal.switchState[i]);
        }

        // Afiseaza valorile afisajului 7 segmente pe consola seriala
        xil_printf("Cifra de pe Display dreapta = %d\r\n", digit1);
        xil_printf("Cifra de pe Display stanga = %d\r\n", digit2);
    }
}

//=======================================================================//
// INITIALIZAREA PERIFERICELOR
//=======================================================================//

// Functie pentru initializarea perifericelor (GPIO pentru switch-uri, LED-uri si afisaje)
void init_perif() {
    extern XGpio Bcd, Bcd_st;

    // Initializarea si configurarea GPIO pentru afisaje 7 segmente
    XGpio_Initialize(&Bcd_st, XPAR_AXI_GPIO_4_DEVICE_ID);
    XGpio_Initialize(&Bcd, XPAR_AXI_GPIO_5_DEVICE_ID);
    XGpio_SetDataDirection(&Bcd, 1, 0x0000);
    XGpio_SetDataDirection(&Bcd_st, 1, 0x0000);
    XGpio_DiscreteWrite(&Bcd, 1, 0x00000000);
    XGpio_DiscreteWrite(&Bcd_st, 1, 0x00000000);

    // Initializarea GPIO pentru switch-uri
    int status = XGpio_Initialize(&SwitchGpio, XPAR_AXI_GPIO_0_DEVICE_ID);
    if (status != XST_SUCCESS) {
        xil_printf("Eroare la initializarea GPIO pentru switch-uri.\r\n");
        return;
    }
    XGpio_SetDataDirection(&SwitchGpio, 1, 0xFFFFFFFF);

    // Initializarea GPIO pentru LED-uri
    status = XGpio_Initialize(&led, XPAR_AXI_GPIO_1_DEVICE_ID);
    if (status != XST_SUCCESS) {
        xil_printf("Eroare la initializarea GPIO pentru LED\r\n");
        return XST_FAILURE;
    }

    XGpio_SetDataDirection(&led, XPAR_AXI_GPIO_1_DEVICE_ID, 0x0000);
}
