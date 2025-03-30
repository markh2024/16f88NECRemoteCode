#include <xc.h>



// CONFIG1
#pragma config FOSC = INTOSCCLK // Oscillator Selection bits (INTRC oscillator; CLKO function on RA6/OSC2/CLKO pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select bit (MCLR pin function is MCLR)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable bit (RB3 is digital I/O)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// CONFIG2
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal External Switchover mode enabled)

#define _XTAL_FREQ 4000000  // Define clock speed (4MHz)

// Relay & LED Pins
#define RY_1   RA3
#define RY_2   RA4
#define RY_3   RA5
#define LED_1  RA0
#define LED_2  RA1
#define LED_3  RA2

// IR Receiver Pin
#define IR_PIN RB4

// NEC Decoding Variables
volatile unsigned char bitIndex = 0;
volatile unsigned long irData = 0;
volatile unsigned char irReceived = 0;

// Function Prototypes
void setup();
void processIRCommand(unsigned char command);
void __interrupt() ISR(void);

void main() {
    setup();
    
    while(1) {
        if (irReceived) {
            unsigned char command = (irData >> 8) & 0xFF; // Extract command byte
            processIRCommand(command);
            irReceived = 0;
        }
    }
}

// Setup function
void setup() {
    // Configure I/O
    TRISA = 0b11000000;  // Set RA0-RA5 as output
    TRISB = 0b00010000;  // Set RB4 (IR Receiver) as input
    PORTA = 0x00;        // Clear all outputs

    // Enable Timer1 (for pulse width measurement)
    T1CON = 0b00110001; // Prescaler 1:8, Timer1 ON
    TMR1 = 0;

    // Enable External Interrupt on RB4 (IR Receiver)
    INTCONbits.RBIE = 1; // Enable Port B change interrupt
    INTCONbits.GIE = 1;  // Enable Global Interrupts
    INTCONbits.PEIE = 1; // Enable Peripheral Interrupts
}

// Interrupt Service Routine (ISR)
void __interrupt() ISR(void) {
    if (INTCONbits.RBIF) {  // If Port B change interrupt (IR Signal Detected)
        unsigned int pulseWidth = TMR1; // Capture pulse width
        TMR1 = 0; // Reset Timer1
        INTCONbits.RBIF = 0; // Clear interrupt flag
        
        // NEC Protocol Decoding
        if (pulseWidth > 8500 && pulseWidth < 9500) {
            bitIndex = 0;
            irData = 0;
        } else if (bitIndex < 32) {
            irData <<= 1;
            if (pulseWidth > 1400) irData |= 1; // Logical '1'
            bitIndex++;
        }
        if (bitIndex >= 32) {
            irReceived = 1;
            bitIndex = 0;
        }
    }
}

// Process IR Command
void processIRCommand(unsigned char command) {
    switch (command) {
        case 0x1A:  // VIDEO_SELECT
            RY_2 = !RY_2;
            break;
        case 0x1B:  // MODE_SELECT
            RY_1 = !RY_1;
            break;
        case 0x05:  // MENU_SELECT
            RY_3 = !RY_3;
            break;
    }
}

