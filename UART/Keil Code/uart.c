#include <reg51.h>
#include <intrins.h>
// Define LCD Pins
sbit rs = P1^0;
sbit rw = P1^1;
sbit en = P1^2;

// Define I2C Pins
sbit SDA = P1^4; // I²C Data line
sbit SCK = P1^5; // I²C Clock line




void delay(int a);
void uart_init();
void uart_tx(char ch);
char uart_rx();
void lcd_cmd(unsigned char ch);
void lcd_init();
void lcd_data(unsigned char ch);
void lcd_string(char *str);
void i2c_start();
void i2c_stop();
void i2c_write(unsigned char mydata);
unsigned char i2c_read(unsigned char ack);
void eeprom_write(unsigned char address, unsigned char mydata);
unsigned char eeprom_read(unsigned char address);


unsigned long milliseconds = 0;

void timer_init() {
    TMOD |= 0x01; // Timer 0, mode 1 (16-bit timer)
    TH0 = 0x00;   // High byte of Timer 0
    TL0 = 0x00;   // Low byte of Timer 0
    TR0 = 1;      // Start Timer 0
}


unsigned long get_elapsed_time() {
    unsigned int timer_value = (TH0 << 8) | TL0; // Combine high and low bytes
    return (timer_value * 12) / (11059200 / 1000); // Convert to milliseconds
}

void delay(int a) {
    int i, j;
    for (i = 0; i < a; i++) {
        for (j = 0; j < 250; j++) {
            
            _nop_(); 
        }
    }
}

void uart_init() {
    SCON = 0x50;  
    TMOD = 0x20;  // Timer 1, mode 2
    TH1 = 0xFD;   // Baud rate 9600
    TR1 = 1;      // Start Timer 1
}

/*void uart_init() {
    SCON = 0x50;  // 8-bit UART mode, REN enabled
    TMOD = 0x20;  // Timer 1, mode 2 (auto-reload)
    TH1 = 0xF9;   // Baud rate 2400
    TR1 = 1;      // Start Timer 1
}*/

void uart_tx(char ch) {
    SBUF = ch;
    while (TI == 0);
    TI = 0;
}

char uart_rx() {
    while (RI == 0);
    RI = 0;
    return SBUF;
}

void lcd_cmd(unsigned char ch) {
    P2 = ch;
    rs = 0;
    rw = 0;
    en = 1;
    delay(10);
    en = 0;
}

void lcd_init() {
    lcd_cmd(0x38); // 8-bit mode
    lcd_cmd(0x0C); // Display ON, Cursor OFF
    lcd_cmd(0x01); // Clear Display
    lcd_cmd(0x80); // Set Cursor to Line 1
}

void lcd_data(unsigned char ch) {
    P2 = ch;
    rs = 1;
    rw = 0;
    en = 1;
    delay(10);
    en = 0;
}

void lcd_string(char *str) {
    int k = 0;
    while (str[k] != '\0') {
        lcd_data(str[k]);
        k++;
    }
}

void i2c_start() {
    SDA = 1;
    SCK = 1;
    delay(1);
    SDA = 0;
    delay(1);
    SCK = 0;
}

void i2c_stop() {
    SDA = 0;
    SCK = 1;
    delay(1);
    SDA = 1;
}

void i2c_write(unsigned char mydata) {
    int i;
    for (i = 0; i < 8; i++) {
        SDA = (mydata & 0x80) ? 1 : 0;
        mydata <<= 1;
        SCK = 1;
        delay(1);
        SCK = 0;
    }
    // Acknowledge
    SDA = 1;
    SCK = 1;
    delay(1);
    SCK = 0;
}

unsigned char i2c_read(unsigned char ack) {
    unsigned char mydata = 0;
    int i;
    SDA = 1; // Release SDA for input
    for (i = 0; i < 8; i++) {
        SCK = 1;
        delay(1);
        mydata = (mydata << 1) | SDA;
        SCK = 0;
    }
    // Send Acknowledge
    SDA = ack ? 0 : 1;
    SCK = 1;
    delay(1);
    SCK = 0;
    return mydata;
}

void eeprom_write(unsigned char address, unsigned char mydata) {
    i2c_start();
    i2c_write(0xA0);       // EEPROM write address
    i2c_write(address);    // Memory location
    i2c_write(mydata);       // Data to write
    i2c_stop();
    delay(10);
}

unsigned char eeprom_read(unsigned char address) {
    unsigned char mydata;
    i2c_start();
    i2c_write(0xA0);       // EEPROM write address
    i2c_write(address);    // Memory location
    i2c_start();
    i2c_write(0xA1);       // EEPROM read address
    mydata = i2c_read(0);    // Read data
    i2c_stop();
    return mydata;
}

void display_rate_on_lcd(unsigned long rate) {
    char rate_str[16];
    int i = 0, j;
    char temp;

    // Convert rate to string by extracting digits
    if (rate == 0) {
        rate_str[i++] = '0'; // Handle the case where rate is 0
    } else {
        while (rate > 0 && i < (sizeof(rate_str) - 1)) {
            rate_str[i++] = (rate % 10) + '0'; 
            rate /= 10;
        }

        // Reverse the string in place to correct digit order
        for (j = 0; j < i / 2; j++) {
            temp = rate_str[j];
            rate_str[j] = rate_str[i - j - 1];
            rate_str[i - j - 1] = temp;
        }
    }

    
    rate_str[i++] = ' ';
    rate_str[i++] = 'b';
    rate_str[i++] = 'p';
    rate_str[i++] = 's';
    rate_str[i] = '\0'; // Null-terminate the string

    
    lcd_cmd(0x01); // Clear LCD
    lcd_string("Trx @ :");

    
    lcd_cmd(0xC0); 
    lcd_string(rate_str);
}


void main() {
    unsigned char address = 0x00;          // Start address for EEPROM write
    unsigned char transmit_address = 0x00; // Address for EEPROM read during transmission
    char received_char;
    unsigned long start_time, current_time, elapsed_time;
    unsigned long transmitted_bytes = 0;
    unsigned long byte_count = 0;
    unsigned long receiving_rate = 0;     
    unsigned long transmitting_rate = 0;  

    uart_init();
    lcd_init();
    timer_init();
    lcd_string("Wating for Data");

    start_time = get_elapsed_time(); // Initialize timer for speed calculation

    while (1) {
        current_time = get_elapsed_time();
        elapsed_time = current_time - start_time;

        if (elapsed_time >= 2000) { // Update receiving speed every 2 seconds
            receiving_rate = (byte_count * 8) / elapsed_time;
            byte_count = 0;         // Reset byte count
            start_time = current_time; // Reset start time
        }

        // Receive a character via UART
        received_char = uart_rx();

        if (received_char == '8') {
            // Stop receiving and start transmitting
            lcd_cmd(0x01); // Clear LCD
            lcd_string("Preparing Transmit");

            start_time = get_elapsed_time(); // Reset timer for transmission rate calculation
            transmitted_bytes = 0;

            while (transmit_address < address) {
                // Read data from EEPROM
                char data_to_transmit = eeprom_read(transmit_address);

                // Transmit data via UART
                uart_tx(data_to_transmit);

                // Increment transmit address and transmitted bytes
                transmit_address++;
                transmitted_bytes++;

                // Calculate transmission rate
                current_time = get_elapsed_time();
                elapsed_time = current_time - start_time;
                transmitting_rate = (transmitted_bytes * 8) / elapsed_time;

                // Display rate on the LCD
                display_rate_on_lcd(transmitting_rate);
            }

            // After transmission, show "Transmission Done" on the LCD
            lcd_cmd(0x01); // Clear LCD
            lcd_string("Transmission Done");
            break; // Exit the main loop
        } else {
            // Save the character in EEPROM
            eeprom_write(address, received_char);
            byte_count++; // Increment byte count

            // Update LCD with "Receiving"
            lcd_cmd(0x01); // Clear LCD
					  lcd_string("Receiving :");
					  lcd_data(received_char);

            // Increment the EEPROM address
            address++;
            if (address == 0x00) {  // Handle address overflow
                lcd_cmd(0x01);
                lcd_string("EEPROM Full!");
                break; // Stop further execution
            }
        }

        
        delay(100);
    }
}
