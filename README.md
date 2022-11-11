# BarBot
Repository containing source code for a drink dispensing robot created for PIE.

Barbot is implemented using an UNO R3 microcontroller, with code written on Arduino IDE.
Here will lie the source code for the implementations ranging from Sprint 1, all the way to the final implementation
This read me will be updated as information becomes relevant.

As of Sprint 2, the BarBot will be using a Mega 2560 R3 microcontroller to enable us to use more motors.

## Dependencies
`LiquidCrystal_I2C` - To control our LCD display we are using the LiquidCrystal_I2C library. To use it, download the .zip file in this directory, and extract it to the `libraries` folder inside of the `Arduino` folder. 

`AccelStepper` - In the interest of increasing the speed performance of the stepper motors, we use AccelStepper. This library can be found in the Library Manager tab in the Arduino IDE. We used version 1.61.0 By Patrick Wasp.

## Hardware
### Motors
We are using NEMA 17 motors for this project. We are specifically using Moons' and Bohong since we were able to find those readily.
### Motor Drivers
We are using A4988 motor drivers since they are economical and effective.
###
