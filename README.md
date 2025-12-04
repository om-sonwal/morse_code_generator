📡 LPC2148 Morse Code Generator

A complete embedded system project that turns a numeric keypad into a Morse Code station. This system runs on the NXP LPC2148 microcontroller, allowing users to type alphanumeric text using a T9-style multi-tap input and transmit it as audio dots and dashes.

🚀 Key Features:

Multi-Tap Typing: Input text using a 4x4 keypad similar to old mobile phones (e.g., tap '2' twice for 'B').
Audio Transmission: Generates precise Morse Code audio signals via an active buzzer.
Visual Feedback: Displays both the current character and the message status on a 16x2 LCD.
Pin Efficient: Utilizes the PCF8574 I2C I/O Expander to drive the LCD, saving valuable GPIO pins.
Simulation Optimized: The code is calibrated for 12MHz operation to ensure smooth simulation in Proteus without CPU overload.

🔌 Hardware Connections:
1. I2C Display Interface
LPC P0.2: Connect to SCL (PCF8574 Pin 15). Must include a 4.7kΩ pull-up resistor to VCC.
LPC P0.3: Connect to SDA (PCF8574 Pin 14). Must include a 4.7kΩ pull-up resistor to VCC.
Address Pins: Connect A0, A1, and A2 to GND (Sets address to 0x40).

2. Audio Output
LPC P0.10: Connects to the Base of a BC547 Transistor via a 10kΩ resistor.
Transistor: The Collector connects to the Buzzer Negative (-). The Emitter connects to Ground.
Buzzer: The Positive (+) terminal connects to VCC.

3. Keypad Matrix (4x4)
Rows (Outputs): Connect LPC P1.16 through P1.19 to Keypad Rows A, B, C, D.
Columns (Inputs): Connect LPC P1.20 through P1.23 to Keypad Columns 1, 2, 3, 4.

⚠️ Important: You must connect a 10kΩ Pull-Down Resistor from each Column pin to Ground.


🛠 Component List:

Microcontroller: NXP LPC2148 or LPC2138

Display: 16x2 LCD with PCF8574 I2C Module

Input: 4x4 Matrix Keypad

Audio: Active Buzzer (Rated 3V or 5V)

Switching: BC547 NPN Transistor

Passive:

4x 10kΩ Resistors (Keypad)

1x 10kΩ Resistor (Transistor Base)

2x 4.7kΩ Resistors (I2C Pull-ups)

⚙️ Simulation Guide (Proteus):

To ensure the project runs correctly in simulation without crashing:

Clock Speed: Double-click the LPC2148 chip and set the Crystal Frequency to 12MHz.

Buzzer Power: Double-click the Buzzer component. Set the Operating Voltage to 3V and Load Resistance to 500.

Wiring Check: Ensure the I2C resistors (on SDA/SCL) are connected to Power, not Ground.


🎮 How to Use:

Power On: The system will initialize and show "Morse System" on the LCD.

Type Message:
Press 2 once for 'A', twice for 'B', three times for 'C'.
Press 3 for D, E, F.
Press 0 for Space.

Send Morse: Press the = key. The system will convert the last typed character into audio Morse code.

Clear: Press C to reset the input.

💻 Software & Tools:

IDE: Keil uVision 4 or 5

Simulator: Proteus 8 Professional

