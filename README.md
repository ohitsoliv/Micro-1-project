# 3D Snek :3

**EE3463 Final Project — UTSA, Team 4**
Justino Garza · Oliv Fleming

## Project Description

A classic Snake game running on an ATmega328P, displayed on a two-layer 8×8
bicolor LED matrix (red + green). The player controls the snake with four
directional push buttons. The snake body is drawn in green; treats appear in
red.

**Layer 0 (bottom):** Eat 5 treats to advance. Game speed is ~400 ms per tick.

**Layer 1 (top):** Eat 8 treats to win. Game speed increases to ~250 ms per tick.

Wall behavior is wrap-around (the snake exits one side and reappears on the
opposite). Self-collision ends the game.

## Hardware

| Component               | Qty | Notes                                |
|--------------------------|-----|--------------------------------------|
| ATmega328P (UTSA board)  | 1   | Running at 16 MHz                    |
| 8×8 bicolor LED matrix   | 2   | Common cathode, stacked as 2 layers  | //fill in here or one
| 74HC595 shift register   | 3   | Daisy-chained for rows + columns     |
| Push buttons             | 4   | Up, Down, Left, Right                |
| Resistors (220 Ω)        | 16  | Current limiting for LED columns     |
| Hookup wire              | —   |                                      |

## Wiring Summary

### Buttons (active-low, internal pull-ups enabled)

| Button | ATmega Pin |
|--------|-----------|
| Up     | PD2       | //fill in 
| Down   | PD3       | //fill in 
| Left   | PD4       | //fill in 
| Right  | PD5       | //fill in

### Shift Registers (74HC595 × 3, daisy-chained)

| Signal | ATmega Pin | Description            |
|--------|-----------|------------------------|
| SER    | PB0       | Serial data in         |
| SRCLK  | PB1       | Shift clock            |
| RCLK   | PB2       | Latch (storage clock)  |

Chain order: ATmega → SR1 (row select) → SR2 (green columns) → SR3 (red columns)

### Layer Select

| Signal       | ATmega Pin | Description                  |
|--------------|-----------|------------------------------|
| Layer select | PB3       | LOW = layer 0, HIGH = layer 1 |

### RNG Seed

PC0 (ADC0) is left floating and read once at startup to seed the random
number generator.

## Building

### Prerequisites

Install the AVR toolchain:

**Windows (MSYS2):**
```
pacman -S mingw-w64-x86_64-avr-gcc mingw-w64-x86_64-avr-libc mingw-w64-x86_64-avrdude
```

**macOS (Homebrew):**
```
brew install avr-gcc avrdude
```

Verify with:
```
avr-gcc --version
avrdude -?
```

### Compile

```
make
```

### Flash

Edit the `PROGRAMMER` line in `Makefile` to match your programmer, then:

```
make flash
```

### Clean

```
make clean
```

## File Structure

```
snake-game/
├── include/
│   ├── config.h    — Pin definitions, constants, timer values
│   ├── game.h      — Game state structs, enums, public API
│   ├── input.h     — Button reading / debounce API
│   ├── display.h   — LED matrix driver API
│   └── timer.h     — Timer init, game_tick_flag
├── src/
│   ├── main.c      — Main loop and state machine
│   ├── game.c      — Snake logic, collision, treats, layers
│   ├── input.c     — Debounced button sampling (Timer0 ISR)
│   ├── display.c   — Shift register mux (Timer2 ISR), animations
│   └── timer.c     — Timer0/1/2 configuration, game tick ISR
├── Makefile
├── .gitignore
└── README.md
```
