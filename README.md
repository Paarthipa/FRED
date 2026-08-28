# FRED – Fault Response and Emergency Detection System

## M1QP – Comprehensive Interrupt-Based Sense-Think-Act System

FRED (Fault Response and Emergency Detection) is an interrupt-driven
Sense-Think-Act embedded system developed using an Arduino Uno R3.

The system combines multiple input devices, Pin Change Interrupts (PCI),
and a Timer1 periodic interrupt to monitor system conditions and respond
using visual, audible and display outputs.

The project demonstrates the ⭐⭐⭐⭐ Distinction-level requirements of the
M1QP task by combining:

- Multiple digital inputs
- Pin Change Interrupts (PCI)
- Timer1 periodic interrupts
- Event-driven processing
- Periodic monitoring
- State and priority management
- Modular interrupt-safe programming
- LCD, LED and buzzer outputs
- Structured Serial Monitor feedback

---

## 1. Project Overview

The FRED system continuously monitors three main input sources:

1. **PIR sensor** – detects motion.
2. **Tilt sensor** – detects tampering or orientation changes.
3. **555 timer heartbeat circuit** – generates a periodic heartbeat
   signal used to monitor system health.

The system uses Pin Change Interrupts to respond to changes on the
PIR, tilt and heartbeat inputs.

A Timer1 interrupt provides a periodic monitoring event of approximately
one second. This allows the system to check heartbeat activity and
update the overall system state without relying entirely on continuous
polling.

The system follows the:

**Sense → Think → Act**

architecture.

---

## 2. Sense-Think-Act Architecture

```text
                 FRED SYSTEM
                     │
        ┌────────────┴────────────┐
        │                         │
   EVENT-DRIVEN              TIME-DRIVEN
       PCI                     TIMER1
        │                         │
   ┌────┼────┐                    │
   │    │    │                    │
  D8   D9   D10              1-second event
  PIR Tilt Heartbeat               │
   │    │    │                     │
   └────┼────┘                     │
        │                          │
        └──────────┬───────────────┘
                   │
              Main Program
                   │
             Priority Logic
                   │
          ┌────────┼────────┐
          │        │        │
         LCD      LEDs    Buzzer