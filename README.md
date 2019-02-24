# STM32 for Abot
## introduction
There are two stm32 programs used on stm32f103 which designed to control `Abot`. `Abot` is a 4-DOF robot arm design by me.
The whole robot system is consist of upper computer or upper computer software and lower computer system. I used Raspberry-Pi as the upper computer. The lower computer system consist of 5 stm32 micro-controllers. Controllers use CAN bus to communicate and work in master-slave mode. There are one Master and 4 Slaves.
## functions
- Master:
Master use serial port to communicate with the upper computer or upper computer software. All the motion information is come from the upper computer or upper computer software.
