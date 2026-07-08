# Developer Guide

This guide explains the runtime behavior from a developer's point of view.

## How Audio Travels

### Browser Path

1. The browser asks for microphone permission through `navigator.mediaDevices.getUserMedia`.
2. A Web Audio `AudioWorkletProcessor` receives float samples.
3. The worklet converts samples to signed 16-bit PCM.
4. The browser sends raw PCM bytes over `/ws`.
5. FastAPI forwards those bytes to Gemini Live as `audio/pcm;rate=16000`.
6. Gemini returns 24 kHz PCM audio chunks.
7. The server sends those chunks back to the browser.
8. The browser converts PCM into `AudioBuffer` objects and schedules them on the audio hardware clock.

### ESP32 Path

1. The INMP441 microphone sends audio to the ESP32 over I2S.
2. The ESP32 reads 16-bit samples from `I2S_NUM_0`.
3. A simple peak-amplitude noise gate decides whether the chunk should be sent.
4. The ESP32 sends audio through binary WebSocket frames to `/esp32`.
5. FastAPI forwards the audio to Gemini Live.
6. Gemini returns 24 kHz PCM audio.
7. The server queues and paces audio into smaller chunks.
8. The ESP32 stores incoming bytes in a fixed ring buffer.
9. A FreeRTOS speaker task writes bytes to `I2S_NUM_1`.
10. The MAX98357A amplifier drives the speaker.

## How Gemini Is Connected

The server uses the Google GenAI SDK async Live API client. Each WebSocket connection creates its own live session:

- `/ws` creates a browser-focused Gemini session.
- `/esp32` creates a hardware-focused Gemini session.

Both sessions configure:

- audio response modality
- the `Aoede` voice
- Nova's system instruction
- an `end_chat_session` tool

The ESP32 session additionally registers RGB lighting tools. The model does not directly control hardware. It emits a tool call, and the Python server maps that tool call to a small text command for the ESP32.

## How The ESP32 Communicates

The firmware uses `WebSocketsClient`.

Binary messages:

- outgoing: microphone PCM
- incoming: assistant PCM

Text messages:

- `SLEEP`
- `CLEAR_AUDIO`
- `RGB_ON`
- `RGB_OFF`
- `RGB_RED`
- `RGB_GREEN`
- `RGB_BLUE`

`webSocketEvent()` is the command router. It updates state, clears buffers, writes audio, sends IR codes, and disconnects when sleep is requested.

## How Sessions Are Stored

The current implementation stores active conversation state inside the live Gemini session. When the WebSocket closes, that live session ends.

`app.py` includes `load_memory()` and `save_memory()` helpers for `memory.json`, but the active endpoints do not currently call them. That means persistent long-term memory is prepared conceptually but not wired into the runtime behavior yet.

## How Reconnection Works

Browser reconnection is manual. The user clicks `Connect & Listen` again after a disconnect.

ESP32 reconnection is automatic while awake:

```cpp
webSocket.setReconnectInterval(5000);
```

If the server sends `SLEEP`, the firmware sets `is_sleeping = true`, disconnects the WebSocket, and returns early from the main loop. In that state, it does not reconnect until firmware behavior is changed or the device is reset.

## How Memory Works

There are three different kinds of memory in this project:

| Type | Where | Purpose |
| --- | --- | --- |
| Session memory | Gemini Live session | Conversation context while connected |
| Playback memory | browser queue / ESP32 ring buffer | Smooth audio playback |
| Persistent memory placeholder | `memory.json` helpers | Future long-term memory |

The ESP32 ring buffer is intentionally fixed-size. That is a good embedded design choice because real-time audio code should avoid unpredictable heap allocation.

## Why These Architectural Decisions Were Made

### WebSockets

WebSockets are a strong fit because audio needs to move continuously in both directions. HTTP request/response would add unnecessary latency and complexity.

### Raw PCM Frames

Raw PCM avoids codec setup, decoding latency, and library complexity. It also maps directly to browser audio buffers and ESP32 I2S writes.

### Separate Browser And ESP32 Endpoints

The browser and ESP32 have different playback needs. The browser can schedule `AudioBuffer` objects accurately. The ESP32 needs paced chunks and a jitter buffer. Separate endpoints keep those concerns clear.

### Server-Side Gemini Bridge

The ESP32 does not talk to Gemini directly. That keeps API keys off the microcontroller, avoids TLS/API complexity on embedded hardware, and lets the Python server translate model events into hardware commands.

### FreeRTOS Speaker Task

Speaker output needs steady timing. Moving playback to a dedicated task prevents microphone reads and WebSocket polling from starving audio output.

### Ring Buffer

The ring buffer smooths network jitter and Gemini chunk variability. Dropping old bytes on overflow is preferable to blocking real-time code and causing system instability.

### Function Calls For Hardware Control

Gemini function calls create a narrow bridge from natural language to hardware actions. The model asks for a named action; the server decides what command is allowed and sends only that command to the ESP32.
