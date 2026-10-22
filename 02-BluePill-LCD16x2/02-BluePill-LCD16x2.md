---

title: "STM32 Lesson — Interfacing a 16x2 Character LCD"  
tags:

- STM32
    
- Embedded-Systems
    
- STM32F103
    
- STM32CubeIDE
    
- GPIO
    
- LCD
    
- HAL
    

---

# STM32 Lesson: Interfacing a 16x2 Character LCD

## 1. Lesson Overview

In this lesson, we interfaced a 16x2 character LCD with an STM32F103C8T6 microcontroller using the STM32 HAL library.

The LCD was configured in **4-bit mode**, allowing us to display text while using only six GPIO pins.

### Hardware and Software

|Item|Description|
|---|---|
|Microcontroller|STM32F103C8T6|
|Development board|STM32 Blue Pill|
|Programmer/Debugger|ST-Link V2|
|Display|16x2 Character LCD|
|LCD Controller|HD44780-compatible controller|
|Development environment|STM32CubeIDE 2.2.0|
|Configuration tool|STM32CubeMX|
|Firmware library|STM32 HAL|
|Communication method|GPIO, 4-bit parallel interface|
|LCD supply voltage|5V|
|GPIO pins used|PA0–PA5|

### Learning Objectives

By the end of this lesson, you should understand how to:

- Create and configure an STM32 project using STM32CubeIDE and CubeMX.
    
- Configure GPIO pins as digital outputs.
    
- Understand the pinout and operating principles of a character LCD.
    
- Distinguish between LCD commands and character data.
    
- Send an 8-bit value through a 4-bit interface.
    
- Initialize an HD44780-compatible LCD.
    
- Display a string using custom HAL-based functions.
    
- Diagnose basic LCD wiring and contrast problems.
    

---

## 2. Understanding the 16x2 Character LCD

A 16x2 character LCD can display 16 characters per row and has two rows.

Its total visible character capacity is:

[  
16 \times 2 = 32 \text{ characters}  
]

Unlike a graphical OLED or TFT display, a character LCD does not normally require us to draw individual pixels. Instead, we send character codes and commands to its controller.

The controller manages the character display, cursor position, internal character memory, and other display operations.

### 2.1. LCD Pinout

A typical 16-pin character LCD has the following connections:

|Pin|Name|Function|
|---|---|---|
|1|VSS|Ground|
|2|VDD|Positive supply voltage|
|3|VO|Contrast adjustment input|
|4|RS|Register Select|
|5|RW|Read/Write control|
|6|E|Enable signal|
|7|D0|Data bit 0|
|8|D1|Data bit 1|
|9|D2|Data bit 2|
|10|D3|Data bit 3|
|11|D4|Data bit 4|
|12|D5|Data bit 5|
|13|D6|Data bit 6|
|14|D7|Data bit 7|
|15|A / LED+|Backlight anode|
|16|K / LED−|Backlight cathode|

The exact electrical specifications depend on the particular LCD module.

### 2.2. Important Control Pins

**RS — Register Select**

The RS pin determines how the LCD interprets the incoming value.

|RS level|Meaning|
|---|---|
|LOW (0)|Command register selected|
|HIGH (1)|Data register selected|

Examples of commands include clearing the display and configuring the display mode.

Data typically represents character codes to be displayed.

**RW — Read/Write**

The RW pin selects the direction of communication.

|RW level|Operation|
|---|---|
|LOW (0)|Write to LCD|
|HIGH (1)|Read from LCD|

In this project, RW is permanently connected to GND. Therefore, the microcontroller only writes to the LCD.

This simplifies the circuit because the firmware does not need to read the LCD's busy flag.

**E — Enable**

The Enable pin provides the timing signal that allows the LCD to latch the signals present on its control and data inputs.

In our implementation, we generate a pulse by setting E HIGH and then LOW.

**VO — Contrast Voltage**

The VO pin controls the contrast of the displayed characters.

Incorrect contrast can make the text nearly invisible or cause dark blocks to appear on the display.

The contrast voltage is separate from the backlight supply.

---

## 3. Understanding 4-Bit and 8-Bit Modes

An HD44780-compatible LCD commonly supports two parallel data modes.

### 3.1. 8-Bit Mode

In 8-bit mode, all eight data lines, D0 through D7, are used.

Advantages:

- Each byte can be transferred in one data operation.
    

Disadvantages:

- Eight GPIO pins are required for data alone.
    
- Additional pins are still needed for RS and E.
    

### 3.2. 4-Bit Mode

In 4-bit mode, only D4 through D7 are used.

Each 8-bit command or character value is transferred in two stages:

1. The upper four bits are transmitted.
    
2. The lower four bits are transmitted.
    

For example, consider the byte:

```
0x41 = 0100 0001
```

It is divided into two nibbles:

```
Upper nibble: 0100
Lower nibble: 0001
```

The LCD receives both nibbles and reconstructs the original byte.

A **nibble** is a group of four bits.

### 3.3. Why We Chose 4-Bit Mode

Our project uses:

- One GPIO pin for RS.
    
- One GPIO pin for E.
    
- Four GPIO pins for D4–D7.
    

Total:

[  
N_{\text{GPIO}} = 1 + 1 + 4 = 6  
]

This saves four GPIO pins compared with an 8-bit data interface.

---

## 4. Hardware Wiring

We connected the LCD to GPIOA on the STM32 Blue Pill.

### 4.1. Connection Table

|LCD Pin|LCD Signal|STM32 / Circuit Connection|
|---|---|---|
|1|VSS|GND|
|2|VDD|5V|
|3|VO|2 kΩ resistor to GND|
|4|RS|PA0|
|5|RW|GND|
|6|E|PA1|
|7–10|D0–D3|Not connected|
|11|D4|PA2|
|12|D5|PA3|
|13|D6|PA4|
|14|D7|PA5|
|15|A / LED+|Backlight connection, if used|
|16|K / LED−|Backlight connection, if used|

The LCD supply in this setup is 5V. The STM32 GPIO pins operate at 3.3V, so the electrical compatibility of the particular LCD module must be considered. A successful display test does not, by itself, guarantee that every 5V LCD input has adequate logic-level margin or that the setup is safe for every module.

### 4.2. Contrast Adjustment

Initially, the characters were difficult to read.

We connected a **2 kΩ resistor between VO and GND**, after which the contrast became readable on our particular module.

This is the result observed in this experiment, not a universal contrast circuit for every LCD.

The standard adjustable solution is a potentiometer connected between VDD and GND, with its wiper connected to VO. This allows the contrast voltage to be tuned.

A fixed resistor is less flexible, and the ideal contrast voltage depends on the module.

### 4.3. Backlight Versus Contrast

These are two different things:

- **Contrast** determines how clearly the character pixels appear relative to the background.
    
- **Backlight** illuminates the display from behind.
    

A display may show correctly initialized characters even when its backlight is disconnected.

If the characters are correctly positioned but difficult to see, inspect both the contrast voltage and the backlight circuit.

For the backlight, check the module's datasheet. If an external current-limiting resistor is required, use one of an appropriate value rather than connecting the LED directly to the supply.

### 4.4. Common Ground

The STM32 and LCD must share a common ground reference.

Without a common ground, the voltage levels generated by the STM32 may not be interpreted reliably by the LCD.

Always disconnect power before changing wiring.

---

## 5. Configuring the Project in STM32CubeIDE

We created a project for the STM32F103C8Tx device.

### 5.1. GPIO Configuration

The following pins were configured as outputs:

|Pin|Purpose|
|---|---|
|PA0|RS|
|PA1|Enable|
|PA2|D4|
|PA3|D5|
|PA4|D6|
|PA5|D7|

For each pin, we selected:

|Setting|Value|
|---|---|
|GPIO Mode|Output Push-Pull|
|Pull-up/Pull-down|No Pull|
|Output Speed|Low|

Push-pull output mode allows a GPIO pin to actively drive the signal LOW or HIGH.

Low output speed is sufficient for this relatively slow interface and helps avoid unnecessarily fast signal transitions.

### 5.2. Debug Configuration

We enabled **Serial Wire** under SYS configuration to retain SWD debugging support for the ST-Link.

After configuring the pins, we generated the project code and opened it in STM32CubeIDE.

### 5.3. GPIO Initialization

CubeMX generates the GPIO initialization function. A simplified representation of the configuration is:

```
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 |
                            GPIO_PIN_2 | GPIO_PIN_3 |
                            GPIO_PIN_4 | GPIO_PIN_5,
                      GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 |
                          GPIO_PIN_2 | GPIO_PIN_3 |
                          GPIO_PIN_4 | GPIO_PIN_5;

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
```

The exact generated code may differ depending on the CubeMX version and project settings.

The important steps are:

1. Enable the GPIOA peripheral clock.
    
2. Set the initial output levels.
    
3. Configure the selected pins as push-pull outputs.
    

The GPIO peripheral clock must be enabled before configuring or using its pins.

---

## 6. Implementing the LCD Driver

We implemented the LCD communication functions inside the `USER CODE BEGIN 0` section of `main.c`.

Placing custom functions in the appropriate user-code sections helps preserve them when CubeMX regenerates the project.

### 6.1. Define the Pin Mapping

```
#define LCD_RS_PORT GPIOA
#define LCD_RS_PIN  GPIO_PIN_0

#define LCD_E_PORT  GPIOA
#define LCD_E_PIN   GPIO_PIN_1

#define LCD_D4_PORT GPIOA
#define LCD_D4_PIN  GPIO_PIN_2

#define LCD_D5_PORT GPIOA
#define LCD_D5_PIN  GPIO_PIN_3

#define LCD_D6_PORT GPIOA
#define LCD_D6_PIN  GPIO_PIN_4

#define LCD_D7_PORT GPIOA
#define LCD_D7_PIN  GPIO_PIN_5
```

These macros give meaningful names to the pins.

For example:

```
LCD_RS_PIN
```

is easier to understand when reading the LCD driver than repeatedly writing `GPIO_PIN_0`.

The mapping also becomes easier to modify if the wiring changes.

### 6.2. Declare the Functions

```
static void LCD_Enable(void);
static void LCD_Send4Bits(uint8_t data);
static void LCD_SendCommand(uint8_t command);
static void LCD_SendData(uint8_t data);
static void LCD_Init(void);
static void LCD_Print(char *text);
```

These declarations tell the compiler the function names, arguments, and return types before their definitions appear.

The `static` keyword restricts these functions to the current C source file.

Because the implementation uses `uint8_t`, ensure that the project includes the appropriate integer type definitions, normally provided through `main.h` and the HAL headers.

---

## 7. Generating the Enable Pulse

The Enable signal tells the LCD when to latch the current input.

```
static void LCD_Enable(void)
{
    HAL_GPIO_WritePin(LCD_E_PORT,
                      LCD_E_PIN,
                      GPIO_PIN_SET);

    HAL_Delay(1);

    HAL_GPIO_WritePin(LCD_E_PORT,
                      LCD_E_PIN,
                      GPIO_PIN_RESET);

    HAL_Delay(1);
}
```

### How It Works

1. Set E HIGH.
    
2. Wait for 1 ms.
    
3. Set E LOW.
    
4. Wait for another 1 ms.
    

`HAL_GPIO_WritePin()` is a HAL function used to set or reset a GPIO output.

`HAL_Delay()` creates a blocking delay in milliseconds when the HAL time base is configured normally.

The 1 ms delays are deliberately conservative for this beginner experiment. They are much longer than the minimum timing requirements of typical HD44780-compatible modules, but they make the initial implementation straightforward.

A more optimized driver can use shorter delays that comply with the controller's datasheet.

---

## 8. Sending Four Bits to the LCD

The next function places a four-bit value on D4–D7.

```
static void LCD_Send4Bits(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_D4_PORT, LCD_D4_PIN,
        (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LCD_D5_PORT, LCD_D5_PIN,
        (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LCD_D6_PORT, LCD_D6_PIN,
        (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LCD_D7_PORT, LCD_D7_PIN,
        (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    LCD_Enable();
}
```

### 8.1. Understanding Bit Masks

A bit mask lets us inspect an individual bit without changing the other bits.

Consider:

```
data & 0x01
```

The mask `0x01` has the binary representation:

```
0001
```

The AND operation checks whether bit 0 is set.

Similarly:

|Expression|Mask|Bit checked|
|---|---|---|
|`data & 0x01`|`0001`|Bit 0|
|`data & 0x02`|`0010`|Bit 1|
|`data & 0x04`|`0100`|Bit 2|
|`data & 0x08`|`1000`|Bit 3|

The expression:

```
(condition) ? value_if_true : value_if_false
```

is the conditional, or ternary, operator.

For example:

```
(data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET
```

sets the GPIO output HIGH if bit 0 is set; otherwise, it sets the output LOW.

### 8.2. Why We Call LCD_Enable()

After placing the four bits on the data pins, we generate the Enable pulse so that the LCD can latch the nibble.

The function sends only four bits. It does not independently decide whether the nibble represents a command or character data; that is controlled by RS.

---

## 9. Sending an 8-Bit Command

Commands configure the LCD or control its behavior.

```
static void LCD_SendCommand(uint8_t command)
{
    HAL_GPIO_WritePin(LCD_RS_PORT,
                      LCD_RS_PIN,
                      GPIO_PIN_RESET);

    LCD_Send4Bits(command >> 4);
    LCD_Send4Bits(command & 0x0F);

    HAL_Delay(2);
}
```

### 9.1. Why Is RS LOW?

Setting RS LOW selects the command register.

Therefore, the LCD interprets the received byte as a control instruction rather than a character to display.

### 9.2. Splitting the Byte

An 8-bit command must be split into two nibbles.

**Upper nibble:**

```
command >> 4
```

The right-shift operator moves the upper four bits into the lower four positions.

**Lower nibble:**

```
command & 0x0F
```

The mask `0x0F` has the binary representation:

```
0000 1111
```

It preserves the lower four bits and clears the upper four bits.

### 9.3. Example

Suppose the command is:

```
0x28
```

Its binary representation is:

```
0010 1000
```

The function sends:

```
First nibble:  0010
Second nibble: 1000
```

The LCD reconstructs the byte from these two transmissions.

The 2 ms delay after transmission gives the LCD time to process the command. Some commands, particularly Clear Display and Return Home, require more processing time than ordinary commands.

---

## 10. Sending Character Data

To display a character, RS must be HIGH.

```
static void LCD_SendData(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_RS_PORT,
                      LCD_RS_PIN,
                      GPIO_PIN_SET);

    LCD_Send4Bits(data >> 4);
    LCD_Send4Bits(data & 0x0F);

    HAL_Delay(1);
}
```

### How It Works

1. Set RS HIGH to select the data register.
    
2. Send the upper nibble.
    
3. Send the lower nibble.
    
4. Wait for the LCD to process the character.
    

The main difference between `LCD_SendCommand()` and `LCD_SendData()` is the state of RS.

|Function|RS|Purpose|
|---|---|---|
|`LCD_SendCommand()`|LOW|Send a command|
|`LCD_SendData()`|HIGH|Send character data|

For example, the ASCII character `'A'` has the value `0x41`.

When sent through `LCD_SendData()`, the LCD receives that character code and displays the corresponding character.

The 1 ms delay is conservative and can be shortened in a later optimization, provided the LCD timing requirements are met.

---

## 11. Initializing the LCD

Before displaying text, the LCD must be initialized.

```
static void LCD_Init(void)
{
    HAL_Delay(50);

    HAL_GPIO_WritePin(LCD_RS_PORT,
                      LCD_RS_PIN,
                      GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LCD_E_PORT,
                      LCD_E_PIN,
                      GPIO_PIN_RESET);

    LCD_Send4Bits(0x03);
    HAL_Delay(5);

    LCD_Send4Bits(0x03);
    HAL_Delay(1);

    LCD_Send4Bits(0x03);
    HAL_Delay(1);

    LCD_Send4Bits(0x02);
    HAL_Delay(1);

    LCD_SendCommand(0x28);
    LCD_SendCommand(0x0C);
    LCD_SendCommand(0x06);
    LCD_SendCommand(0x01);

    HAL_Delay(2);
}
```

### 11.1. Initial Power-Up Delay

```
HAL_Delay(50);
```

The LCD needs time after power-up before it can reliably receive initialization commands.

The initial delay provides that time.

### 11.2. Why Send `0x03` Three Times?

The LCD's internal state after power-up may not yet be synchronized with the intended 4-bit communication mode.

The repeated initialization nibbles help place the controller in a known state.

### 11.3. Entering 4-Bit Mode

```
LCD_Send4Bits(0x02);
```

This special initialization step selects 4-bit operation for a typical HD44780-compatible controller.

It is important to distinguish this step from sending a normal command after the controller is already operating in 4-bit mode.

### 11.4. Function Set: `0x28`

```
LCD_SendCommand(0x28);
```

For a typical HD44780-compatible controller, this configures:

- 4-bit interface mode.
    
- Two-line display mode.
    
- 5×8 character font.
    

### 11.5. Display Control: `0x0C`

```
LCD_SendCommand(0x0C);
```

This enables the display and disables the cursor and cursor blinking.

### 11.6. Entry Mode Set: `0x06`

```
LCD_SendCommand(0x06);
```

This configures the cursor to advance to the right after each character is written, without shifting the entire display.

### 11.7. Clear Display: `0x01`

```
LCD_SendCommand(0x01);
```

This clears the displayed characters and returns the cursor to the home position.

Clear Display takes longer than many ordinary commands, so the firmware includes an additional delay afterward.

### Initialization Command Summary

|Value|Command|Purpose|
|---|---|---|
|`0x03`|Initialization nibble|Establish a known controller state|
|`0x02`|Set 4-bit interface|Switch to 4-bit operation|
|`0x28`|Function Set|4-bit, two-line, 5×8 font|
|`0x0C`|Display Control|Display ON, cursor OFF, blink OFF|
|`0x06`|Entry Mode Set|Increment cursor, no display shift|
|`0x01`|Clear Display|Clear characters and return home|

These values assume a typical HD44780-compatible controller. Compatible modules may differ in implementation details.

---

## 12. Displaying a String

We created a helper function that sends a string character by character.

```
static void LCD_Print(char *text)
{
    while (*text != '\0')
    {
        LCD_SendData((uint8_t)*text);
        text++;
    }
}
```

### Understanding the String

In C, a string is a sequence of characters terminated by a null character:

```
'\0'
```

The loop continues while the current character is not the null terminator.

### Understanding the Pointer

The variable `text` is a pointer to a character.

```
*text
```

accesses the character at the pointer's current position.

```
text++;
```

moves the pointer to the next character.

For example, the string:

```
"Hi!"
```

is stored conceptually as:

```
'H'  'i'  '!'  '\0'
```

The function sends `H`, then `i`, then `!`, and stops when it reaches `'\0'`.

For this lesson, the function accepts a modifiable `char *`. A later improvement would be to use `const char *`, since the function does not modify the string.

---

## 13. Main Application

After implementing the driver functions, we called them from `main()`.

```
int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();

    LCD_Init();

    LCD_Print("Hello, STM32!");

    while (1)
    {
    }
}
```

### Execution Sequence

**Step 1 —** `**HAL_Init()**`

Initializes the HAL library and its basic services, including the time base used by `HAL_Delay()`.

**Step 2 —** `**SystemClock_Config()**`

Configures the system clock. In this experiment, the clock configuration uses the internal 8 MHz HSI oscillator without enabling the PLL.

**Step 3 —** `**MX_GPIO_Init()**`

Configures GPIOA and the six LCD control/data pins.

**Step 4 —** `**LCD_Init()**`

Initializes the LCD controller and configures its operating mode.

**Step 5 —** `**LCD_Print()**`

Sends the text `Hello, STM32!` to the LCD one character at a time.

**Step 6 — Infinite loop**

```
while (1)
{
}
```

The program remains running after displaying the text.

No repeated display operation is necessary because the LCD retains its displayed characters until another operation changes them or power is removed.

---

## 14. Important STM32 HAL Concepts

### 14.1. `HAL_GPIO_WritePin()`

General form:

```
HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState);
```

|Parameter|Meaning|
|---|---|
|`GPIOx`|GPIO port, such as GPIOA|
|`GPIO_Pin`|Pin or combination of pins|
|`PinState`|`GPIO_PIN_SET` or `GPIO_PIN_RESET`|

Example:

```
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
```

This sets PA1 HIGH.

### 14.2. `HAL_Delay()`

General form:

```
HAL_Delay(milliseconds);
```

Example:

```
HAL_Delay(10);
```

This requests a blocking delay of approximately 10 milliseconds using the HAL time base.

While the function is waiting, the CPU does not continue executing the next statements in the same flow.

### 14.3. GPIO Ports and Pins

A GPIO port groups several pins together.

For example:

```
GPIOA
```

identifies Port A, while:

```
GPIO_PIN_0
```

identifies pin 0 within that port.

The complete hardware pin is therefore PA0.

### 14.4. Hexadecimal Values

LCD commands are often written in hexadecimal.

The prefix `0x` indicates a hexadecimal integer literal in C.

Examples:

```
0x01
0x02
0x06
0x0C
0x28
```

Hexadecimal is convenient for working with bit fields because each hexadecimal digit represents exactly four binary bits.

---

## 15. Build and Flash Process

After completing the firmware, we built the project in STM32CubeIDE.

The project initially encountered a linker configuration issue because the linker script path pointed to the project directory instead of the actual linker script file.

The issue was resolved by selecting:

```
STM32F103C8TX_FLASH.ld
```

After cleaning and rebuilding, the build completed successfully.

The firmware was then programmed using ST-Link.

The programming process reported a recognized target device and successful download and verification.

### General Workflow

1. Save the source files.
    
2. Build the project.
    
3. Resolve any compiler or linker errors.
    
4. Connect the ST-Link and target board.
    
5. Download the firmware.
    
6. Verify the result on the physical LCD.
    

A successful build only proves that the project compiled and linked. Hardware testing is still required to confirm that the wiring and firmware behave correctly.

---

## 16. Troubleshooting Notes

### Problem: LCD Is Blank

Check:

- Is VDD connected to the correct supply voltage?
    
- Is VSS connected to GND?
    
- Is the STM32 ground connected to LCD ground?
    
- Is the contrast voltage on VO appropriate?
    
- Are RS, E, and D4–D7 connected to the correct GPIO pins?
    
- Was `LCD_Init()` called before `LCD_Print()`?
    

### Problem: Dark Blocks Appear

Dark blocks can indicate that the LCD is powered but has not been initialized correctly, although the exact appearance depends on the module and contrast setting.

Check the initialization sequence, data wiring, and contrast.

### Problem: Text Is Present but Difficult to Read

Check contrast first.

In our experiment, using a 2 kΩ resistor from VO to GND improved readability. A potentiometer is the more adjustable solution.

Also check whether the backlight is connected. Backlight brightness and character contrast are separate properties.

### Problem: Incorrect or Garbled Characters

Check:

- D4–D7 wiring order.
    
- The order of the upper and lower nibbles.
    
- RS and E connections.
    
- Initialization timing.
    
- The LCD controller's compatibility with the selected initialization sequence.
    

### Problem: Build Fails at the Linker Stage

Check the linker script configuration and confirm that the selected `.ld` file belongs to the target MCU and is a valid file path.

---

## 17. What We Learned

The most important concepts from this lesson are:

1. A character LCD can be controlled directly through GPIO pins.
    
2. The 4-bit interface reduces the number of required data pins.
    
3. RS determines whether a value is interpreted as a command or character data.
    
4. E provides the timing pulse used to latch incoming data.
    
5. An 8-bit value is transmitted as two four-bit nibbles.
    
6. Bitwise operations such as `&` and `>>` are useful for manipulating data.
    
7. LCD initialization must happen before sending ordinary text.
    
8. HAL functions simplify GPIO control and timing.
    
9. Hardware problems such as poor contrast can remain even when firmware is correct.
    
10. Successful firmware compilation and successful hardware operation are two separate milestones.
    

---

## 18. Suggested Exercises

Complete these exercises before moving on to a more advanced LCD driver.

- Change the displayed text to `"STM32 is running"`.
    
- Display `"Hello!"` instead of `"Hello, STM32!"`.
    
- Experiment with different contrast settings without changing the GPIO mapping.
    
- Explain the difference between `LCD_SendCommand()` and `LCD_SendData()`.
    
- Explain why each byte is sent in two parts.
    
- Change the display-control command to show the cursor, consulting the LCD command table.
    
- Investigate how to position the cursor on the second row.
    
- Refactor the driver so the LCD functions can be reused from a separate `.c` and `.h` file.
    

## 19. Next Lesson

A logical next step is to improve the LCD driver with reusable functions such as:

- `LCD_SetCursor(row, column)`
    
- `LCD_Clear()`
    
- `LCD_PrintAt(row, column, text)`
    

This will allow us to display different messages at specific positions instead of writing only from the initial cursor position.

After that, we can explore a cleaner driver structure and reduce unnecessary blocking delays.

---

## Quick Reference

|Function|Purpose|
|---|---|
|`LCD_Enable()`|Generates the Enable pulse|
|`LCD_Send4Bits()`|Sends one nibble over D4–D7|
|`LCD_SendCommand()`|Sends an LCD command|
|`LCD_SendData()`|Sends one character byte|
|`LCD_Init()`|Initializes the LCD|
|`LCD_Print()`|Displays a null-terminated string|

**Project result:** The STM32F103C8T6 successfully displayed `Hello, STM32!` on the 16x2 LCD using a GPIO-based 4-bit interface and STM32 HAL.