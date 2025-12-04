/*
 * Project: LPC2138/2148 Morse Code (Standard)
 * Crystal Frequency: 12MHz (User Configured)
 * CPU Clock: 60MHz (PLL Enabled)
 * PCLK: 30MHz
 *
 * *** WIRING CHECKLIST ***
 * 1. P0.2 & P0.3 -> Resistors MUST go to VCC (Power), NOT Ground.
 * 2. Keypad Cols (P1.20-23) -> MUST have 10k Resistors to Ground.
 * 3. I2C Address -> 0x40 (Since A0,A1,A2 are Grounded).
 */

#include <lpc214x.h>

// --- Hardware ---
#define I2C_ADDR     0x40       // Default for PCF8574 with A0-A2 grounded
#define BUZZER_PIN   (1 << 10)  // P0.10

// --- Timing (Calibrated for 60MHz) ---
#define DOT_DELAY    150000     // ~100ms
#define DASH_DELAY   (DOT_DELAY * 3)
#define SYMBOL_SPACE (DOT_DELAY)
#define LETTER_SPACE (DOT_DELAY * 3)

// --- Morse Tables ---
const char *morse_a_z[] = { ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.." };
const char *morse_0_9[] = { "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----." };

// --- Globals ---
char current_char = 0;
int tap_index = -1;
int last_key = -1;

// --- System Init ---
void Init_PLL(void) {
    // 12MHz Crystal * 5 = 60MHz CPU
    PLL0CFG = 0x24;           // M=5, P=2
    PLL0CON = 0x01;           // Enable PLL
    PLL0FEED = 0xAA; PLL0FEED = 0x55; 
    
    while(!(PLL0STAT & 0x00000400)); // Wait for Lock
    
    PLL0CON = 0x03;           // Connect PLL
    PLL0FEED = 0xAA; PLL0FEED = 0x55;
    
    VPBDIV = 0x02;            // PCLK = 30MHz
}

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 6000; j++);
}

void delay_short(unsigned int count) {
    while(count--);
}

// --- I2C Driver ---
void I2C_Init() {
    PINSEL0 |= 0x00000050;    
    // PCLK=30MHz. 100kHz I2C = 300 ticks
    I2C0SCLH = 150; I2C0SCLL = 150;
    I2C0CONCLR = 0x6C; I2C0CONSET = 0x40;
}

void I2C_WaitForSI() {
    int safe = 0;
    while (!(I2C0CONSET & 0x08)) {
        safe++; if(safe > 200000) break;
    }
}

void I2C_Write(unsigned char data) {
    I2C0DAT = data; I2C0CONCLR = 0x28; I2C_WaitForSI();
}

void I2C_Start() { I2C0CONSET = 0x20; I2C_WaitForSI(); }
void I2C_Stop()  { I2C0CONSET = 0x10; I2C0CONCLR = 0x08; }

// --- LCD Driver ---
void LCD_Send(unsigned char val, unsigned char rs) {
    unsigned char data = (val & 0xF0) | (rs & 0x01) | 0x08; // 0x08=Backlight
    I2C_Start();
    I2C_Write(I2C_ADDR);
    I2C_Write(data | 0x04); delay_short(1000); // Pulse En
    I2C_Write(data & ~0x04); delay_short(1000);
    I2C_Stop();
}

void LCD_Byte(unsigned char val, unsigned char rs) {
    LCD_Send(val, rs); LCD_Send(val << 4, rs);
}

void LCD_Cmd(unsigned char cmd) { LCD_Byte(cmd, 0); delay_ms(2); }
void LCD_Data(unsigned char data) { LCD_Byte(data, 1); delay_ms(1); }
void LCD_String(char *msg) { while(*msg) LCD_Data(*msg++); }

void LCD_Init() {
    delay_ms(50);
    LCD_Send(0x30, 0); delay_ms(5);
    LCD_Send(0x30, 0); delay_ms(1);
    LCD_Send(0x30, 0); delay_ms(1);
    LCD_Send(0x20, 0); 
    LCD_Cmd(0x28); LCD_Cmd(0x0C); LCD_Cmd(0x01); 
    delay_ms(50);
}

// --- Keypad ---
char Keypad_Scan() {
    unsigned int row, col, scanVal;
    char keys[4][4] = { {'7','8','9','/'}, {'4','5','6','*'}, {'1','2','3','-'}, {'C','0','=','+'} };

    IO1DIR |= 0x000F0000; IO1DIR &= ~0x00F00000; 

    for (row = 0; row < 4; row++) {
        IO1CLR = 0x000F0000; IO1SET = (1 << (16 + row));
        delay_short(2000); 

        scanVal = (IO1PIN >> 20) & 0x0F;
        
        if (scanVal != 0) {
            for (col = 0; col < 4; col++) {
                if (scanVal & (1 << col)) {
                    delay_ms(20); // Debounce
                    while ((IO1PIN >> 20) & 0x0F); 
                    return keys[row][col];
                }
            }
        }
    }
    return 0;
}

// --- Logic ---
char Get_MultiTap(char key, int idx) {
    if (key == '2') return "ABC2"[idx % 4];
    if (key == '3') return "DEF3"[idx % 4];
    if (key == '4') return "GHI4"[idx % 4];
    if (key == '5') return "JKL5"[idx % 4];
    if (key == '6') return "MNO6"[idx % 4];
    if (key == '7') return "PQRS7"[idx % 5];
    if (key == '8') return "TUV8"[idx % 4];
    if (key == '9') return "WXYZ9"[idx % 5];
    if (key == '0') return ' ';
    return key;
}

const char* Get_Morse(char c) {
    if (c >= 'A' && c <= 'Z') return morse_a_z[c - 'A'];
    if (c >= '0' && c <= '9') return morse_0_9[c - '0'];
    return 0;
}

void Play_Morse(const char *code) {
    while (*code) {
        if (*code == '.') {
            IO0SET = BUZZER_PIN; delay_short(DOT_DELAY);
            IO0CLR = BUZZER_PIN; delay_short(SYMBOL_SPACE);
        } else if (*code == '-') {
            IO0SET = BUZZER_PIN; delay_short(DASH_DELAY);
            IO0CLR = BUZZER_PIN; delay_short(SYMBOL_SPACE);
        }
        code++;
    }
    delay_short(LETTER_SPACE);
}

int main() {
    char key;
    const char *morseCode;

    Init_PLL(); // Set to 60MHz
    PINSEL2 = 0; 
    IO0DIR |= BUZZER_PIN;
    
    I2C_Init();
    LCD_Init();
    
    LCD_Cmd(0x80); LCD_String("Morse System");
    LCD_Cmd(0xC0); LCD_String("Ready");
    delay_ms(1000);
    LCD_Cmd(0x01);

    while(1) {
        key = Keypad_Scan();
        if (key != 0) {
            if (key == '=') {
                if (current_char != 0) {
                    LCD_Cmd(0x01); 
                    LCD_Cmd(0x80); LCD_String("Sending: "); LCD_Data(current_char);
                    
                    morseCode = Get_Morse(current_char);
                    if (morseCode) {
                        LCD_Cmd(0xC0); LCD_String((char*)morseCode);
                        Play_Morse(morseCode);
                    }
                    current_char = 0; last_key = -1; tap_index = -1;
                    delay_ms(500); LCD_Cmd(0x01);
                }
            } else if (key == 'C') {
                LCD_Cmd(0x01); current_char = 0;
            } else {
                if (key == last_key) tap_index++;
                else { tap_index = 0; last_key = key; }
                
                current_char = Get_MultiTap(key, tap_index);
                LCD_Cmd(0x01); LCD_Cmd(0x80); LCD_String("Char: "); LCD_Data(current_char);
            }
        }
    }
}