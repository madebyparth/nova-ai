# Troubleshooting

## Server Does Not Start

Check that dependencies are installed:

```bash
pip install fastapi uvicorn google-genai
```

Check that port `8000` is free.

## Browser Cannot Connect

Use:

```text
http://localhost:8000
```

The browser WebSocket path is:

```text
ws://localhost:8000/ws
```

If microphone access fails, make sure the browser has microphone permission.

## ESP32 Cannot Connect

Check:

- Wi-Fi SSID and password
- server IP address
- server port
- computer firewall rules
- ESP32 and server are on the same network

The ESP32 connects to:

```text
ws://<server-ip>:8000/esp32
```

## No Audio From ESP32 Speaker

Check:

- MAX98357A wiring
- `SPK_LRC`, `SPK_BCLK`, and `SPK_DIN` pin definitions
- speaker power and ground
- serial logs for incoming binary WebSocket frames
- whether the ring buffer is filling

## Microphone Not Sending Audio

Check:

- INMP441 wiring
- `MIC_WS`, `MIC_SCK`, and `MIC_SD` pin definitions
- noise gate thresholds
- whether the room is quiet enough that audio stays below `THRESHOLD_IDLE`

## Assistant Audio Cuts Off

The ESP32 endpoint uses a server-side pacer and ESP32-side jitter buffer. If audio still cuts off:

- increase `RING_BUFFER_SIZE`
- increase `PREBUFFER_BYTES`
- reduce Wi-Fi congestion
- check server CPU load
- reduce chunk pacing aggressiveness

## Barge-In Does Not Stop Audio

Browser flow:

- Gemini must emit the `interrupted` flag.
- The server sends `CLEAR_AUDIO`.
- The browser clears queued sources.

ESP32 flow:

- Gemini must emit the `interrupted` flag.
- The server sets `clear_audio_flag`.
- The pacer flushes `audio_queue`.
- The firmware should clear its ring buffer when it receives `CLEAR_AUDIO`.

Note: the current ESP32 server path sets the server-side clear flag but does not send a `CLEAR_AUDIO` text command during every interrupt. The firmware supports `CLEAR_AUDIO`, so this is a good future reliability improvement.

## RGB Commands Do Not Work

Check:

- IR LED pin
- IR LED direction and resistor
- RGB remote protocol compatibility
- NEC codes in `NovaAI.ino`
- Gemini tool call logs in the Python console

## Session Does Not End On Goodbye

There are two session-ending paths:

- Gemini calls `end_chat_session`.
- The fallback farewell detector sees a short farewell phrase in text output.

If neither happens, inspect the server console transcript and tool-call logs.
