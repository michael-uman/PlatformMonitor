# PortMonitor

## Description

This project demonstrates MCU controller programming via the HAL and FreeRTOS APIs. It also uses a Qt Front End (`frontend`) GUI for display of the devices status.

## Command Packets

The `frontend` sends commands to the MCU to support functions like turning the LED on and off, and requesting the version # of the firmware.

The command packets are raw binary data. The data structure is defined such that the # of bits in each field is the same on the host and the target processors. The endian-ness of the integers are the same thus there is no need to do swapping.

Here is the data structure used to send commands to the MCU:

```c
typedef struct {
    uint32_t    cmd;
    uint32_t    data; // lt cmdr ;)
} RECVCMD_t;
```

And the commands recognized by the controller software:

```c
typedef enum {
    SENDCMD_START = 0x0080,
    SENDCMD_MSG = SENDCMD_START,
    SENDCMD_STATUS,
    SENDCMD_VERSION,
    SENDCMD_END,
} eSENDCMD;
```

## JSON Packets

The MCU communicates with the `frontend` by sending the `Status Packet` every ~1/4 second. This packet contains an identifying # (counter) along with the state of the button and the LED.

The software employs several other JSON packets which can be sent. These include the `Message Packet` and the `Version Packet` which are discussed below.

The `frontend` opens the serial port for read/write and schedules a version request for 1/2 second in the future. Due to the fact that the device is streaming packets, the odds are that the first line from the serial port will be garbled, thus it will be identified as an invalid JSON format. The version request is sent and the JSON reply is received and the JSON decoded to display the version in the User Interface.

The packets are described here:

### Status Packet

```json
{
    "id": <uint32_t>,
    "button": <int>,
    "led": <int>
}
```

### Message Packet

```json
{
    "message": <"message string">
}
```

### Version Packet

```json
{
    "version": <"x.x.x">
}
```

## FreeRTOS Micro-Controller Firmware

The `MCU` firmware was developed using the `STM32CubeMX` package to generate the prototype project. This software was used to generate the skeleton code which initializes the STM32 peripherals.

The firmware uses the `FreeRTOS` middleware package to provide multi-threaded application support. The firmware employs three threads working in synchrony to provide functions to the host User Interface code.

The thread tasks are summarized below:

* Periodic polling thread (`flashTask`)
    * This thread is responsible for reading the status of the buttons and putting a message in the `send thread`'s message gueue.
* Send thread (`uartTask`)
    * This thread waits on a message queue. When it receives a message it looks at the message command and it sends the result data via the UART to the host. There are `status commands` and `version commands`.
* Receive thread (`recvTask`)
    * This thread issues a read from UART request which will be signalled by the interrupt when data is ready. When the IRQ handler receives the data it then copies the packet into a message queu and sends it to this thread which is waiting for a message. Then the command is parsed and the application state is updated.
    

