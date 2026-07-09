# Architecture

> Updated to match the current NovaAI prototype.

## Components
- FastAPI server acting as a Gemini Live bridge.
- ESP32 firmware handling I2S audio, WS2812 status ring and IR RGB control.
- Browser client for debugging/testing.

## Current Hardware
- ESP32 NodeMCU-32S
- INMP441 microphone
- MAX98357A amplifier
- WS2812B 12-LED ring
- IR LED (RGB control only)

## Audio Flow
INMP441 (16 kHz PCM) → ESP32 → WebSocket → FastAPI → Gemini Live → FastAPI → ESP32 (24 kHz PCM) → Ring buffer → MAX98357A.

## Hardware Control
Current Gemini function calls:
- RGB_ON
- RGB_OFF
- RGB_RED
- RGB_GREEN
- RGB_BLUE
- SLEEP

AC control exists experimentally but is **not part of the current release**.
