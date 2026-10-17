# AUTOSAR-style UDS on a Virtual CAN Bus

This is a small, portable C learning project for exploring how a UDS request
can pass through an AUTOSAR-style communication stack. It runs in a terminal
and does not need an ECU, CAN adapter, or other hardware.

> **Important:** This is an educational simulator, not an AUTOSAR-compliant
> implementation. Its module boundaries are inspired by AUTOSAR Classic, but
> it does not use AUTOSAR-generated configuration, an AUTOSAR RTE, a production
> CAN driver, or a certified protocol stack. It implements ISO-TP single frames
> only, so each UDS request and response is limited to seven bytes.

## Layer flow

```text
Tester command
    |
    v
SimCan       Creates a virtual CAN data frame (request ID 0x7E0)
    |
    v
CanIf        Checks the configured CAN ID and passes the frame upward
    |
    v
CanTp        Decodes an ISO-TP single frame
    |
    v
PduR         Routes the diagnostic PDU to the DCM
    |
    v
Dcm          Checks UDS service, session, security, and request parameters
    |
    v
Rte_DemoApp  Provides simulated ECU data to the diagnostic service
```

Responses travel back down through `PduR`, `CanTp`, and `CanIf` to `SimCan`
and are printed with response CAN ID `0x7E8`. `EcuM` is a small simulation of
the reset-request boundary.

## Included UDS behavior

| SID | Service | Demonstration |
| --- | --- | --- |
| `0x10` | DiagnosticSessionControl | Default (`0x01`) and extended (`0x03`) sessions |
| `0x11` | ECUReset | Simulated hard reset request (`0x01`) |
| `0x22` | ReadDataByIdentifier | Reads demo DIDs `0xF1A0` and `0xF1A1` |
| `0x27` | SecurityAccess | Seed/key sequence using a deliberately weak demo algorithm |
| `0x2E` | WriteDataByIdentifier | Writes DID `0xF1A0` in extended session after unlocking |
| `0x3E` | TesterPresent | Tester keep-alive; supports suppressed positive response |

The project also demonstrates negative responses for unsupported services,
incorrect lengths, unsupported subfunctions, invalid DIDs, invalid keys, and
requests made in the wrong session/state.

## Build

Requirements: a C99 compiler and CMake 3.16 or newer.

```text
cmake -S . -B build
cmake --build build
```

Run the executable from the project directory:

```text
./build/autosar_uds_simulator
```

On Windows with a Visual Studio generator, the executable is typically at
`build\Debug\autosar_uds_simulator.exe`.

## Try it

Enter UDS payload bytes in hexadecimal. Do not include the ISO-TP PCI byte;
the virtual CAN adapter adds it for you.

```text
send 10 03
send 27 01
send 27 02 B7 91
send 22 F1 A0
send 2E F1 A0 00 28
send 22 F1 A0
send 3E 00
```

The seed is fixed (`0x1234`) and the sample key calculation is
`seed XOR 0xA5A5`, yielding `0xB791`. This is intentionally insecure and exists
only to demonstrate the UDS request/response sequence. Never use this scheme
for real access control.

`0xF1A0` is a two-byte simulated vehicle-speed value initialized to `0x0014`;
`0xF1A1` is a read-only two-byte simulated engine-speed value initialized to
`0x0BB8`. These raw values are demo data and do not define real signal scaling.

The shell accepts one to seven request bytes per command. Use `help` for the
commands or `quit` to exit. The in-memory DID value returns to its default
when the program restarts.

## Understanding the trace

For `send 22 F1 A0`, the simulator prints each layer's view of the same request:

```text
[SimCan] TX | CAN ID: 0x7E0 | Data: 03 22 F1 A0 00 00 00 00
[CanIf] RX | CAN ID: 0x7E0 | Data: 03 22 F1 A0 00 00 00 00
[CanTp] RX UDS | Data: 22 F1 A0
[PduR] ROUTE | UDS: 22 F1 A0
[DCM] RX | SID: 0x22 | DID: 0xF1A0
[DCM] TX | UDS: 62 F1 A0 00 14
[PduR] ROUTE TX | UDS: 62 F1 A0 00 14
[CanTp] TX CAN | Data: 05 62 F1 A0 00 14 00 00
[CanIf] TX | CAN ID: 0x7E8 | Data: 05 62 F1 A0 00 14 00 00
[SimCan] TX | CAN ID: 0x7E8 | Data: 05 62 F1 A0 00 14 00 00
```

On request, SimCan adds the CAN ID and ISO-TP single-frame length byte (`03`),
CanIf accepts the configured CAN ID, CanTp removes that transport byte, and
PduR passes the UDS payload to DCM. DCM interprets SID `0x22` as ReadDataByIdentifier
and DID `0xF1A0` as the demo vehicle-speed data. The response begins with
positive-response SID `0x62`, echoes the DID, and ends in value `00 14`.
On response, each layer adds its own information again; `05` is the response
UDS length and `0x7E8` is the ECU response CAN ID. Unused CAN data bytes are
zero-filled by this simulator.

## Suggested interview explanation

Describe this accurately as a **personal AUTOSAR-style UDS-on-CAN simulator**.
Walk through a request such as `22 F1 A0`: the virtual bus creates a CAN frame,
CanIf validates its ID, CanTp removes the ISO-TP single-frame header, PduR
routes the diagnostic PDU, DCM handles the UDS service, and the demo application
supplies DID data through an RTE-like interface. Then trace the response back
down the stack. Be ready to explain that this project models the architecture
for learning and is not an AUTOSAR-certified stack or a hardware integration.

## Source map

- `src/main.c`: interactive terminal shell
- `src/sim_can.c`: virtual CAN request/response display
- `src/can_if.c`: CAN ID filtering and simulated transmission
- `src/can_tp.c`: ISO-TP single-frame encoding/decoding
- `src/pdu_router.c`: diagnostic PDU routing
- `src/dcm.c`: UDS service handling and diagnostic state
- `src/rte_demo_app.c`: simulated application data/DID access
- `src/ecum.c`: simulated ECU reset boundary

## Header and build-file map

- `include/project_types.h`: shared types and CAN/UDS message-size limits
- `include/sim_can.h`: virtual CAN endpoint interface
- `include/can_if.h`: simulated CAN Interface function declarations
- `include/can_tp.h`: ISO-TP single-frame transport declarations
- `include/pdu_router.h`: diagnostic PDU routing declarations
- `include/dcm.h`: diagnostic service manager declarations
- `include/rte_demo_app.h`: demo application data access declarations
- `include/ecum.h`: simulated reset request declaration
- `CMakeLists.txt`: source list, C99 compiler settings, include path, and warnings
