/************************************************************************
* 5 semestre - Eng. da Computao - Insper
*
* 2021 - Exemplo com HC05 com RTOS
*
*/

#include <asf.h>
#include "conf_board.h"
#include <string.h>

/************************************************************************/
/* defines                                                              */
/************************************************************************/

// LEDs
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

#define LED1_PIO      PIOA
#define LED1_PIO_ID   ID_PIOA
#define LED1_IDX      4
#define LED1_IDX_MASK (1 << LED1_IDX)


// Botão
#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX      11
#define BUT_IDX_MASK (1 << BUT_IDX)

// Botão
#define PIN1_PIO      PIOA
#define PIN1_PIO_ID   ID_PIOA
#define PIN1_IDX      19
#define PIN1_IDX_MASK (1 << PIN1_IDX)

// Botão
#define PIN2_PIO      PIOB
#define PIN2_PIO_ID   ID_PIOB
#define PIN2_IDX      2
#define PIN2_IDX_MASK (1 << PIN2_IDX)

// Botão
#define PIN3_PIO      PIOC
#define PIN3_PIO_ID   ID_PIOC
#define PIN3_IDX      30
#define PIN3_IDX_MASK (1 << PIN3_IDX)

// Botão
#define PIN4_PIO      PIOC
#define PIN4_PIO_ID   ID_PIOC
#define PIN4_IDX      17
#define PIN4_IDX_MASK (1 << PIN4_IDX)

#define AFEC_POT AFEC0
#define AFEC_POT_ID ID_AFEC0
#define AFEC_POT_CHANNEL 0 // Canal do pino PD30

// usart (bluetooth ou serial)
// Descomente para enviar dados
// pela serial debug

// #define DEBUG_SERIAL

#ifdef DEBUG_SERIAL
#define USART_COM USART1
#define USART_COM_ID ID_USART1
#else
#define USART_COM USART0
#define USART_COM_ID ID_USART0
#endif

/************************************************************************/
/* RTOS                                                                 */
/************************************************************************/

TimerHandle_t xTimer;

#define TASK_BLUETOOTH_STACK_SIZE            (4096/sizeof(portSTACK_TYPE))
#define TASK_BLUETOOTH_STACK_PRIORITY        (tskIDLE_PRIORITY)
#define TASK_LED_STACK_SIZE            (4096/sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY        (tskIDLE_PRIORITY)
#define TASK_ADC_STACK_SIZE (1024 * 10 / sizeof(portSTACK_TYPE))
#define TASK_ADC_STACK_PRIORITY (tskIDLE_PRIORITY)

QueueHandle_t xQueueVolume;
QueueHandle_t xQueueADC;

typedef struct {
	uint value;
} adcData;


/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

static void config_AFEC_pot(Afec *afec, uint32_t afec_id, uint32_t afec_channel,
                            afec_callback_t callback);

/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/
volatile char but_flag = 0;
volatile char but2_flag = 0;
volatile char but3_flag = 0;
volatile char but4_flag = 0;
/************************************************************************/
/* RTOS application HOOK                                                */
/************************************************************************/
void pisca_led(int n, int t);
void io_init(void);
void pin_toggle(Pio *pio, uint32_t mask);


void but_callback(void)
{
	but_flag = 1;
}


void pisca_led(int n, int t){
	int j=70;
	for (int i=0;i<n;i++){
		pio_clear(LED_PIO, LED_IDX_MASK);
		delay_ms(t);
		pio_set(LED_PIO, LED_IDX_MASK);
		delay_ms(t);
	}
}

void pin_toggle(Pio *pio, uint32_t mask) {
	if(pio_get_output_data_status(pio, mask))
		pio_clear(pio, mask);
	else
		pio_set(pio,mask);
}

/* Called if stack overflow during execution */
extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	* identify which task has overflowed its stack.
	*/
	for (;;) {
	}
}

/* This function is called by FreeRTOS idle task */
extern void vApplicationIdleHook(void) {
	pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
}

/* This function is called by FreeRTOS each tick */
extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

	/* Force an assert. */
	configASSERT( ( volatile void * ) NULL );
}

/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/
int valorAntigo = 0;
static void AFEC_pot_callback(void) {
  adcData adc;
  adc.value = afec_channel_get_value(AFEC_POT, AFEC_POT_CHANNEL);
  char valor = adc.value*65/4096;
  BaseType_t xHigherPriorityTaskWoken = pdTRUE;
  if (valor != valorAntigo){
	valorAntigo = valor;
	xQueueSendFromISR(xQueueVolume, &valor, xHigherPriorityTaskWoken);
  }
}

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

static void config_AFEC_pot(Afec *afec, uint32_t afec_id, uint32_t afec_channel,
                            afec_callback_t callback) {
  /*************************************
   * Ativa e configura AFEC
   *************************************/
  /* Ativa AFEC - 0 */
  afec_enable(afec);

  /* struct de configuracao do AFEC */
  struct afec_config afec_cfg;

  /* Carrega parametros padrao */
  afec_get_config_defaults(&afec_cfg);

  /* Configura AFEC */
  afec_init(afec, &afec_cfg);

  /* Configura trigger por software */
  afec_set_trigger(afec, AFEC_TRIG_SW);

  /*** Configuracao específica do canal AFEC ***/
  struct afec_ch_config afec_ch_cfg;
  afec_ch_get_config_defaults(&afec_ch_cfg);
  afec_ch_cfg.gain = AFEC_GAINVALUE_0;
  afec_ch_set_config(afec, afec_channel, &afec_ch_cfg);

  /*
  * Calibracao:
  * Because the internal ADC offset is 0x200, it should cancel it and shift
  down to 0.
  */
  afec_channel_set_analog_offset(afec, afec_channel, 0x200);

  /***  Configura sensor de temperatura ***/
  struct afec_temp_sensor_config afec_temp_sensor_cfg;

  afec_temp_sensor_get_config_defaults(&afec_temp_sensor_cfg);
  afec_temp_sensor_set_config(afec, &afec_temp_sensor_cfg);

  /* configura IRQ */
  afec_set_callback(afec, afec_channel, callback, 5);
  NVIC_SetPriority(afec_id, 5);
  NVIC_EnableIRQ(afec_id);
}

void io_init(void) {

	// Ativa PIOs
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(LED1_PIO_ID);
	pmc_enable_periph_clk(BUT_PIO_ID);
	pmc_enable_periph_clk(PIN1_PIO_ID);
	pmc_enable_periph_clk(PIN2_PIO_ID);
	pmc_enable_periph_clk(PIN3_PIO_ID);
	pmc_enable_periph_clk(PIN4_PIO_ID);

	// Configura Pinos
	pio_configure(LED_PIO, PIO_OUTPUT_1, LED_IDX_MASK, PIO_DEFAULT | PIO_DEBOUNCE);
	pio_configure(LED1_PIO, PIO_OUTPUT_1, LED1_IDX_MASK, PIO_DEFAULT | PIO_DEBOUNCE);
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP);
	pio_configure(PIN1_PIO, PIO_INPUT, PIN1_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_configure(PIN2_PIO, PIO_INPUT, PIN2_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_configure(PIN3_PIO, PIO_INPUT, PIN3_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_configure(PIN4_PIO, PIO_INPUT, PIN4_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	
	// Configura PIO para lidar com o pino do bot�o como entrada
	// com pull-up
	pio_set_debounce_filter(BUT_PIO, BUT_IDX_MASK, 10);
	
	pio_handler_set(PIN1_PIO,
	PIN1_PIO_ID,
	PIN1_IDX_MASK,
	PIO_IT_FALL_EDGE,
	but_callback);
	
	pio_handler_set(PIN2_PIO,
	PIN2_PIO_ID,
	PIN2_IDX_MASK,
	PIO_IT_FALL_EDGE,
	but_callback);
	
	pio_handler_set(PIN3_PIO,
	PIN3_PIO_ID,
	PIN3_IDX_MASK,
	PIO_IT_FALL_EDGE,
	but_callback);
	
	pio_handler_set(PIN4_PIO,
	PIN4_PIO_ID,
	PIN4_IDX_MASK,
	PIO_IT_FALL_EDGE,
	but_callback);
	
	// Ativa interrup��o e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(PIN1_PIO, PIN1_IDX_MASK);
	pio_enable_interrupt(PIN2_PIO, PIN2_IDX_MASK);
	pio_enable_interrupt(PIN3_PIO, PIN3_IDX_MASK);
	pio_enable_interrupt(PIN4_PIO, PIN4_IDX_MASK);
	pio_get_interrupt_status(PIN1_PIO);
	pio_get_interrupt_status(PIN2_PIO);
	pio_get_interrupt_status(PIN3_PIO);
	pio_get_interrupt_status(PIN4_PIO);
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais pr�ximo de 0 maior)
	NVIC_EnableIRQ(PIN1_PIO_ID);
	NVIC_EnableIRQ(PIN2_PIO_ID);
	NVIC_EnableIRQ(PIN3_PIO_ID);
	NVIC_EnableIRQ(PIN4_PIO_ID);
	NVIC_SetPriority(PIN1_PIO_ID, 4); // Prioridade 4
	NVIC_SetPriority(PIN2_PIO_ID, 4); // Prioridade 4
	NVIC_SetPriority(PIN3_PIO_ID, 4); // Prioridade 4
	NVIC_SetPriority(PIN4_PIO_ID, 4); // Prioridade 4
}

static void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		#if (defined CONF_UART_CHAR_LENGTH)
		.charlength = CONF_UART_CHAR_LENGTH,
		#endif
		.paritytype = CONF_UART_PARITY,
		#if (defined CONF_UART_STOP_BITS)
		.stopbits = CONF_UART_STOP_BITS,
		#endif
	};

	/* Configure console UART. */
	stdio_serial_init(CONF_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
	#if defined(__GNUC__)
	setbuf(stdout, NULL);
	#else
	/* Already the case in IAR's Normal DLIB default configuration: printf()
	* emits one character at a time.
	*/
	#endif
}

uint32_t usart_puts(uint8_t *pstring) {
	uint32_t i ;

	while(*(pstring + i))
	if(uart_is_tx_empty(USART_COM))
	usart_serial_putchar(USART_COM, *(pstring+i++));
}

void usart_put_string(Usart *usart, char str[]) {
	usart_serial_write_packet(usart, str, strlen(str));
}

int usart_get_string(Usart *usart, char buffer[], int bufferlen, uint timeout_ms) {
	uint timecounter = timeout_ms;
	uint32_t rx;
	uint32_t counter = 0;

	while( (timecounter > 0) && (counter < bufferlen - 1)) {
		if(usart_read(usart, &rx) == 0) {
			buffer[counter++] = rx;
		}
		else{
			timecounter--;
			vTaskDelay(1);
		}
	}
	buffer[counter] = 0x00;
	return counter;
}

void usart_send_command(Usart *usart, char buffer_rx[], int bufferlen,
char buffer_tx[], int timeout) {
	usart_put_string(usart, buffer_tx);
	usart_get_string(usart, buffer_rx, bufferlen, timeout);
}

void config_usart0(void) {
	sysclk_enable_peripheral_clock(ID_USART0);
	usart_serial_options_t config;
	config.baudrate = 9600;
	config.charlength = US_MR_CHRL_8_BIT;
	config.paritytype = US_MR_PAR_NO;
	config.stopbits = false;
	usart_serial_init(USART0, &config);
	usart_enable_tx(USART0);
	usart_enable_rx(USART0);

	// RX - PB0  TX - PB1
	pio_configure(PIOB, PIO_PERIPH_C, (1 << 0), PIO_DEFAULT);
	pio_configure(PIOB, PIO_PERIPH_C, (1 << 1), PIO_DEFAULT);
}

int hc05_init(void) {
	char buffer_rx[128];
	usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT+NAMEmengo", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
	vTaskDelay( 500 / portTICK_PERIOD_MS);
	usart_send_command(USART_COM, buffer_rx, 1000, "AT+PIN0000", 100);
}

void vTimerCallback(TimerHandle_t xTimer) {
	/* Selecina canal e inicializa conversão */
	afec_channel_enable(AFEC_POT, AFEC_POT_CHANNEL);
	afec_start_software_conversion(AFEC_POT);
}

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/
static void task_adc(void *pvParameters) {

  // configura ADC e TC para controlar a leitura
  config_AFEC_pot(AFEC_POT, AFEC_POT_ID, AFEC_POT_CHANNEL, AFEC_pot_callback);

  int x = 0;
  afec_disable_interrupt(AFEC_POT,AFEC_INTERRUPT_EOC_5);
  for (;;)  {
	  if (x >=40){
		  afec_channel_enable(AFEC_POT, AFEC_POT_CHANNEL);
		  afec_start_software_conversion(AFEC_POT);
		  x = 0;
	  }
	  x++;
	  vTaskDelay(50);
  }
}

void task_leds(void) {
	io_init();
	
	while(1) {
		if (but_flag) {
			pisca_led(5, 100);
			but_flag = 0;
		}
		
		// dorme por 500 ms
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

void task_bluetooth(void) {
	printf("Task Bluetooth started \n");
	
	printf("Inicializando HC05 \n");
	config_usart0();
	hc05_init();

	// configura LEDs e Botões
	io_init();

	char button1 = '0';
	char eof = 'X';
	int pio = 0;
	char buttonAntigo = '0';

	// Task não deve retornar.
	while(1) {
		// atualiza valor do botão
		if(pio_get(PIN1_PIO, PIO_INPUT, PIN1_IDX_MASK) == 0) {
			button1 = '1';
		} else if (pio_get(PIN2_PIO, PIO_INPUT, PIN2_IDX_MASK) == 0) {
			button1 = '2';
			pin_toggle(LED1_PIO, LED1_IDX_MASK);
		}
		else if (pio_get(PIN3_PIO, PIO_INPUT, PIN3_IDX_MASK) == 0) {
			button1 = '3';
		}
		else if (pio_get(PIN4_PIO, PIO_INPUT, PIN4_IDX_MASK) == 0) {
			button1 = '4';
		} else {
			button1 = 'c';
		}

		char valor;
		if (xQueueReceive(xQueueVolume, &valor, 0)) {
			button1 = valor;
		}

		// envia status botão
		while(!usart_is_tx_ready(USART_COM)) {
			vTaskDelay(10 / portTICK_PERIOD_MS);
		}
		
		
		if (buttonAntigo != button1) {
			buttonAntigo = button1;
			
			printf("%c \n", button1);
			usart_write(USART_COM, button1);
			
			// envia fim de pacote
			while(!usart_is_tx_ready(USART_COM)) {
				vTaskDelay(10 / portTICK_PERIOD_MS);
			}
			usart_write(USART_COM, eof);
		}
		// dorme por 500 ms
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/

int main(void) {
	/* Initialize the SAM system */
	sysclk_init();
	board_init();
	delay_init();
	io_init();

	configure_console();
	xQueueADC = xQueueCreate(100, sizeof(adcData));
	if (xQueueADC == NULL)
		printf("falha em criar a queue xQueueADC \n");
	xQueueVolume = xQueueCreate(32, sizeof(char));
	if (xQueueVolume == NULL)
		printf("falha em criar a queue xQueueVolume \n");

	/* Create task to make led blink */
	xTaskCreate(task_bluetooth, "BLT", TASK_BLUETOOTH_STACK_SIZE, NULL,	TASK_BLUETOOTH_STACK_PRIORITY, NULL);
	xTaskCreate(task_leds, "BLT", TASK_LED_STACK_SIZE, NULL,	TASK_LED_STACK_PRIORITY, NULL);
	 if (xTaskCreate(task_adc, "ADC", TASK_ADC_STACK_SIZE, NULL,
	 TASK_ADC_STACK_PRIORITY, NULL) != pdPASS) {
		 printf("Failed to create test ADC task\r\n");
	 }

	/* Start the scheduler. */
	vTaskStartScheduler();

	while(1){}

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}
