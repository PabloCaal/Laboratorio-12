/* 
 * File:   prelab.c
 * Author: Pablo Caal
 *
 * Created on 18 de mayo de 2022, 10:13 AM
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT    // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF               // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF              // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF              // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF                 // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF                // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF              // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF               // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF              // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF                // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V           // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF                // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>

/*------------------------------------------------------------------------------
 * CONSTANTES 
 ------------------------------------------------------------------------------*/
#define _XTAL_FREQ 4000000      // Frecuencia de oscilador en 4 MHz

/*------------------------------------------------------------------------------
 * VARIABLES 
 ------------------------------------------------------------------------------*/
int MODO_SLEEP = 0;               // Variable de contador que envía el maestro al esclavo

/*------------------------------------------------------------------------------
 * PROTOTIPO DE FUNCIONES 
 ------------------------------------------------------------------------------*/
void setup(void);

/*------------------------------------------------------------------------------
 * INTERRUPCIONES 
 ------------------------------------------------------------------------------*/
void __interrupt() isr (void){    
    if(PIR1bits.ADIF){                  // Verificación de interrupción del módulo ADC
        if(ADCON0bits.CHS == 0){        // Verificación de canal AN0
            PORTC = ADRESH;
        }
        PIR1bits.ADIF = 0;              // Limpieza de bandera de interrupción
    }    
    if(INTCONbits.RBIF){                // Verificación de interrupción del PORTB
        if(!PORTBbits.RB0){             // Verificación de interrupción de RB0 
            MODO_SLEEP = 1;             // Activación de Modo Sleep (Bandera)
            PORTEbits.RE0 = 1;          // Activación de pin RE0 para indicar modo sleep
            SLEEP();                    // Activación de Modo Sleep
        }
        else if(!PORTBbits.RB1){        // Verificación de interrupción de RB1 
            MODO_SLEEP = 0;             // Desactivación de Modo Sleep
            PORTEbits.RE0 = 0;          // Desactivación de pin RE0 para indicar modo sleep
        }
        INTCONbits.RBIF = 0;            // Limpieza de bandera de interrupción del PORTB
    }
    return;
}

/*------------------------------------------------------------------------------
 * CICLO PRINCIPAL
 ------------------------------------------------------------------------------*/
void main(void) {
    setup();
    while(1){
        if (!MODO_SLEEP){               // Activo solo si no esta en modo SLEEP
            if(ADCON0bits.GO == 0){             
                ADCON0bits.GO = 1;      // Iniciar proceso de conversión     
            }
        }
    }
    return;
}

/*------------------------------------------------------------------------------
 * CONFIGURACION 
 ------------------------------------------------------------------------------*/
void setup(void){   
    OSCCONbits.IRCF = 0b0110;    // 1MHz
    OSCCONbits.SCS = 1;         // Reloj interno
    
    ANSEL = 0b00000001;         // AN0 como entrada analógicas
    ANSELH = 0x00;              // I/O digitales
        
    TRISA = 0b00000001;         // AN0 como entrada y el resto como salidas  
    TRISBbits.TRISB0 = 1;       // RBO como entrada
    TRISBbits.TRISB1 = 1;       // RB1 como entrada   
    TRISC = 0x00;               // PORTC como salida
    TRISE = 0x00;               // PORTE como salida
    
    PORTA = 0x00;               // Limpieza del PORTA
    PORTB = 0x00;               // Limpieza del PORTB
    PORTC = 0x00;               // Limpieza del PORTC
    PORTE = 0x00;               // Limpieza del PORTE
    
    // Configuración de interrupciones por cambio de estado para PORTB
    OPTION_REGbits.nRBPU = 0;   // Habilitación de resistencias de pull-up del PORTB
    WPUBbits.WPUB0 = 1;         // Habilitación de resistencia de pull-up de RB0
    WPUBbits.WPUB1 = 1;         // Habilitación de resistencia de pull-up de RB1
    
    // Configuración de interrucpiones
    PIR1bits.ADIF = 0;          // Limpiamos bandera de ADC
    PIE1bits.ADIE = 1;          // Habilitamos interrupcion de ADC
    INTCONbits.GIE = 1;         // Habilitamos interrupciones globales
    INTCONbits.PEIE = 1;        // Habilitamos interrupciones de perifericos
    INTCONbits.RBIE = 1;        // Habilitación de interrupciones del PORTB
    IOCBbits.IOCB0 = 1;         // Habilitación de interrupción por cambio de estado para RB0
    IOCBbits.IOCB1 = 1;         // Habilitación de interrupción por cambio de estado para RB1
    
    
    // Configuración ADC
    ADCON0bits.ADCS = 0b11;         // FRC -> Funcione como SLEEP
    ADCON1bits.VCFG0 = 0;           // VDD
    ADCON1bits.VCFG1 = 0;           // VSS
    ADCON0bits.CHS = 0b0000;        // Selección de canal AN0
    ADCON1bits.ADFM = 0;            // Configuración de justificado a la izquierda
    ADCON0bits.ADON = 1;            // Habilitación del modulo ADC
    __delay_us(40);                 // Sample time
    
    return;
}           