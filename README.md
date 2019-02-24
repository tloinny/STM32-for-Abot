# STM32 for Abot
## introduction
There are two stm32 programs used on stm32f103 which designed to control `Abot`. `Abot` is a 4-DOF robot arm design by me.
The whole robot system is consist of upper computer or upper computer software and lower computer system. I used Raspberry-Pi as the upper computer. The lower computer system consist of 5 stm32 micro-controllers. Controllers use CAN bus to communicate and work in master-slave mode. There are one Master and 4 Slaves.

## main functions
- Master:
Master using serial port to communicate with the upper computer or upper computer software. All the motion information is come from the upper computer or upper computer software. While Master receive valid motion information it will distribute the information to slaves by CAN bus immediately.
- Slave:
Slaves using CAN bus to communicate with the Master, every Slave have its distinctive ID, Master send information to all the Slave in the form of broadcasting. Slaves use its ID to filter out the useful information for itself. Then Slave use the information to control Abot's joint by driving the stepper motor.

## programing environment
IDE-Version:
uVision V5.25.2.0

## hardware requirement
- stm32 with Medium Capacity
- At least having one CAN (you can reference the [official website of stm32](https://www.stmcu.com.cn/))
- CAN transceiver(like [tja1050](https://www.alldatasheet.com/datasheet-pdf/pdf/19755/PHILIPS/TJA1050.html))