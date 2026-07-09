# API

## GET /
Serves the browser voice interface.

## WebSocket /ws
Browser ↔ Gemini Live bridge.

## WebSocket /esp32
ESP32 ↔ Gemini Live bridge.

Current text commands:
- SLEEP
- RGB_ON
- RGB_OFF
- RGB_RED
- RGB_GREEN
- RGB_BLUE

Binary frames contain raw PCM audio (16 kHz upstream, 24 kHz downstream).
