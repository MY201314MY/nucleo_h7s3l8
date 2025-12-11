###### witth mcuboot

```shell
west build -b nucleo_h7s3l8 --sysbuild -p

-- Build files have been written to: /home/pi/example/zephyr/nucleo_h7s3l8/with_mcuboot/build/with_mcuboot
[145/145] Linking C executable zephyr/zephyr.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:       19964 B     523904 B      3.81%
             RAM:        4672 B       456 KB      1.00%
           SRAM0:          0 GB       456 KB      0.00%
           SRAM1:          0 GB        16 KB      0.00%
            DTCM:          0 GB       128 KB      0.00%
            ITCM:          0 GB        64 KB      0.00%
          EXTMEM:          0 GB        32 MB      0.00%
        IDT_LIST:          0 GB        32 KB      0.00%
        
 
-- Build files have been written to: /home/pi/example/zephyr/nucleo_h7s3l8/with_mcuboot/build/mcuboot
[280/280] Linking C executable zephyr/zephyr.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:       40616 B        64 KB     61.98%
             RAM:       87168 B       456 KB     18.67%
           SRAM0:          0 GB       456 KB      0.00%
           SRAM1:          0 GB        16 KB      0.00%
            DTCM:          0 GB       128 KB      0.00%
            ITCM:          0 GB        64 KB      0.00%
          EXTMEM:          0 GB        32 MB      0.00%
        IDT_LIST:          0 GB        32 KB      0.00%

```



# openocd

## server

```shell
openocd -f ~/zephyr/zephyr/boards/st/nucleo_h7s3l8/support/openocd.cfg 
Open On-Chip Debugger 0.12.0-00033-g0de861e21 (2025-11-22-18:52) [https://github.com/STMicroelectronics/OpenOCD]
Licensed under GNU GPL v2
For bug reports, read
	http://openocd.org/doc/doxygen/bugs.html
init
Info : Listening on port 6666 for tcl connections
Info : Listening on port 4444 for telnet connections
Info : STLINK V3J15M6 (API v3) VID:PID 0483:3754
Info : Target voltage: 3.287408
Info : Unable to match requested speed 1800 kHz, using 1000 kHz
Info : Unable to match requested speed 1800 kHz, using 1000 kHz
Info : clock speed 1000 kHz
Info : stlink_dap_op_connect(connect)
Info : SWD DPIDR 0x6ba02477
Info : [STM32H7S3XX.cpu0] Cortex-M7 r1p2 processor detected
Info : [STM32H7S3XX.cpu0] target has 8 breakpoints, 4 watchpoints
Info : gdb port disabled
Info : starting gdb server for STM32H7S3XX.cpu0 on 3333
Info : Listening on port 3333 for gdb connections
[STM32H7S3XX.cpu0] halted due to debug-request, current mode: Thread 
xPSR: 0x01000000 pc: 0x08001558 msp: 0x24015440
Info : accepting 'gdb' connection on tcp/3333
Debugger attaching: halting execution
force hard breakpoints
Info : Device: STM32H7Rx/7Sx
Info : flash size probed value 65535k
Info : STM32H7 flash has a single bank
Info : Bank (0) size is 65535 kb, base address is 0x08000000
Info : New GDB Connection: 1, Target STM32H7S3XX.cpu0, state: halted
Warn : Prefer GDB command "target extended-remote :3333" instead of "target remote :3333"
```

## client

```shell
/home/pi/opt/zephyr-sdk-0.17.4/arm-zephyr-eabi/bin/arm-zephyr-eabi-gdb ~/example/zephyr/nucleo_h7s3l8/with_mcuboot/build/with_mcuboot/zephyr/zephyr.elf 
GNU gdb (Zephyr SDK 0.17.4) 12.1
Copyright (C) 2022 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "--host=x86_64-build_pc-linux-gnu --target=arm-zephyr-eabi".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://github.com/zephyrproject-rtos/sdk-ng/issues>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from /home/pi/example/zephyr/nucleo_h7s3l8/with_mcuboot/build/with_mcuboot/zephyr/zephyr.elf...
(gdb) target remote localhost:3333
Remote debugging using localhost:3333
0x08001558 in ?? ()
(gdb) c
Continuing.
^C
Program received signal SIGINT, Interrupt.
arch_cpu_idle () at /home/pi/zephyr/zephyr/arch/arm/core/cortex_m/cpu_idle.c:104
104		__enable_irq();
(gdb) 
```

partition

```c
hex(8*1024*1024+384)
hex(8*1024*1024+4096)
hex(8*1024*1024*2+4096*2)
hex(8*1024*1024*2+4096*2+64*1024)
```

CAN BUS
```c
inspiron:~$ can start can@4000a000 
starting can@4000a000

inspiron:~$ can show can@4000a000
core clock:      24000000 Hz
max bitrate:     8000000 bps
max std filters: 28
max ext filters: 8
capabilities:    normal loopback listen-only fd 
mode:            normal 
state:           error-active
rx errors:       0
tx errors:       0
timing:          sjw 1..128, prop_seg 0..0, phase_seg1 2..256, phase_seg2 2..128, prescaler 1..512
timing data:     sjw 1..16, prop_seg 0..0, phase_seg1 1..32, phase_seg2 1..16, prescaler 1..32
transceiver:     passive/none

(Need a phy.)
inspiron:~$ can send can@4000a000 010 1 2 3 4 5 6 7 8
enqueuing CAN frame #0 with standard (11-bit) CAN ID 0x010, RTR 0, CAN FD 0, BRS 0, DLC 8
failed to send CAN frame #0 (err -114)
[00:05:36.731,000] <dbg> can_mcan: can_mcan_send: Sending 8 bytes. Id: 0x10, ID type: standard
```

# pinctrl

```shell
pin functions

A02 ETH_MDIO
A05 SPI1_SCK
A06 SPI1_MISO
A07 ETH_RMII_CRS_DV
A10 USART1_RX
B05 SPI1_MOSI
G06 ETH_RMII_REF_CLK
B07 LED3
B08 I2C1_SCL
B09 I2C1_SDA 
B14 USART1_TX
C13 button
C14 LSE
C15 LSE
D00 FDCAN1_RX
D01 FDCAN1_TX 
D08 USART3_TX
D09 USART3_RX
D10 LED1
D13 LED2
F12 UCPD_ISENSE
F13 UCPD_VSENSE
G04 ETH_RMII_RXD0
G05 ETH_RMII_RXD1
G06 ETH_MDC
G11 ETH_RMII_TX_EN
G12 ETH_RMII_TXD1
G13 ETH_RMII_TXD0
H00 HSE
H01 HSE
M00 UCPD_CC1
M01 UCPD_CC2
M05 USB_HS_N
M06 USB_HS_P
M08 UCPD_INT
M09 UCPD_PWR_EN
N00 XSPI_P2_DQS0
N01 XSPI_P2_NCS1
N02 XSPI_P2_IO0
N03 XSPI_P2_IO1
N04 XSPI_P2_IO2
N05 XSPI_P2_IO3
N06 XSPI_P2_CLK
N08 XSPI_P2_IO4
N09 XSPI_P2_IO5
N10 XSPI_P2_IO6
N11 XSPI_P2_IO7
```
