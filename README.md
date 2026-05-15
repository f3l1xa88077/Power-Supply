# Overview

This project focused on developing a step-down voltage regulator based around the LM317T linear voltage regulator.

The system is powered by four 18650 Li-ion batteries and is controlled by a master rocker switch. To set the output, the user interacts with a rotary encoder, which provides input signals to an Arduino R4. The Arduino then translates these movements into commands for a digital potentiometer. This potentiometer works within a voltage divider and op-amp circuit to manipulate the voltage on the adjust pin of an LM317T regulator, effectively setting the final output. To ensure precision, an op-amp steps the output voltage down and feeds it back into the R4, creating a closed feedback loop that allows the microcontroller to monitor and accurately maintain the desired regulation.

# Components

An overview of the main components used in the project:

- [LM317T](https://au.mouser.com/ProductDetail/511-LM317T) used to regulate +VBAT to +VOUT.
- [L4941BV](https://au.mouser.com/ProductDetail/511-L4941BV) to regulate a stable 5V.
- [MCP4162-502E/P](https://au.mouser.com/ProductDetail/Microchip-Technology/MCP4162-502E-P?qs=hH%252BOa0VZEiDDRVodHevOyA%3D%3D) to vary the LM317T adjust voltage level.
- [LM358P](https://au.mouser.com/ProductDetail/595-LM358P) To step up and step down voltages (non-inverting).
- [Arduino Nano Rev4](https://au.mouser.com/ProductDetail/782-ABX00142) as the microcontroller.
- [EN11-HSM1AF20](https://au.mouser.com/ProductDetail/858-EN11-HSM1AF20) as a user interface component to control desired output voltage.

See the project file for the Bill of Materials.

# Development

## Prototyping

Include image of breadboard

## PCB

.

### Modifications

talk about solder jobs needed once I found pcb errors.

# Results

.

# Future Suggestions

.