# README: EEPROM with AT89C51 Microcontroller

## Overview

This project demonstrates interfacing an external EEPROM (24C02) with the AT89C51 microcontroller using the I2C protocol. The system performs the following tasks:

1. Receives a predefined text ("hi  i am raj ") via UART.
2. Displays "Receiving Data..." on a 16x2 LCD while data is being received.
3. Stores each received character into the EEPROM.
4. Reads back the data from EEPROM and transmits it via UART.
5. Displays "Transmitting..." on the LCD during the transmission phase.

---

## Hardware Connections

### Components Used:

- **AT89C51 Microcontroller**
- **24C02 EEPROM**
- **16x2 LCD Display**

### Pin Configuration:

#### I2C Connections:

- **EEPROM SDA** (Data Line): Connected to **P1.4** of AT89C51.
- **EEPROM SCL** (Clock Line): Connected to **P1.5** of AT89C51.

#### LCD Connections:

- **RS**: Connected to **P1.0**
- **RW**: Connected to **P1.1**
- **EN**: Connected to **P1.2**
- **Data Pins**: Connected to **Port 2 (P2.0 - P2.7)**

#### UART Connections:

- TX and RX lines connected to appropriate serial pins of the microcontroller.

---

## Code Explanation

### 1. **EEPROM Write Function**

The `eeprom_write` function writes a single byte to a specific memory address in the EEPROM. It uses the I2C protocol for communication.

```c
void eeprom_write(unsigned char address, unsigned char mydata) {
    i2c_start();               // Start I2C communication
    i2c_write(0xA0);           // Send EEPROM write control byte (address + write bit)
    i2c_write(address);        // Send memory address to write to
    i2c_write(mydata);         // Send data to write
    i2c_stop();                // Stop I2C communication
    delay(10);                 // Wait for write cycle to complete
}
```

- **Parameters**:

  - `address`: Memory location in EEPROM to write the data.
  - `mydata`: The data byte to be written.

- **Sequence**:

  1. Send a start condition.
  2. Send the EEPROM address (0xA0 for write operations).
  3. Specify the target memory address.
  4. Send the data byte.
  5. Send a stop condition to terminate communication.

### 2. **EEPROM Read Function**

The `eeprom_read` function reads a single byte from a specific memory address in the EEPROM.

```c
unsigned char eeprom_read(unsigned char address) {
    unsigned char mydata;
    i2c_start();               // Start I2C communication
    i2c_write(0xA0);           // Send EEPROM write control byte (address + write bit)
    i2c_write(address);        // Specify memory address to read from
    i2c_start();               // Repeated start for read operation
    i2c_write(0xA1);           // Send EEPROM read control byte (address + read bit)
    mydata = i2c_read(0);      // Read data with no acknowledgment
    i2c_stop();                // Stop I2C communication
    return mydata;             // Return the read data byte
}
```

- **Parameters**:

  - `address`: Memory location in EEPROM to read the data from.

- **Sequence**:

  1. Send a start condition.
  2. Send the EEPROM address (0xA0 for write operations).
  3. Specify the target memory address.
  4. Send a repeated start condition.
  5. Send the EEPROM address (0xA1 for read operations).
  6. Read the data byte.
  7. Send a stop condition to terminate communication.

### 3. **Main Function Logic**

The main function orchestrates the flow of receiving, storing, and transmitting data:

#### Receiving Data:

1. Display "Receiving Data..." on the LCD.
2. Receive the predefined string `"hi  i am raj "` character by character via UART.
3. For each received character:
   - Verify that it matches the expected string.
   - Store the character into EEPROM using the `eeprom_write` function.

#### Transmitting Data:

1. Display "Transmitting..." on the LCD.
2. Read the stored string back from EEPROM using the `eeprom_read` function.
3. Transmit the read string via UART.
4. Display "Process Done" on the LCD upon completion.

---

## Key Features

1. **I2C Protocol**:

   - Used for communication between AT89C51 and 24C02 EEPROM.
   - Simple and efficient for serial data transfer.

2. **LCD Integration**:

   - Provides user feedback during the data processing stages.
   - Displays current operation status ("Receiving Data..." or "Transmitting...").

3. **UART Communication**:

   - Facilitates the exchange of data between the microcontroller and an external device.
   - Echoes received data back to the sender during the transmission phase.

---

## How to Use

1. **Connect Hardware**:

   - Wire the AT89C51 microcontroller, 24C02 EEPROM, and LCD as described in the "Hardware Connections" section.

2. **Flash Code**:

   - Compile and load the provided code into the AT89C51 microcontroller using Keil uVision or a similar IDE.

3. **Test Functionality**:

   - Send the string `"hi  i am raj "` via UART.
   - Observe the "Receiving Data..." and "Transmitting..." messages on the LCD.
   - Verify that the received data is correctly echoed back after being stored in EEPROM.

---

## Conclusion

This project demonstrates the practical integration of an external EEPROM with an AT89C51 microcontroller. The code showcases how to utilize the I2C protocol for reading and writing data, combined with UART and LCD for real-time feedback and communication. By following the steps provided, you can easily replicate and expand upon this functionality for other applications.

