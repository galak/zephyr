.. _spm_nspe_psoc6:

SPM base NS environment example for PSoC6 CM4
#############################################

Overview
********
This sample is a simple SPM base NS environment example for PSoC6 CM4.
Needs Secure part on CM0+

CM4(spm_nspe) sends debug log to Uart Port5 which is available through USB 
connection with the PSoC6 kit.
CM0+(spm_spe) sends debug messages to UART port 12(pin0(RX)/pin1(TX)) - 
connect external UART to USB convertor to these pins on kit

Project description:

- CM0+ starts CM4 core
- CM4 writes the message with the counter in loop to the shared memory block, 
triggers interrupt to CM0+ core and wait on response.
- Once interrupt received, CM0+ prints out received message and initiate 
response interrupt to CM4.

Building and Running
********************
It can be built with creating an Eclipse project as follows:

.. code-block: bash

   cmake -G"Eclipse CDT4 - Ninja" -DBOARD=cy8ckit_062_wifi_bt_m4 ..
   ninja

After build, dowload the created zephyr.elf by openOCD with configuration:
GDB port: 3333
Telnet port: 4444
Config options:
--search "${cy_tools_path:openocd}/scripts"
-c "gdb_memory_map enable"
-c "gdb_flash_program enable"
-c "interface cmsis-dap"
-c "transport select swd"
-c "source [find interface/cmsis-dap.cfg]"
-c "source [find target/psoc6.cfg]"

Sample Output
=============

.. code-block:: console

   ***** Booting Zephyr OS v1.12.0-rc2-48-g2c3c674aa *****
   CM4: Main
   IPC test app arm
   CM4: ipc_interrupt_handler - 1 INTR = 0x1000000
   CM4: ipc_interrupt_handler - 1 INTR = 0x1000000
   CM4: ipc_interrupt_handler - 1 INTR = 0x1000000
   CM4: ipc_interrupt_handler - 1 INTR = 0x1000000
   CM4: ipc_interrupt_handler - 1 INTR = 0x1000000
   ...


