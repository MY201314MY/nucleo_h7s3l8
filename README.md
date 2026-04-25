# with mcuboot

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

# partition

```shell
# image-0
hex(8*1024*1024+384)
# image-1
hex(8*1024*1024+4096)
# image-scratch
hex(8*1024*1024*2+4096*2)
# storage
hex(8*1024*1024*2+4096*2+64*1024)
```

# can bus

```shell
#1.start can bus

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

# standard
inspiron:~$ can send can@4000a000 010 1 2 3 4 5 6 7 8
enqueuing CAN frame #11 with standard (11-bit) CAN ID 0x010, RTR 0, CAN FD 0, BRS 0, DLC 8
CAN frame #11 successfully sent
[00:21:32.098,000] <dbg> can_mcan: can_mcan_send: Sending 8 bytes. Id: 0x10, ID type: standard

# add filter
inspiron:~$ can filter add can@4000a000 123
can@4000a000  --       123   [8]  11 22 33 44 55 66 77 99 
[00:12:01.777,000] <dbg> can_mcan: can_mcan_line_1_isr: RX FIFO0 INT
[00:12:01.777,000] <dbg> can_mcan: can_mcan_get_message: Frame on filter 0, ID: 0x123

# CAN FD
inspiron:~$ can stop can@4000a000 
stopping can@4000a000
inspiron:~$ can mode can@4000a000 fd
setting mode 0x00000004
inspiron:~$ can start can@4000a000
starting can@4000a000

inspiron:~$ can send can@4000a000 -f 123 11 22
enqueuing CAN frame #7 with standard (11-bit) CAN ID 0x123, RTR 0, CAN FD 1, BRS 0, DLC 2
CAN frame #7 successfully sent
[00:16:41.168,000] <dbg> can_mcan: can_mcan_send: Sending 2 bytes. Id: 0x123, ID type: standard  FD frame 

can@4000a000  --       123  [12]  11 22 33 44 55 66 77 88 aa bb 00 00 
[00:20:13.476,000] <dbg> can_mcan: can_mcan_line_1_isr: RX FIFO0 INT
[00:20:13.476,000] <dbg> can_mcan: can_mcan_get_message: Frame on filter 0, ID: 0x123
```

# ethernet
```shell
*** Booting MCUboot v2.2.0-243-g234c66e66ee3 ***
*** Using Zephyr OS build v4.3.0-2311-gcb2109e7d924 ***

[00:00:00.051,000] <inf> phy_mii: PHY (0) ID 7C131
[00:00:00.051,000] <dbg> can_mcan: can_mcan_init: IP rel: 3.2.1 04.18.24
[00:00:00.052,000] <dbg> can_mcan: can_mcan_init: Presc: 1, TS1: 17, TS2: 6
[00:00:00.052,000] <dbg> can_mcan: can_mcan_init: Sample-point err : 0
[00:00:00.052,000] <dbg> can_mcan: can_mcan_init: Sample-point err data phase: 0
[00:00:00.052,000] <dbg> can_mcan: can_mcan_set_timing_data: TDC enabled, using TDCO 18
[00:00:00.052,000] <dbg> can_stm32fd: config_can_0_irq: Enable CAN0 IRQ
[00:00:00.052,000] <dbg> eth_stm32_hal: eth_initialize: MAC 02:80:e1:40:ec:24
[00:00:00.055,000] <dbg> eth_stm32_hal: eth_stm32_hal_stop: Stopping ETH HAL driver
[00:00:00.055,000] <dbg> eth_stm32_hal: eth_stm32_hal_stop: HAL_ETH_Stop{_IT} returned error (Ethernet is already stopped)
*** Booting Zephyr OS build v4.3.0-2311-gcb2109e7d924 ***
[00:00:00.058,000] <inf> main: app start address: 0x0x70000000
[00:00:00.058,000] <inf> main: soc : stm32h7s3xx
[00:00:00.058,000] <inf> main: board : nucleo_h7s3l8
[00:00:00.058,000] <inf> main: frequency : 600 MHz (Cortex-M7)
[00:00:01.556,000] <inf> phy_mii: PHY (0) Link speed 100 Mb, full duplex
[00:00:01.556,000] <dbg> eth_stm32_hal: eth_stm32_hal_stop: Stopping ETH HAL driver
[00:00:01.556,000] <dbg> eth_stm32_hal: eth_stm32_hal_stop: HAL_ETH_Stop{_IT} returned error (Ethernet is already stopped)
[00:00:01.556,000] <dbg> eth_stm32_hal: eth_stm32_hal_start: Starting ETH HAL driver
inspiron:~$ net ip
  ipv4  ipv6
inspiron:~$ net ipv4 
IPv4 support                              : enabled
IPv4 fragmentation support                : disabled
IPv4 conflict detection support           : disabled
Path MTU Discovery (PMTU)                 : disabled
Max number of IPv4 network interfaces in the system          : 1
Max number of unicast IPv4 addresses per network interface   : 1
Max number of multicast IPv4 addresses per network interface : 2

IPv4 addresses for interface 1 (0x24001760) (Ethernet)
====================================================
Type            State           Ref     Address
inspiron:~$ net dhcpv
  dhcpv4  dhcpv6
inspiron:~$ net dhcpv4 client s
  start  stop
inspiron:~$ net dhcpv4 client start 1
inspiron:~$ net ipv4
IPv4 support                              : enabled
IPv4 fragmentation support                : disabled
IPv4 conflict detection support           : disabled
Path MTU Discovery (PMTU)                 : disabled
Max number of IPv4 network interfaces in the system          : 1
Max number of unicast IPv4 addresses per network interface   : 1
Max number of multicast IPv4 addresses per network interface : 2

IPv4 addresses for interface 1 (0x24001760) (Ethernet)
====================================================
Type            State           Ref     Address
DHCP    preferred       1       192.168.3.32/255.255.255.0
inspiron:~$ net ping 8.8.8.8
PING 8.8.8.8
28 bytes from 8.8.8.8 to 192.168.3.32: icmp_seq=1 ttl=112 time=45.34 ms
28 bytes from 8.8.8.8 to 192.168.3.32: icmp_seq=2 ttl=112 time=45.04 ms
28 bytes from 8.8.8.8 to 192.168.3.32: icmp_seq=3 ttl=112 time=45.74 ms
inspiron:~$ net ping 192.168.3.1
PING 192.168.3.1
28 bytes from 192.168.3.1 to 192.168.3.32: icmp_seq=1 ttl=64 time=1.52 ms
28 bytes from 192.168.3.1 to 192.168.3.32: icmp_seq=2 ttl=64 time=1.44 ms
28 bytes from 192.168.3.1 to 192.168.3.32: icmp_seq=3 ttl=64 time=1.44 ms
inspiron:~$ net dns www.baidu.com
Query for 'www.baidu.com' sent.
dns: 103.235.46.115
dns: 103.235.46.102
dns: All results received
inspiron:~$ net ping 103.235.46.115
PING 103.235.46.115
28 bytes from 103.235.46.115 to 192.168.3.32: icmp_seq=1 ttl=48 time=80.45 ms
28 bytes from 103.235.46.115 to 192.168.3.32: icmp_seq=2 ttl=48 time=80.75 ms
Ping timeout
inspiron:~$ net ping 103.235.46.102
PING 103.235.46.102
28 bytes from 103.235.46.102 to 192.168.3.32: icmp_seq=1 ttl=46 time=193.77 ms
28 bytes from 103.235.46.102 to 192.168.3.32: icmp_seq=2 ttl=46 time=172.29 ms
28 bytes from 103.235.46.102 to 192.168.3.32: icmp_seq=3 ttl=46 time=196.20 ms
```

# I2C

## ATECC608A

```c
i2c bus at 100k Hz under shell

inspiron:~$ i2c read i2c@40005400 60 0
00000000: 04 11 33 43 ff ff ff ff  ff ff ff ff ff ff ff ff |..3C.... ........|
```

### cryptoauthlib

```shell
./cryptoauth_test help

Usage:
help - Display Menu
sha204 - Set Target Device to ATSHA204A
sha206 - Set Target Device to ATSHA206A
ecc108 - Set Target Device to ATECC108A
ecc204 - Set Target Device to ECC204
ta010 - Set Target Device to TA010
sha104 - Set Target Device to SHA104
sha105 - Set Target Device to SHA105
ecc508 - Set Target Device to ATECC508A
ecc608 - Set Target Device to ATECC608
info - Get the Chip Revision
sernum - Get the Chip Serial Number
readcfg - Read the Config Zone
hal - Tests hal drivers functionality
rand - Generate Some Random Numbers
lockstat - Zone Lock Status
tng - Run unit tests on TNG type part.
wpc - Run unit tests on WPC type part.
basic - Run Basic Test on Selected Device
util - Run Helper Function Tests
clkdivm0 - Set ATECC608 to ClockDivider M0(0x00)
clkdivm1 - Set ATECC608 to ClockDivider M1(0x05)
clkdivm2 - Set ATECC608 to ClockDivider M2(0x0D)
cd - Run Unit Tests on Cert Data
cio - Run Unit Test on Cert I/O
crypto - Run Unit Tests for Software Crypto Functions
calib - Run calib api tests
exit - Exit the test application
```



```shell
[00:00:52.210,000] <dbg> crypto: example_crypto_operations: operation 1 selected
[00:00:52.243,000] <inf> crypto: config
                                 01 23 87 a9 00 00 60 02  a7 d7 27 f7 ee c1 5d 00 |.#....`. ..'...].
                                 c0 00 00 00 83 20 87 20  8f 20 c4 8f 8f 8f 8f 8f |..... .  . ......
                                 9f 8f af 8f 00 00 00 00  00 00 00 00 00 00 00 00 |........ ........
                                 00 00 af 8f ff ff ff ff  00 00 00 00 ff ff ff ff |........ ........
                                 00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 |........ ........
                                 00 00 00 00 00 00 00 00  ff ff 00 00 00 00 00 00 |........ ........
                                 33 00 33 00 33 00 1c 00  1c 00 1c 00 1c 00 1c 00 |3.3.3... ........
                                 3c 00 3c 00 3c 00 3c 00  3c 00 3c 00 3c 00 1c 00 |<.<.<.<. <.<.<...
[00:00:52.318,000] <inf> crypto: slot config
                                 20 87 20 8f 20 c4 8f 8f  8f 8f 8f 9f 8f af 8f 00 | . . ... ........
                                 00 00 00 00 00 00 00 00  00 00 00 00 00 af 8f ff |........ ........
[00:00:52.340,000] <inf> crypto: key config
                                 33 00 33 00 33 00 1c 00  1c 00 1c 00 1c 00 1c 00 |3.3.3... ........
                                 3c 00 3c 00 3c 00 3c 00  3c 00 3c 00 3c 00 1c 00 |<.<.<.<. <.<.<...
```



```c
./cryptoauth_test sernum -d ecc608 -i hid i2c 0 -a 0x00

serial number:
01 23 B9 9C B4 6C A1 C6 EE

./cryptoauth_test sernum -d ecc608 -i hid i2c 0 -a 0x6A

serial number:
01 23 3F CE 99 12 44 0F 01

./cryptoauth_test sernum -d ecc608 -i hid i2c 0 -a 0x6C

serial number:
01 23 8B 26 36 5D C4 18 01
```



```shell
./cryptoauth_test readcfg -d ecc608 -i hid i2c 0 -a 0x00


01 23 B9 9C 00 00 60 03  B4 6C A1 C6 EE 61 65 00
C0 00 00 00 83 0F 83 0F  83 0F 83 0F 83 0F 83 0F
83 0F 83 0F 0F 0F 0F 0F  0F 0F 0F 0F 0F 0F 0F 0F
0F 0F 0F 0F 00 00 00 01  00 00 00 00 0F FF FF FF
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00  FF FF 00 00 00 00 00 00
33 00 33 00 33 00 33 00  33 00 33 00 33 00 33 00
3C 00 3C 00 3C 00 3C 00  3C 00 3C 00 3C 00 3C 00

./cryptoauth_test readcfg -d ecc608 -i hid i2c 0 -a 0x6A


01 23 3F CE 00 00 60 03  99 12 44 0F 01 61 55 00
6A 00 00 01 85 00 82 00  85 20 85 20 85 20 0F 8F
8F 0F 8F 8F 0F 0F 8F 0F  0F 8F 0F 8F 0F 8F 8F 8F
8F 8F 8F 8F 00 00 00 01  00 00 00 00 0F FF FF FF
00 00 00 00 00 00 00 F7  00 69 76 00 00 00 00 00
00 00 00 00 00 00 00 00  FF FF 0E 60 00 00 00 00
53 00 53 00 73 00 73 00  73 00 1C 00 7C 00 1C 00
3C 00 1A 00 1C 00 10 00  1C 00 1C 00 1C 00 1C 00

./cryptoauth_test readcfg -d ecc608 -i hid i2c 0 -a 0x6C


01 23 8B 26 00 00 60 03  36 5D C4 18 01 61 69 00
6C 00 00 01 85 00 82 00  85 20 85 20 85 20 8F 46
8F 0F 9F 8F 0F 0F 8F 0F  0F 0F 0F 0F 0F 0F 0F 0F
0D 1F 0F 0F FF FF FF FF  00 00 00 00 FF FF FF FF
00 00 00 00 00 00 03 F7  00 69 76 00 00 00 00 00
00 00 00 00 00 00 00 00  FF FF 0E 60 00 00 00 00
53 00 53 00 73 00 73 00  73 00 38 00 7C 00 1C 00
3C 00 1A 00 3C 00 30 00  3C 00 30 00 12 00 30 00
```

0x6A
```shell
01 23 3F CE 00 00 60 03  99 12 44 0F 01 61 55 00


6A(16:address) 
00 00 01 

[ device private key ] slot config 00 : 85 00 : ES : ECDH
[ device public  CA  ] slot config 01 : 82 00 : IS : NONE
[ROOT CA             ] slot config 02 : 85 20 : ES : ECDH
slot config 03 : 85 20
slot config 04 : 85 20
slot config 05 : 0F 8F
slot config 06 : 8F 0F
slot config 07 : 8F 8F
slot config 08 : 0F 0F
slot config 09 : 8F 0F
slot config 10 : 0F 8F
slot config 11 : 0F 8F
slot config 12 : 0F 8F
slot config 13 : 8F 8F
slot config 14 : 8F 8F
slot config 15 : 8F 8F

00 00 00 01  00 00 00 00 0F FF FF FF
00 00 00 00 00 00 00 F7  00 69 76 00 00 00 00 00
00 00 00 00 00 00 00 00  FF FF 0E 60 00 00 00 00

key config 00 : 53 00
key config 01 : 53 00
key config 02 : 73 00
key config 03 : 73 00 
key config 04 : 73 00
key config 05 : 1C 00
key config 06 : 7C 00
key config 07 : 1C 00
key config 08 : 3C 00
key config 09 : 1A 00
key config 10 : 1C 00
key config 11 : 10 00
key config 12 : 1C 00
key config 13 : 1C 00
key config 14 : 1C 00
key config 15 : 1C 00
```

```shell
[00:00:04.413,000] <dbg> operation: example_operations: operation 5 selected
[00:00:04.420,000] <inf> operation: --- Hard-Reading MPU Registers (16 slots) ---
[00:00:04.428,000] <inf> operation: region 01: addr: 0x40000000 --- xn : 0 --- ap : 1 --- size: 0x20000000 bytes
[00:00:04.439,000] <inf> operation: region 02: addr: 0x70000000 --- xn : 0 --- ap : 2 --- size: 0x00008000 bytes
[00:00:04.450,000] <inf> operation: region 03: addr: 0x24000000 --- xn : 1 --- ap : 1 --- size: 0x00080000 bytes
[00:00:04.461,000] <inf> operation: region 04: addr: 0x08FFF800 --- xn : 0 --- ap : 1 --- size: 0x00000200 bytes
[00:00:04.472,000] <inf> operation: region 05: addr: 0x30004000 --- xn : 1 --- ap : 1 --- size: 0x00004000 bytes
[00:00:04.482,000] <inf> operation: region 06: addr: 0x30004000 --- xn : 0 --- ap : 1 --- size: 0x00000100 bytes
[00:00:04.493,000] <inf> operation: region 07: addr: 0x38800000 --- xn : 1 --- ap : 1 --- size: 0x00001000 bytes

not very clear???

[00:00:04.504,000] <inf> operation: region 08: addr: 0x30000000 --- xn : 1 --- ap : 1 --- size: 0x00004000 bytes
[00:00:04.515,000] <inf> operation: region 09: addr: 0x70000000 --- xn : 0 --- ap : 2 --- size: 0x02000000 bytes
[00:00:04.525,000] <inf> operation: region 10: addr: 0x2401F380 --- xn : 1 --- ap : 5 --- size: 0x00000080 bytes
```


# SPI

## read w25qxx id

```shell
inspiron:~$ spi conf spim_dt 8000000
inspiron:~$ spi transceive spim_dt 0x9F 0x00 0x00 0x00
TX:
00000000: 9f 00 00 00                                      |....             |
RX:
00000000: 00 ef 40 17                                      |..@.             |
inspiron:~$ spi conf spim_dt 1171875
inspiron:~$ spi transceive spim_dt 0x9F 0x00 0x00 0x00
TX:
00000000: 9f 00 00 00                                      |....             |
RX:
00000000: 00 ef 40 17                                      |..@.             |

```

ubuntu eth
```shell
sudo ufw disable
```

PATCH
```c
(.venv) pi@inspiron:~/zephyr/zephyr$ git log -n 1
commit 23892b038f6a94f6677318fecbc4df98fc8e2ac5 (HEAD -> nucleo_h7s3l8)
Author: Thinh Le Cong <thinh.le.xr@bp.renesas.com>
Date:   Fri Oct 24 13:05:18 2025 +0700

    drivers: serial: fix IAR warning Pe1072 about declaration after a label
    
    Fix Pe1072 warning (declaration after case label) by wrapping with braces
    
    Signed-off-by: Thinh Le Cong <thinh.le.xr@bp.renesas.com>
(.venv) pi@inspiron:~/zephyr/zephyr$ west diff
=== diff for manifest (zephyr):
diff --git zephyr/drivers/entropy/Kconfig.stm32 zephyr/drivers/entropy/Kconfig.stm32
index 494af0d109e..4a0b476bda8 100644
--- zephyr/drivers/entropy/Kconfig.stm32
+++ zephyr/drivers/entropy/Kconfig.stm32
@@ -8,6 +8,7 @@ menuconfig ENTROPY_STM32_RNG
 	default y
 	depends on DT_HAS_ST_STM32_RNG_ENABLED || DT_HAS_ST_STM32_RNG_NOIRQ_ENABLED
 	select ENTROPY_HAS_DRIVER
+	select USE_STM32_HAL_PKA
 	help
 	  This option enables the RNG processor, which is a entropy number
 	  generator, based on a continuous analog noise, that provides
diff --git zephyr/modules/mbedtls/configs/config-mbedtls.h zephyr/modules/mbedtls/configs/config-mbedtls.h
index 3c33586c4c1..fd1c07752d2 100644
--- zephyr/modules/mbedtls/configs/config-mbedtls.h
+++ zephyr/modules/mbedtls/configs/config-mbedtls.h
@@ -561,4 +561,16 @@
 #include CONFIG_MBEDTLS_USER_CONFIG_FILE
 #endif
 
+/* ECDSA */
+#define MBEDTLS_ECDSA_GENKEY_ALT
+#define MBEDTLS_ECDSA_SIGN_ALT
+#define MBEDTLS_ECDSA_VERIFY_ALT
+
+/* ECDH */
+#define MBEDTLS_ECDH_GEN_PUBLIC_ALT
+#define MBEDTLS_ECDH_COMPUTE_SHARED_ALT
+
+/* RSA */
+#define MBEDTLS_MPI_EXP_MOD_ALT
+
 #endif /* MBEDTLS_CONFIG_H */
diff --git zephyr/modules/mbedtls/debug.c zephyr/modules/mbedtls/debug.c
index 69bafc9c6d6..d4d70b52531 100644
--- zephyr/modules/mbedtls/debug.c
+++ zephyr/modules/mbedtls/debug.c
@@ -42,16 +42,16 @@ void zephyr_mbedtls_debug(void *ctx, int level, const char *file, int line, cons
 	switch (level) {
 	case 0:
 	case 1:
-		LOG_ERR("%s:%04d: %.*s", basename, line, str_len, str);
+		LOG_DBG("L:%04d: %.*s", line, str_len, str);
 		break;
 	case 2:
-		LOG_WRN("%s:%04d: %.*s", basename, line, str_len, str);
+		//LOG_WRN("%s:%04d: %.*s", basename, line, str_len, str);
 		break;
 	case 3:
-		LOG_INF("%s:%04d: %.*s", basename, line, str_len, str);
+		//LOG_INF("%s:%04d: %.*s", basename, line, str_len, str);
 		break;
 	default:
-		LOG_DBG("%s:%04d: %.*s", basename, line, str_len, str);
+		//LOG_DBG("%s:%04d: %.*s", basename, line, str_len, str);
 		break;
 	}
 }
diff --git zephyr/soc/st/stm32/stm32h7rsx/mpu_regions.c zephyr/soc/st/stm32/stm32h7rsx/mpu_regions.c
index 02bb198cb74..e65bdc40297 100644
--- zephyr/soc/st/stm32/stm32h7rsx/mpu_regions.c
+++ zephyr/soc/st/stm32/stm32h7rsx/mpu_regions.c
@@ -40,6 +40,8 @@ static const struct arm_mpu_region mpu_regions[] = {
 	MPU_REGION_ENTRY("SRAM_ETH_DESC", DT_REG_ADDR(sram_eth_node), REGION_PPB_ATTR(REGION_256B)),
 #endif
 #endif
+	MPU_REGION_ENTRY("BACKUP_SRAM", 0x38800000, REGION_RAM_ATTR(REGION_4K)),
+    MPU_REGION_ENTRY("SRAM_PKA_BUF", 0x30000000, REGION_RAM_NOCACHE_ATTR(REGION_16K)),
 };
 
 const struct arm_mpu_config mpu_config = {
diff --git zephyr/subsys/net/lib/sockets/sockets_inet.c zephyr/subsys/net/lib/sockets/sockets_inet.c
index ae47f2aba8a..31daed0b685 100644
--- zephyr/subsys/net/lib/sockets/sockets_inet.c
+++ zephyr/subsys/net/lib/sockets/sockets_inet.c
@@ -96,6 +96,7 @@ static int zsock_socket_internal(int family, int type, int proto)
 	struct net_context *ctx;
 	int res;
 
+	NET_INFO("F:%s -- L:%d -- fd:%d", __func__, __LINE__, fd);
 	if (fd < 0) {
 		return -1;
 	}
@@ -117,6 +118,7 @@ static int zsock_socket_internal(int family, int type, int proto)
 		return -1;
 	}
 
+	NET_INFO("F:%s -- L:%d -- ret:%d", __func__, __LINE__, res);
 	/* Initialize user_data, all other calls will preserve it */
 	ctx->user_data = NULL;
 

=== diff for mbedtls (modules/crypto/mbedtls):
diff --git modules/crypto/mbedtls/library/bignum.c modules/crypto/mbedtls/library/bignum.c
index f6b8f9998..839ee43cb 100644
--- modules/crypto/mbedtls/library/bignum.c
+++ modules/crypto/mbedtls/library/bignum.c
@@ -1734,7 +1734,7 @@ cleanup:
     return ret;
 }
 
-int mbedtls_mpi_exp_mod(mbedtls_mpi *X, const mbedtls_mpi *A,
+__attribute__((weak)) int mbedtls_mpi_exp_mod(mbedtls_mpi *X, const mbedtls_mpi *A,
                         const mbedtls_mpi *E, const mbedtls_mpi *N,
                         mbedtls_mpi *prec_RR)
 {
@@ -2057,7 +2057,7 @@ int mbedtls_mpi_inv_mod(mbedtls_mpi *X, const mbedtls_mpi *A, const mbedtls_mpi
     return MBEDTLS_ERR_MPI_NOT_ACCEPTABLE;
 }
 
-#if defined(MBEDTLS_GENPRIME)
+#if 1
 
 /* Gaps between primes, starting at 3. https://oeis.org/A001223 */
 static const unsigned char small_prime_gaps[] = {
diff --git modules/crypto/mbedtls/library/rsa.c modules/crypto/mbedtls/library/rsa.c
index 08267dbfc..526362688 100644
--- modules/crypto/mbedtls/library/rsa.c
+++ modules/crypto/mbedtls/library/rsa.c
@@ -1249,7 +1249,7 @@ int mbedtls_rsa_public(mbedtls_rsa_context *ctx,
     }
 
     olen = ctx->len;
-    MBEDTLS_MPI_CHK(mbedtls_mpi_exp_mod_unsafe(&T, &T, &ctx->E, &ctx->N, &ctx->RN));
+    MBEDTLS_MPI_CHK(mbedtls_mpi_exp_mod(&T, &T, &ctx->E, &ctx->N, &ctx->RN));
     MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(&T, output, olen));
 
 cleanup:
diff --git modules/crypto/mbedtls/library/ssl_client.c modules/crypto/mbedtls/library/ssl_client.c
index 0bd00cd91..4cee94960 100644
--- modules/crypto/mbedtls/library/ssl_client.c
+++ modules/crypto/mbedtls/library/ssl_client.c
@@ -350,6 +350,9 @@ static int ssl_write_client_hello_cipher_suites(
     cipher_suites = p;
     for (size_t i = 0; ciphersuite_list[i] != 0; i++) {
         int cipher_suite = ciphersuite_list[i];
+        /*
+            if(cipher_suite == 0x003d || cipher_suite == 0x003d || cipher_suite == 0x0035 || cipher_suite == 0x009c || cipher_suite == 0x003c || cipher_suite == 0x002f) continue;
+        */
         const mbedtls_ssl_ciphersuite_t *ciphersuite_info;
 
         ciphersuite_info = mbedtls_ssl_ciphersuite_from_id(cipher_suite);
diff --git modules/crypto/mbedtls/library/ssl_tls.c modules/crypto/mbedtls/library/ssl_tls.c
index 30cde2792..1312dd72f 100644
--- modules/crypto/mbedtls/library/ssl_tls.c
+++ modules/crypto/mbedtls/library/ssl_tls.c
@@ -4583,7 +4583,7 @@ int mbedtls_ssl_handshake_step(mbedtls_ssl_context *ssl)
 
 #if defined(MBEDTLS_SSL_CLI_C)
     if (ssl->conf->endpoint == MBEDTLS_SSL_IS_CLIENT) {
-        MBEDTLS_SSL_DEBUG_MSG(2, ("client state: %s",
+        MBEDTLS_SSL_DEBUG_MSG(0, ("client state: %s",
                                   mbedtls_ssl_states_str((mbedtls_ssl_states) ssl->state)));
 
         switch (ssl->state) {
diff --git modules/crypto/mbedtls/library/ssl_tls12_client.c modules/crypto/mbedtls/library/ssl_tls12_client.c
index 65d6dbd1a..dabfaf4d0 100644
--- modules/crypto/mbedtls/library/ssl_tls12_client.c
+++ modules/crypto/mbedtls/library/ssl_tls12_client.c
@@ -1387,7 +1387,7 @@ static int ssl_parse_server_hello(mbedtls_ssl_context *ssl)
     MBEDTLS_SSL_DEBUG_MSG(3, ("%s session has been resumed",
                               ssl->handshake->resume ? "a" : "no"));
 
-    MBEDTLS_SSL_DEBUG_MSG(3, ("server hello, chosen ciphersuite: %04x", (unsigned) i));
+    MBEDTLS_SSL_DEBUG_MSG(0, ("server hello, chosen ciphersuite: %04x", (unsigned) i));
     MBEDTLS_SSL_DEBUG_MSG(3, ("server hello, compress alg.: %d",
                               buf[37 + n]));
 
@@ -1423,7 +1423,7 @@ static int ssl_parse_server_hello(mbedtls_ssl_context *ssl)
         return MBEDTLS_ERR_SSL_HANDSHAKE_FAILURE;
     }
 
-    MBEDTLS_SSL_DEBUG_MSG(3,
+    MBEDTLS_SSL_DEBUG_MSG(0,
                           ("server hello, chosen ciphersuite: %s", suite_info->name));
 
 #if defined(MBEDTLS_SSL_ECP_RESTARTABLE_ENABLED)

Empty diff in 63 projects.
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
