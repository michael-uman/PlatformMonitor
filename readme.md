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
