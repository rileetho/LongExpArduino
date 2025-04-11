# LongExpArduino
Repository for my long experiment arduino sketches<br/>
<br/>
**Goal:** Create a system that records temperature, displacement and time every 30 sec, and records these onto an SD card that can be ejected and reinserted at the flick of a switch. LCD screen to display current disp and temp during the experiment.<br/>
<br/>
*Includes:*<br/>
1. **rtc-example**: Example 1 sec interrupt using UNO R4 Minima onboard RTC<br/>
2. **sketch_final_2.0**: Long experiment master sketch<br/>
3. **sd_end_test_2**: Testing SD.end() and SD.begin() for ejecting and re-inserting SD cards by on/off switch<br/>
4. **rtc_int_test**: Testing interrupts using external RTC PCF8523<br/>
5. **rtc_int_30s_test**: Testing interrupts using external RTC + write out every 30 sec<br/>
6. **sd_rtc_test**: SD card + switch to eject/re-initialise + RTC (no interrupts), logging time every 5 sec<br/>
7. **rtc_int_30s_sd_switch**: Combined SD card + switch on/off + RTC + interrupts + log every 30 sec<br/>
