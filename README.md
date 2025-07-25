# Anjay Lite Bare Metal Client

## Overview

This project demonstrates how to use [Anjay Lite](https://github.com/AVSystem/Anjay-lite) in a bare-metal application
on the [STM32 Nucleo-U385RG-Q](https://www.st.com/en/evaluation-tools/nucleo-u385rg-q.html) development board.  
The board features an STMicroelectronics microcontroller known for its excellent power efficiency.

Internet connectivity is provided through a BG96 modem. The project includes a simple
modem integration and a network API compatibility layer, implemented in the `net.c` file, 
which provides the required socket API functions.

This example implements the mandatory LwM2M objects:

* [Security Object (/0)](https://raw.githubusercontent.com/OpenMobileAlliance/lwm2m-registry/prod/0.xml)  
* [Server Object (/1)](https://raw.githubusercontent.com/OpenMobileAlliance/lwm2m-registry/prod/1.xml)  
* [Device Object (/3)](https://raw.githubusercontent.com/OpenMobileAlliance/lwm2m-registry/prod/3.xml)  

as well as an example object:

* [Temperature Object (/3303)](https://raw.githubusercontent.com/OpenMobileAlliance/lwm2m-registry/prod/3303.xml)  

Temperature data is read from the MCU’s internal temperature sensor.

---

## Prerequisites

### Hardware
* STM32 Nucleo-U385RG-Q development board  
* Cellular modem: Airgain NL-SWDK2 Shield  
* USB Type-C cable to connect the development kit to your PC  

### Software
* A recent Linux distribution (e.g. Ubuntu 24.04)  
* `git`  
* `gcc-arm-none-eabi` (see details below)  
* `cmake`  
* STM32CubeProgrammer with CLI added to your `PATH`  
* A serial monitor application (e.g. `minicom`) to view logs  

---

## Connecting the Cellular Modem

This project was originally developed with the
[Airgain NL-SWDK2 Shield](https://www.airgain.com/products/skywire-dev-kit-swdk2/) in mind as a plug-and-play demo.  
However, most BG96 development kits should work with only minor peripheral configuration changes.  

For details, see the tested modules listed under [Other Supported Modules](#support-for-other-bg96-modules).

---

### Airgain NL-SWDK2 Shield

To use the Airgain NL-SWDK2 Shield:

1. Set the `JP3` jumper from `STLK` to `VIN 5V` to power the Nucleo board from the shield’s external supply.  
2. Mount the shield directly on top of the Nucleo board.  
3. Configure DIP switches:
   - SW1-1 (ARD1): `ON`  
   - SW1-2 (ARD2): `OFF`  
   - SW1-3 (Auto-ON): `OFF`  
   
   This enables D0/D1 pins for UART communication and allows the MCU to control the modem’s power supply.  
4. Insert a SIM card.  
5. Connect an external power supply (5–15V) to the barrel jack on the modem board.  

**Important:** Always connect both antenna cables and securely attach the antennas before powering on.

![STM32 Nucleo and NL-SWDK2 Shield board](nucleo_nl_swdk2.png)

---

## Cloning the Repository

```sh
git clone https://github.com/AVSystem/Anjay-lite-bare-metal-client --recursive
```

---

## Configuration

The project was generated with CubeMX v6.14.0 and STM32Cube FW\_U3 V1.1.0, then reorganized.
If you need to change pin or peripheral configuration, the base CubeMX project is provided as a
backup in `deps/ST/anjay-lite-bare-metal-client.ioc`.
Note: The current project structure is not directly compatible with newly generated CubeMX outputs.

`Anjay Lite` configuration is handled via `cmake`, but for convenience static config files are provided in the `config` directory.

To update settings, edit `config/anj/anj_config.h` and rebuild the project.

Application-specific configuration options:

* Endpoint name (default: `anjay-lite-bare-metal-client`)
  Override with: `-DCONFIG_ENDPOINT_NAME="your_endpoint_name"`
* APN name (default: `internet`)
  Override with: `-DCONFIG_APN="your_apn_name"`

---

## Building

The system toolchain version of `gcc-arm-none-eabi` (from `apt`/`dnf`) is incompatible due to a 
[known bug](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1067692).
Use the official version from [ARM Downloads](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads), 
version 11.2 or newer.
Make sure its `bin` directory is in your `PATH`.

Verify with:

```sh
which arm-none-eabi-gcc
```

It should point to the newly installed compiler.

Install dependencies:

```sh
sudo apt-get update && \
    sudo apt-get install cmake make build-essential
```

Build the project:

```sh
mkdir build
cd build/
cmake ..
make -j
```

CMake presets are available — run `cmake .. --list-presets` inside `build/` to see them.

---

## Flashing

The `st-flash` tool (v1.8.0) does not yet support U3 microcontrollers.
Instead, use [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) version 2.19.0 or newer.

Add its `bin` folder to your `PATH`. Check the version with:

```sh
STM32_Programmer_CLI -v
```

Flash the board:

```sh
STM32_Programmer_CLI -c port=SWD mode=UR reset=HWrst -d anjay-lite-bare-metal-client.elf -v -rst
```

---

## Serial Logs

Logs are sent via the onboard ST-Link at 115200 baud, 8N1.
On Linux, the device is typically available at `/dev/ttyACM0` if no other serial devices are connected.

---

## Connecting to the LwM2M Server

To connect with the Coiote IoT Device Management Platform:

1. Register at [https://eu.iot.avsystem.cloud](https://eu.iot.avsystem.cloud).
2. Create a new device.
3. For now, choose Security mode: NoSec (this example does not use secure CoAP).
4. Use the endpoint name configured in CMake.

Detailed instructions are available here: 
[Adding devices in Coiote IoT](https://eu.iot.avsystem.cloud/doc/user/getting-started/add-devices/)

---

## Support for Other BG96 Modules

Besides the [Airgain NL-SWDK2 Shield](https://www.airgain.com/products/skywire-dev-kit-swdk2/),
this project has also been tested with the 
[STEVAL-STMODLTE Development Board](https://www.st.com/en/evaluation-tools/steval-stmodlte.html).

### Using the STEVAL-STMODLTE Module

1. Wire the module using jumper wires as shown below:

| STM32 Nucleo Pin | Arduino Pin | Eval Modem Pin     | Function               |
| ---------------- | ----------- | ------------------ | ---------------------- |
| PA2              | D0 (RX)     | TXDS – Pin 2       | LPUART1\_RX – Modem TX |
| PA3              | D1 (TX)     | RXDS – Pin 3       | LPUART1\_TX – Modem RX |
| **GND**          | **GND**     | **GND – Pin 5/16** | **Common Ground**      |
| PA0              | A0          | PWRen – Pin 9      | Modem Power Control    |

2. Power the modem:

   * Normally via the onboard micro-USB port.
   * If needed, provide an external 5V supply via `VCC (Pin 6/15)` and `GND (Pin 5/16)`.

3. Using a physical SIM (instead of the onboard eSIM):

   * Insert a SIM card.
   * Connect both `Sim_Select0 (Pin 8)` and `Sim_Select1 (Pin 18)` to GND.

Note: The onboard eSIM is enabled by default. Perform step 3 only if you plan to use a physical SIM card.

---

# Links

* [Anjay Lite source repository](https://github.com/AVSystem/Anjay-lite)
* [Anjay Lite documentation](https://avsystem.github.io/Anjay-lite/index.html)
* [Anjay Lite integrations](https://avsystem.github.io/Anjay-lite/Integrations.html)
* [Doxygen API documentation](https://avsystem.github.io/Anjay-lite/api/index.html)
* [AVSystem IoT Devzone](https://iotdevzone.avsystem.com)
* [AVSystem Discord server](https://discord.avsystem.com)
