Arduino code and hardware to control an modified PayMatic coin box.

# Introduction
I was asked to repair a broken PayMatic Pico Timer, since it has suffered from
a voltage overload (it was connected to 380V instead of 230V). The replacement
unit typically used for this matter was roughly 150-200 EUR. 

It contains the following functions:
* It triggering a switch when an coin is inserted, allowing the user to use the
  attached device for a specific amount of time.
* Press the button to allow the machine to be used for a short amount of time.
  A washmachine has for example an electronically controlled door, without any
  power the door cannot be opened.
* Allow the administrator to set both times by inserted an key.

The project itself took roughly 20 hours to complete and since parts are
shipped overseas it took roughly 6 months to get all the parts required.


# Bill of Materials
The replacement parts together cost roughly 15 EUR.

## Hardware
* NEW UNO R3 ATmega328P CH340 Mini USB Board for Compatible-Arduino [4 EUR]
* New DC5V 3.8A 20W Universal Regulated Switching Power Supply Adapter AC 100-265V [4 EUR]
* 5V 2 Two Channel Relay Module With optocoupler for Arduino PIC ARM DSP AVR [2 EUR]
* TM1637 4 Bits Digital Tube LED Display Module With Clock Display For Arduino NEW  [2 EUR]

## Consumables
A look items from the following (bulk) consumables:
* 328 Pcs Assorted Heat Shrink Tube 5 Colors 8 Sizes Tubing Wrap Sleeve
* 0.3 MM 63/37Rosin Roll Core Solder Wire Tin/Lead Flux Solder Welding Iron Reel
* 1500pcs Bootlace Ferrules Kit (Non-insulated + Single + Double Entry)0.5-4mm2
* 200pcs 5x40pcs Dupont Cable 20cm 2.54mm 1p to 1p Female to Male jumper wire
* double-sided tape
* tywraps

## Tools
* Soldering Iron
* PCB Cell Phone Circuit Board Repair Holder Kit Universal Rework Station
* VC99+ 5999 auto range DMM multimeter tester temp R C frq buzz AC DC vs FLUKE 17B
* AWG23-10 Self Adjusting Ratcheting Ferrule Crimper tool HSC8 6-4A 0.25-6mm²
* BEST-109 Electrical Wire Cable Scissor Flush Cutter Stripper Diagonal Plier Tool


# Walk-through
This is a high-level walk-through of the steps done to repair it. Since it
involves handling high-currents which is *DANGEROUS*, I assumes you are clever a
enough to know the steps in between.

1. Remove the old Pico Timer module
2. Solder dupont headers to the tick sensor cables.
3. Alter PCB to make give all buttons an distict function, this require cutting.
   the PCB at one place, relocating one header and adding one hack wire.
4. Alter the wire-ribbon to add dupont headers to proper outputs.
5. Secure items with double-sided tape.
6. Compile and load the arduino code.
7. Test Unit and finish assembly.


# Lessons learned
## 7 Segment displays for Arduino
The are sold in two type, with shift-bit-registers and with a chip like a
TM1637, MAX7xxx. The shift-bit-register types requires constant updating by the
Arduino to keep them lit which make them rather useless if you like to do
other stuff with the arduino as well.

## Pin bouncing
When you press a button it sometimes registers multiple times adding a
debouncing function quickly elimiate this anoying issue.

## Interrupt pins
Most ideally you would like to use interrupt pins to trigger an button update
on the arduino code, this avoids useless polling of the pins all the time.
How-ever due to the limited amount of interrupt pins available this might not
always be possible.

## Power Supply Adapter
Measure voltage if the load is connected. I have had two broken ones shipped on
which the voltage dropped to a level of around 3.8V causing all kind of weird
behaviour on 5V line devices. If a power supply is making high-pitch noises
under load be alarmed.

## Double-Sided tape is bad
Since it was all secured by double-sided industrial tape, replacing a broken
module/PCB/power supply is annoying and a lot of work. For my next poject I
would use some (plastic) plate and spacers to put the components on.

## Pin HIGH or LOW
I am still not sure what is the best approch. Since I like to keep things
simple I follow the principle of making the pin LOW if a button is pressed,
as this seems common practice among PCB designing. I am guessing this is done
to avoid putting voltages on wires which dispensate heat and can cause
interference.
