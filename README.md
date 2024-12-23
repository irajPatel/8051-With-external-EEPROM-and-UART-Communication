# README: EEPROM and AT89C51 Integration with UART and LCD Display

This README file provides detailed instructions for connecting an EEPROM (24C02) with the AT89C51 microcontroller, explaining the code used for EEPROM read/write operations, and demonstrating how to calculate and display the speed of UART transmission on an LCD.

---

## Hardware Connections

### Components:
- **AT89C51 Microcontroller**
- **EEPROM (24C02)**
- **16x2 LCD Display**
- **Virtual Terminal (Proteus)**
- **Pull-Up Resistors (4.7kΩ)**

### Connections:

#### EEPROM (24C02) to AT89C51
- **SDA (EEPROM)** -> **P1.4 (AT89C51)**
- **SCL (EEPROM)** -> **P1.5 (AT89C51)**
- **VCC (EEPROM)** -> **5V**
- **GND (EEPROM)** -> **Ground**
- **A0, A1, A2 (EEPROM)** -> **Ground** (for default I²C address 0xA0 for write, 0xA1 for read)
- **WP (EEPROM)** -> **Ground** (for write operations)
- **Pull-Up Resistors**: Connect 4.7kΩ resistors between SDA, SCL, and 5V.

#### LCD to AT89C51
- **RS (LCD)** -> **P1.0 (AT89C51)**
- **RW (LCD)** -> **P1.1 (AT89C51)**
- **EN (LCD)** -> **P1.2 (AT89C51)**
- **Data Lines (D4-D7)** -> **P2.4-P2.7 (AT89C51)**

#### UART Communication (Virtual Terminal)
- **TXD (AT89C51)** -> **RXD (Virtual Terminal)**
- **RXD (AT89C51)** -> **TXD (Virtual Terminal)**

---

## Code Explanation

### 1. UART Initialization
```c
void uart_init() {
    SCON = 0x50;  // 8-bit UART mode
    TMOD = 0x20;  // Timer 1, Mode 2
    TH1 = 0xFD;   // Baud rate 9600
    TR1 = 1;      // Start Timer 1
}
```
This configures the UART for 8-bit communication with a baud rate of 9600.

### 2. EEPROM Write
```c
void eeprom_write(unsigned char address, unsigned char mydata) {
    i2c_start();
    i2c_write(0xA0);       // EEPROM write address
    i2c_write(address);    // Memory location
    i2c_write(mydata);     // Data to write
    i2c_stop();
    delay(10);
}
```
**Process:**
1. Send a START condition.
2. Send the EEPROM's I²C address for write operations.
3. Send the memory address where data will be stored.
4. Write the data byte.
5. Send a STOP condition.

### 3. EEPROM Read
```c
unsigned char eeprom_read(unsigned char address) {
    unsigned char mydata;
    i2c_start();
    i2c_write(0xA0);       // EEPROM write address
    i2c_write(address);    // Memory location
    i2c_start();
    i2c_write(0xA1);       // EEPROM read address
    mydata = i2c_read(0);  // Read data
    i2c_stop();
    return mydata;
}
```
**Process:**
1. Send a START condition.
2. Write the EEPROM's I²C address and memory location.
3. Send a REPEATED START condition.
4. Write the EEPROM's I²C address for read operations.
5. Read the data byte and send a STOP condition.

### 4. Main Function
The main function receives text data via UART, calculates the transmission speed, stores it in EEPROM, and displays the speed on the LCD.

#### Implementation:
```c
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

    // Calculate transmission speed
    elapsed_time = start_time + (byte_count * 10); // Simplified calculation
    unsigned int speed = byte_count / elapsed_time; // Bytes per millisecond

    // Display speed on LCD
    lcd_cmd(0x01); // Clear LCD
    lcd_string("Speed: ");
    lcd_data(speed + '0');

    // Transmit data back from EEPROM
    lcd_cmd(0x01); // Clear LCD
    lcd_string("Transmitting");

    for (unsigned char i = 0; i < byte_count; i++) {
        received_char = eeprom_read(i);
        uart_tx(received_char);
    }
}
```

---

## References
To calculate transmission speed, the logic is inspired by concepts discussed in [this StackOverflow thread](https://stackoverflow.com/questions/14671657/how-to-measure-serial-receive-byte-speed-eg-bytes-per-second).

---

## Summary
This project demonstrates how to:
1. Interface an AT89C51 microcontroller with EEPROM (24C02) and an LCD.
2. Receive and store data via UART.
3. Calculate and display the UART transmission speed.
4. Transmit stored data back via UART.



