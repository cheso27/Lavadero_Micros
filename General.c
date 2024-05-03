#include "General.h"

// Variables globales
volatile uint8_t Stop = 0; // '1' Bloque maqueta
volatile uint32_t s = 0; // Cuenta segundos
volatile uint32_t ms = 0; // Cuenta milisegundos
volatile uint8_t numCar = 0; // Cuenta coches dentro del lavadero

volatile uint8_t EnableEntrance = 1; // '1' permite nuevas entradas de vehículos y '0' no permite
volatile uint8_t enable_prove_new = 0; // '1' pendiente comprobar entradas de vehículos 
volatile uint32_t cnt_prove_new = 0; // Cuenta comprobar entradas de vehículos 

	// Variables globales - Lavadero Horizontal
volatile uint8_t lav_H[3] = {0,0,0}; // lav_H[0] para SO3, lav_H[1] para SO4, lav_H[2] para SO5
volatile uint8_t prev_lav_H[3] = {0,0,0};
volatile uint8_t aux_lavH = 0;
volatile uint8_t limit_switch_lavH = 0; //'1' SW2 detecta rodillo abajo
	// Variables globales - Secado
volatile uint8_t secado[3] = {0,0,0}; // secado[0] para SO7, secado[1] para SO8, secado[2] para SO9
volatile uint8_t prev_secado[3] = {0,0,0};
volatile uint8_t aux_secado = 0;
volatile uint8_t limit_switch_secado = 0; //'1' SW3 detecta secador abajo

/* Puede ser interesante
volatile uint8_t estLavV = 0; // '1' activado Lavadero Vertical y '0' desactivado
volatile uint8_t estLavH = 0; // '1' activado Lavadero Vertical y '0' desactivado
volatile uint8_t estSec  = 0; // '1' activado Secadero y '0' desctivado
*/

//////////////////////////////

// Funciones de Stop

void setStop(void){
	Stop = 1;
}
	
uint8_t getStop(void){
	return Stop;
}

//////////////////////////////

// Funciones temporales

void setupTimers(void){
	cli();
	// TIMER 1 => Timer segundos : Modo CTC (ICRn) con preescalado 256
	TCCR1A = 0b00000000;
	TCCR1B = 0b00001101;
	TIMSK1 = 0b00000010;
	OCR1A =  uCFreq/256;
	// TIMER 3 => Timer milisegundos :  Modo CTC (ICRn) sin preescalado
	TCCR3A = 0b00000000;
	TCCR3B = 0b00001001;
	TIMSK3 = 0b00000010;
	OCR3A =	uCFreq/1000;
	sei();
}

void delay_milliseconds(uint32_t dms){
	volatile uint32_t delay = ms + dms;
	while( ms < delay ){}
}
void delay_seconds(uint32_t ds){
	volatile uint32_t delay = s + ds;
	while( s < delay ){}
}

uint32_t millis(void){
	return ms;
}

uint32_t seconds(void){
	return s;
}

////////////////////////////

// Funciones globales

void setup_General(void){ //Incluir todas funciones inicialización
	setupTimers();
}

uint8_t getNumCar(void){ // Devuelve número coches en lavadero
	return NumCar;
}

void incNumCar(void){ // Incrementa en uno el contador de coches en lavadero
	NumCar++;
}

void decNumCar(void){ // Decrementa en uno el contador de coches en lavadero
	NumCar--;
}

////////////////////////////

// Funciones de interrupción

ISR(TIMER1_COMPA_vect){ // Segundos
	s++;
	// CONTINUARÁ
}

ISR(TIMER3_COMPA_vect){ // Milisegundos
	ms++;
	// CONTINUARÁ
	if(ms % Check_height_sensors == 0){ //#define macro en General.h, tenemos que definir check_height_sensors en ms
		prev_lav_H = lav_H;
		//lav_H[0] = REG_S03_PIN & (1<<PIN_SO3_PIN); //REG_S03_PIN sustituir por etiqueta correcta
		//lav_H[1] = REG_S04_PIN & (1<<PIN_SO4_PIN); //REG_S04_PIN sustituir por etiqueta correcta
		//lav_H[2] = REG_S05_PIN & (1<<PIN_SO5_PIN); //REG_S05_PIN sustituir por etiqueta correcta
		lav_H[0] = isBitSet(REG_S03_PIN,PIN_SO3_PIN);
		lav_H[1] = isBitSet(REG_S04_PIN,PIN_SO4_PIN);
		lav_H[2] = isBitSet(REG_S05_PIN,PIN_SO5_PIN);
		limit_switch_lavH = isClrSet(REG_SW_PIN,PIN_SW2_PIN); // isClrSet porque SW2 '0' al detectar
		
		prev_secado = secado;
		lav_H[0] = isBitSet(REG_S07_PIN,PIN_SO7_PIN); //REG_S07_PIN sustituir por etiqueta correcta
		lav_H[1] = isBitSet(REG_S08_PIN,PIN_SO8_PIN); //REG_S08_PIN sustituir por etiqueta correcta
		lav_H[2] = isBitSet(REG_S09_PIN,PIN_SO9_PIN); //REG_S09_PIN sustituir por etiqueta correcta
		limit_switch_secado = isClrSet(REG_SW_PIN,PIN_SW3_PIN); // isClrSet porque SW3 '0' al detectar
		
		if(prev_lav_H == lav_H){ // Filtrado rebotes lavado
			aux_lavH = 1;
		}
		
		if(prev_secado == secado){ // Filtrado rebotes secado
			aux_secado = 1;
		}
	}
	
	if (enable_prove_new && (ms-cnt_prove_new > Tiempo_prove_new)){ //Comprueba si ha pasado cierto tiempo desde la última verificación
		enable_prove_new = 0; //Desactiva la bandera de verificación
	}
	
}

ISR(FLACO SUBIDA_SO1){ // Nuevo vehículo - Etapa 1
	if (BARRERA NO ABAJO){
		enable_prove_new = 1;
		cnt_prove_new = ms;
	}
}

ISR(FLACO BAJADA_SO3){ // Nuevo vehículo - Etapa 2
	if (enable_prove_new){
		incNumCar();
		enable_prove_new = 0;
	}
}
