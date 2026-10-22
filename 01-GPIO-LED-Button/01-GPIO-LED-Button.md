---
title: "STM32 GPIO: Blinking an LED and Reading a Button"
aliases:
  - STM32 LED and Button
  - STM32 GPIO Registers
tags:
  - STM32
  - STM32F103C8T6
  - Blue-Pill
  - GPIO
  - LED
  - Button
  - C
  - Registers
---

# STM32 GPIO: Blinking an LED and Reading a Button

> [!abstract] Learning goals
> This note explains how to control the onboard LED and read a push button on the STM32 Blue Pill using direct register access. It covers GPIO clocks, GPIO configuration registers, bitwise operations, input pull-ups, the active-low LED, and how to test the program.

## Table of Contents

- [[#1. Hardware and Wiring]]
- [[#2. The Complete Program]]
- [[#3. What Happens Before main]]
- [[#4. Enabling GPIO Clocks]]
- [[#5. How GPIO Registers Work]]
- [[#6. Configuring PA0 as a Button Input]]
- [[#7. Reading the Button with IDR]]
- [[#8. Configuring PC13 as an LED Output]]
- [[#9. Why the LED Is Active-Low]]
- [[#10. The Main Loop]]
- [[#11. Bitwise Operators Used in the Program]]
- [[#12. Build Flash and Test]]
- [[#13. Common Mistakes]]
- [[#14. Exercises]]
- [[#15. Summary]]

---

## 1. Hardware and Wiring

This exercise uses:

- STM32 Blue Pill board
- STM32F103C8T6 microcontroller
- A push button
- The onboard LED, usually connected to PC13
- STM32CubeIDE

### Button wiring

Connect one terminal of the push button to `PA0` and the other terminal to `GND`.

We will configure PA0 as an input with an internal pull-up resistor, so an external pull-up resistor is not needed for this simple exercise.

### LED

On a typical Blue Pill board, the onboard LED is connected to `PC13` and is **active-low**:

- PC13 = `0` → LED ON
- PC13 = `1` → LED OFF

> [!note] Board variations
> Some Blue Pill-compatible boards may differ. If your onboard LED is wired differently, verify the board schematic or pinout.

---

## 2. The Complete Program

This program turns the LED on while the button is pressed and turns it off when the button is released.

```c
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    // Enable GPIOA and GPIOC clocks
    RCC->APB2ENR |= (1U << 2) | (1U << 4);

    // Configure PA0 as input with pull-up
    GPIOA->CRL &= ~(0xFU << 0);
    GPIOA->CRL |=  (0x8U << 0);
    GPIOA->ODR |=  (1U << 0);

    // Configure PC13 as 2 MHz push-pull output
    GPIOC->CRH &= ~(0xFU << 20);
    GPIOC->CRH |=  (0x2U << 20);

    // LED off initially (PC13 is active-low)
    GPIOC->BSRR = (1U << 13);

    while (1)
    {
        if ((GPIOA->IDR & (1U << 0)) == 0)
        {
            // Button pressed: LED on
            GPIOC->BRR = (1U << 13);
        }
        else
        {
            // Button released: LED off
            GPIOC->BSRR = (1U << 13);
        }
    }
}
```

> [!important] Where does this code go?
> Place the code in the `main.c` file in the existing CubeIDE project, preserving the project-generated functions such as `SystemClock_Config()`. The STM32 device header is normally included through the project's generated headers. Do not put `#include "stm32f1xx.h"` inside `main()`.

---

## 3. What Happens Before `main`?

```c
HAL_Init();
SystemClock_Config();
```

### `HAL_Init()`

Initializes the STM32 HAL library and its basic services, including the HAL time base and low-level setup expected by the project.

### `SystemClock_Config()`

Configures the microcontroller's system clock according to the generated project configuration.

The code then uses direct register access for GPIO. This means we are not using functions such as `HAL_GPIO_ReadPin()` or `HAL_GPIO_WritePin()` for the button and LED.

> [!note]
> Using HAL for initialization and direct register access for GPIO is possible. For a learning exercise, it is useful because it exposes how the hardware registers work.

---

## 4. Enabling GPIO Clocks

The first register operation is:

```c
RCC->APB2ENR |= (1U << 2) | (1U << 4);
```

### What is RCC?

RCC stands for **Reset and Clock Control**. It controls clock distribution and several reset-related functions in the STM32.

A peripheral generally needs its clock enabled before its registers can be used normally.

### What is APB2ENR?

`APB2ENR` is the APB2 peripheral clock enable register. On the STM32F103, it contains enable bits for peripherals such as GPIOA and GPIOC.

The expression:

```c
(1U << 2)
```

sets bit 2, which enables the GPIOA clock.

The expression:

```c
(1U << 4)
```

sets bit 4, which enables the GPIOC clock.

The `|` operator combines the two masks, and `|=` sets those bits without clearing the other bits in the register.

### Why not assign a plain number?

Using:

```c
RCC->APB2ENR = (1U << 2) | (1U << 4);
```

would overwrite the other bits of the register. Using `|=` preserves their current values while setting the required enable bits.

---

## 5. How GPIO Registers Work

The STM32 peripherals are controlled through **memory-mapped registers**. These registers occupy defined addresses in the processor's address space.

For GPIO ports on the STM32F103, some important registers are:

| Register | Purpose |
|---|---|
| `CRL` | Configuration for pins 0–7 |
| `CRH` | Configuration for pins 8–15 |
| `IDR` | Reads the current input logic levels |
| `ODR` | Holds output data and selects pull-up/pull-down for input mode |
| `BSRR` | Sets or resets output bits atomically |
| `BRR` | Resets output bits |

Each GPIO pin has a four-bit configuration field in `CRL` or `CRH`.

### Why are fields four bits wide?

In the STM32F1 GPIO configuration model, each pin's four-bit field consists of:

- Two `MODE` bits
- Two `CNF` bits

Together they determine whether the pin is an input or output and, depending on the mode, the input or output configuration.

For example:

- PA0 uses bits 3:0 of `GPIOA->CRL`.
- PC13 uses bits 23:20 of `GPIOC->CRH`.

---

## 6. Configuring PA0 as a Button Input

The relevant code is:

```c
GPIOA->CRL &= ~(0xFU << 0);
GPIOA->CRL |=  (0x8U << 0);
GPIOA->ODR |=  (1U << 0);
```

Let's examine each line.

### 6.1. Clear PA0's four-bit configuration field

```c
GPIOA->CRL &= ~(0xFU << 0);
```

`0xF` is hexadecimal for decimal 15, or binary `1111`.

Shifting it left by zero leaves it unchanged:

```text
0000 0000 0000 0000 0000 0000 0000 1111
```

The `~` operator inverts the mask. In a 32-bit register, the result has zeros in the lowest four bits and ones elsewhere.

Applying `&=` clears only the four-bit field for PA0 while preserving all other bits in `CRL`.

Why clear the field first? Because OR can change a zero bit to one, but it cannot change an existing one to zero. Clearing the field first ensures that the new configuration is applied cleanly.

### 6.2. Set PA0 to input with pull-up/pull-down

```c
GPIOA->CRL |= (0x8U << 0);
```

`0x8` is binary `1000`. For STM32F1 GPIO configuration, the PA0 field becomes:

```text
CNF = 10
MODE = 00
```

This selects input mode with pull-up/pull-down.

The pin's pull direction is then selected through the corresponding bit in `ODR`.

### 6.3. Select pull-up

```c
GPIOA->ODR |= (1U << 0);
```

This sets bit 0 in `ODR`. In input pull-up/pull-down mode:

- ODR bit = 1 → pull-up
- ODR bit = 0 → pull-down

With the pull-up selected, PA0 normally reads as logic `1` while the button is released. Pressing the button connects PA0 to GND, making it read as logic `0`.

### Button state table

| Button state | Electrical connection | PA0 / IDR bit 0 |
|---|---|---:|
| Released | Internal pull-up holds PA0 high | 1 |
| Pressed | PA0 connected to GND | 0 |

---

## 7. Reading the Button with IDR

The condition inside the main loop is:

```c
(GPIOA->IDR & (1U << 0)) == 0
```

### What is IDR?

`IDR` means **Input Data Register**. It reports the current logic level of the GPIO input pins.

To examine only PA0, we use a bit mask:

```c
1U << 0
```

This produces the mask:

```text
00000001
```

The `&` operator keeps only the bit selected by the mask.

Example when PA0 is high:

```text
IDR:       00000101
Mask:      00000001
           --------
IDR & Mask 00000001  -> result is 1
```

Example when PA0 is low:

```text
IDR:       00000100
Mask:      00000001
           --------
IDR & Mask 00000000  -> result is 0
```

The condition `== 0` therefore means: “Is PA0 currently low?”

Because the button connects PA0 to GND when pressed, a low input means that the button is pressed.

---

## 8. Configuring PC13 as an LED Output

The configuration code is:

```c
GPIOC->CRH &= ~(0xFU << 20);
GPIOC->CRH |=  (0x2U << 20);
```

### 8.1. Why CRH?

`CRL` configures pins 0–7, while `CRH` configures pins 8–15. Since PC13 is in the range 8–15, we configure it through `GPIOC->CRH`.

### 8.2. Why bit 20?

Each pin has a four-bit field. In `CRH`, pin 8 starts at bit 0:

- PC8 uses bits 3:0
- PC9 uses bits 7:4
- PC10 uses bits 11:8
- PC11 uses bits 15:12
- PC12 uses bits 19:16
- PC13 uses bits 23:20

Therefore, the shift amount for PC13 is:

```c
(13 - 8) * 4 = 20
```

### 8.3. What does `0x2` mean?

`0x2` is binary `0010`. In the STM32F1 GPIO configuration field:

```text
CNF = 00
MODE = 10
```

This configures PC13 as a general-purpose push-pull output with a maximum output speed setting of 2 MHz.

The “2 MHz” value is an output speed/slew-rate configuration. It is **not** the LED's blinking frequency.

---

## 9. Why the LED Is Active-Low

On a typical Blue Pill, the onboard LED is wired so that driving PC13 low turns it on.

The program initially executes:

```c
GPIOC->BSRR = (1U << 13);
```

This sets output bit 13, making PC13 high and turning the LED off.

To turn the LED on, the code uses:

```c
GPIOC->BRR = (1U << 13);
```

This resets output bit 13, making PC13 low and turning the LED on.

### BSRR and BRR

`BSRR` stands for **Bit Set/Reset Register**.

- Bits 0–15 of BSRR set the corresponding output bits.
- Bits 16–31 reset the corresponding output bits.

`BRR` is the bit reset register; writing a one to a bit position resets the corresponding output bit.

These registers are useful because they allow individual output bits to be set or reset without performing a read-modify-write operation on the entire `ODR` register.

### LED state table

| PC13 output | LED state |
|---:|---|
| 0 | ON |
| 1 | OFF |

This is the opposite of an active-high LED, where a high output would turn the LED on.

---

## 10. The Main Loop

The program repeatedly checks the button:

```c
while (1)
{
    if ((GPIOA->IDR & (1U << 0)) == 0)
    {
        GPIOC->BRR = (1U << 13);
    }
    else
    {
        GPIOC->BSRR = (1U << 13);
    }
}
```

### When the button is pressed

1. PA0 is connected to GND.
2. Bit 0 of `IDR` reads as zero.
3. The condition `== 0` is true.
4. `GPIOC->BRR = (1U << 13)` makes PC13 low.
5. The onboard LED turns on.

### When the button is released

1. The internal pull-up holds PA0 high.
2. Bit 0 of `IDR` reads as one.
3. The condition `== 0` is false.
4. The `else` block runs.
5. `GPIOC->BSRR = (1U << 13)` makes PC13 high.
6. The onboard LED turns off.

The loop executes continuously, so the LED follows the current button state.

> [!note] Button debouncing
> Mechanical buttons can produce several rapid transitions when pressed or released. This simple program does not implement debouncing. For a basic level-sensitive LED exercise, it may be sufficient, but reliable event counting or more complex input handling often requires debouncing.

---

## 11. Bitwise Operators Used in the Program

### `<<` — Left shift

```c
1U << 4
```

Moves the bit pattern of the value `1U` four positions to the left, producing a value with bit 4 set.

### `&` — Bitwise AND

```c
GPIOA->IDR & (1U << 0)
```

Keeps only the bits that are set in both operands. Here, it isolates PA0's input bit.

### `|` — Bitwise OR

```c
(1U << 2) | (1U << 4)
```

Combines two masks, setting bit 2 and bit 4 in the result.

### `~` — Bitwise NOT

```c
~(0xFU << 0)
```

Inverts every bit in the mask. This creates a mask that can clear the selected field when used with AND.

### `|=` — OR assignment

```c
GPIOA->ODR |= (1U << 0);
```

Equivalent in concept to:

```c
GPIOA->ODR = GPIOA->ODR | (1U << 0);
```

It sets the selected bit while preserving other bits in the register.

### `&=` — AND assignment

```c
GPIOA->CRL &= ~(0xFU << 0);
```

Equivalent in concept to:

```c
GPIOA->CRL = GPIOA->CRL & ~(0xFU << 0);
```

It clears the selected field while preserving other bits.

---

## 12. Build, Flash, and Test

### Step 1: Build

In STM32CubeIDE, use **Project → Build Project** or press `Ctrl+B`.

Resolve any compiler or linker errors before attempting to program the board.

### Step 2: Flash and run

Start a Debug session or use the programming workflow configured for your project. If using Debug, the IDE may program the board and halt the CPU at the start of the session.

Use Resume (often `F8`) to continue execution when the debugger is halted.

### Step 3: Test the button

- Release the button: the LED should be off.
- Press the button: the LED should turn on.
- Release it again: the LED should turn off.

If the LED behaves in the opposite way, check whether your board uses the typical active-low PC13 LED wiring.

---

## 13. Common Mistakes

### Mistake 1: Forgetting to enable the GPIO clock

Without enabling the peripheral clock, GPIO register access may not behave as expected.

### Mistake 2: Using CRL for PC13

PC13 is configured through `CRH`, not `CRL`, because CRL only covers pins 0–7.

### Mistake 3: Forgetting to clear a configuration field

OR-ing a new value into an old field may leave unwanted bits set. Clear the field first, then set the desired configuration.

### Mistake 4: Assuming a pressed button always reads as 1

That depends on the wiring. With our pull-up and button-to-GND circuit, a pressed button reads as 0.

### Mistake 5: Assuming a high output always turns the LED on

The onboard PC13 LED on a typical Blue Pill is active-low: low means ON, high means OFF.

### Mistake 6: Confusing GPIO output speed with blink frequency

The `MODE = 10` configuration selects a 2 MHz maximum output speed setting. It does not make the LED blink at 2 MHz.

### Mistake 7: Including the device header inside `main()`

The project's generated headers normally include the appropriate STM32 device definitions. Do not put `#include "stm32f1xx.h"` inside the body of `main()`.

---

## 14. Exercises

### Exercise 1: Predict the LED state

What happens after this line executes?

```c
GPIOC->BSRR = (1U << 13);
```

<details>
<summary>Answer</summary>

PC13 is set high. On a typical Blue Pill with an active-low PC13 LED, the LED turns off.
</details>

### Exercise 2: Predict the LED state

What happens after this line executes?

```c
GPIOC->BRR = (1U << 13);
```

<details>
<summary>Answer</summary>

PC13 is driven low, so the active-low onboard LED turns on.
</details>

### Exercise 3: Read the button

Suppose `GPIOA->IDR` is `0x00000001`. What happens in this condition?

```c
if ((GPIOA->IDR & (1U << 0)) == 0)
{
    // LED ON
}
else
{
    // LED OFF
}
```

<details>
<summary>Answer</summary>

Bit 0 is 1, so the condition is false and the `else` block runs. The LED is turned off.
</details>

### Exercise 4: Change the button pin

If the button were connected to PA1 instead of PA0, which mask would isolate its input bit?

<details>
<summary>Answer</summary>

Use `1U << 1` instead of `1U << 0`. The GPIO configuration field and pull-up selection would also need to be changed to PA1's bit positions.
</details>

### Exercise 5: Why clear before setting?

Why do we use this pattern?

```c
GPIOA->CRL &= ~(0xFU << 0);
GPIOA->CRL |=  (0x8U << 0);
```

<details>
<summary>Answer</summary>

The first line clears the four-bit PA0 configuration field. The second line writes the desired configuration. Without clearing first, old one-bits could remain set because OR cannot turn a bit from 1 to 0.
</details>

---

## 15. Summary

- `RCC->APB2ENR` enables the GPIO peripheral clocks.
- `GPIOA->CRL` configures PA0 because PA0 is in the pin range 0–7.
- `GPIOA->ODR` selects the internal pull-up for PA0 in input pull-up/pull-down mode.
- `GPIOA->IDR` reads the current input level.
- `GPIOC->CRH` configures PC13 because PC13 is in the pin range 8–15.
- `GPIOC->BRR` drives PC13 low, turning the typical active-low onboard LED on.
- `GPIOC->BSRR` sets PC13 high, turning that LED off.
- `&`, `|`, `~`, `<<`, `&=`, and `|=` are used to manipulate individual bits and fields in registers.
- The program uses direct register access for GPIO while retaining the project's HAL and clock initialization.

The key idea is to understand each register operation as a controlled change to a specific hardware bit or field, rather than treating the code as a collection of magic numbers.
