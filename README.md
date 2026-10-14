
# STM32 Metronome Project

This repo contains the code I wrote and used for my CMPE3001 Embedded Systems Engineering unit at university. Here are some notes regarding the project: 

## Utilised Hardware

1. Utilised MCU - STM32L476RG (Nucleo board)
2. External modules/peripherals - keypad matrix, I2C 16x2 LCD, passive piezoelectric buzzer, servo motor and various buttons & LEDs  

## Design Requirements

The outline of the project for this unit stated that the STM32 nucleo board must be used to interface with the various sensors/actuators on the development board. These sensors/actuators included:
1. Keypad matrix
2. I2C 16x2 LCD
3. Heart rate sensor
4. 6-DOF IMU sensor
5. Servo motor
6. Array of LEDs & pushbuttons

The project design had to meet the following criteria:
1. Utilise the STM32 CubeIDE for programming
2. Take input from min. 2 input devices
3. Output to min. 2 output devices
4. Have appropriate logic design
5. Utilise modular programming practices like functions
6. Have well-documented source code

## Choice of Project

With the design criteria in mind, my group settled on a digital metronome, with the following features:
- Takes user input for BPM with keypad 
- Choice of three outputs: servo, LED array and buzzer for indicating a beat, with onboard switch utilised to toggle between the 3
- Displays current BPM, tempo & output mode on LCD

## Design Choices

* Utilises multiple timers - one for overall timekeeping & generating the beat, one for servo PWM, one for buzzer signal
* External interrupts - for onboard pushbutton & keypad matrix to avoid polling
* Systick timer - for debouncing switches, refreshing LCD and servo movement to avoid using blocking code
* Functions - for repetitive code blocks & ISRs

## Prototyping

![images/prototype.jpg]

## Result

![images/final.jpg]
