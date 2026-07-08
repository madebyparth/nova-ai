# API

NovaAI exposes a small local API through FastAPI.

## `GET /`

Returns the embedded browser voice interface.

### Response

`text/html`

The page contains:

- Connect button
- Disconnect button
- microphone capture
- browser WebSocket client
- audio playback queue
- `CLEAR_AUDIO` and `CLOSE_SESSION` command handling

## `WebSocket /ws`

Browser voice bridge.

### Client To Server

Binary WebSocket frames:

```text
audio/pcm;rate=16000
16-bit signed little-endian PCM
mono
```

### Server To Client

Binary WebSocket frames:

```text
audio/pcm;rate=24000
16-bit signed little-endian PCM
mono
```

Text commands:

| Command | Meaning |
| --- | --- |
| `CLEAR_AUDIO` | Stop currently scheduled assistant audio and clear the browser playback queue. |
| `CLOSE_SESSION` | End the browser session after a farewell. |

### Behavior

The server opens a Gemini Live session and runs two asynchronous tasks:

- browser audio to Gemini
- Gemini response audio/tool events to browser

The first task to finish causes the other task to be cancelled.

## `WebSocket /esp32`

ESP32 hardware bridge.

### Device To Server

Binary WebSocket frames:

```text
audio/pcm;rate=16000
16-bit signed PCM
mono
```

### Server To Device

Binary WebSocket frames:

```text
audio/pcm;rate=24000
16-bit signed PCM
mono
```

Text commands:

| Command | Meaning |
| --- | --- |
| `SLEEP` | End the hardware session and disconnect the ESP32 WebSocket. |
| `RGB_ON` | Send the IR code for RGB lights on. |
| `RGB_OFF` | Send the IR code for RGB lights off. |
| `RGB_RED` | Send the IR code for red lighting. |
| `RGB_GREEN` | Send the IR code for green lighting. |
| `RGB_BLUE` | Send the IR code for blue lighting. |

### Behavior

The server opens a Gemini Live session and runs three asynchronous tasks:

- ESP32 audio to Gemini
- Gemini response handling
- paced outbound audio streaming to the ESP32

The audio pacer prevents large Gemini audio chunks from overwhelming the ESP32 speaker pipeline.

## Gemini Tool Mapping

| Gemini tool call | Browser endpoint action | ESP32 endpoint action |
| --- | --- | --- |
| `end_chat_session` | send `CLOSE_SESSION` | send `SLEEP` |
| `rgb_on` | unavailable | send `RGB_ON` |
| `rgb_off` | unavailable | send `RGB_OFF` |
| `rgb_red` | unavailable | send `RGB_RED` |
| `rgb_green` | unavailable | send `RGB_GREEN` |
| `rgb_blue` | unavailable | send `RGB_BLUE` |

## Error Handling

Both WebSocket endpoints catch disconnects and broad exceptions, mark the session inactive, cancel outstanding tasks, and attempt to close the WebSocket cleanly.

The current prototype prints diagnostics to stdout instead of using structured logging.
