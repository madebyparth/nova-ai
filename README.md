# NovaAI

> **An open-source real-time AI voice assistant combining ESP32 hardware, FastAPI, and Gemini Live.**

![Python](https://img.shields.io/badge/Python-3.10+-3776AB?style=for-the-badge&logo=python&logoColor=white)
![FastAPI](https://img.shields.io/badge/FastAPI-WebSocket-009688?style=for-the-badge&logo=fastapi&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-I2S%20Voice-000000?style=for-the-badge&logo=espressif&logoColor=white)
![Gemini](https://img.shields.io/badge/Gemini-Live%20API-4285F4?style=for-the-badge&logo=google&logoColor=white)

NovaAI is a local voice gateway for building a physical AI assistant. The Python server accepts real-time PCM audio over WebSockets, forwards it to Gemini Live, streams generated speech back, and sends control commands to an ESP32-powered hardware device.

## Demo

> Add a short demo GIF here:

```text
docs/assets/novaai-demo.gif
```

## Features

- Real-time browser voice interface at `/`
- ESP32 hardware voice endpoint at `/esp32`
- Gemini Live audio-to-audio conversation bridge
- 16 kHz PCM microphone input streaming
- 24 kHz PCM assistant audio playback
- Barge-in handling with audio queue clearing
- ESP32 I2S microphone support for INMP441
- ESP32 I2S speaker output for MAX98357A
- WS2812B LED ring state feedback
- IR-based RGB lighting control tools
- Graceful session close flow using Gemini function calls
- Browser-side audio worklet capture and seamless playback scheduling
- ESP32-side jitter buffer and dedicated speaker task

## Why This Exists

Most AI assistants live behind apps, laptops, and cloud dashboards. NovaAI explores what it takes to make an AI assistant feel physical: always nearby, spoken to naturally, and capable of controlling real room hardware.

The interesting part is not just calling an AI API. It is the full pipeline:

- capturing raw microphone audio on constrained hardware
- streaming it over a local network
- preserving low-latency conversational flow
- handling interruptions while audio is already playing
- turning AI tool calls into physical actions

## System Architecture

```mermaid
flowchart LR

Browser["Browser UI<br/>Web Audio API"]

Server["FastAPI Server<br/>app.py"]

Gemini["Gemini Live API"]

ESP32["ESP32 Device<br/>NovaAI.ino"]

Mic["INMP441 Microphone"]
Amp["MAX98357A I2S Amplifier"]
Speaker["Speaker"]
LED["WS2812B LED Ring"]
IR["IR RGB Controller"]

Browser -->|PCM 16 kHz Audio| Server
Server -->|Browser Audio + Text| Browser

ESP32 -->|PCM 16 kHz Audio| Server
Server -->|PCM 24 kHz Audio + Device Commands| ESP32

Server <-->|Realtime Audio & Tool Calls| Gemini

Mic --> ESP32
ESP32 --> Amp
Amp --> Speaker
ESP32 --> LED
ESP32 --> IR
```

## Tech Stack

### Software
- Python
- FastAPI
- Gemini Live
- WebSockets

### Hardware
- ESP32
- INMP441
- MAX98357A
- WS2812B

### Protocols
- I2S
- PCM Audio
- WebSockets

## Folder Structure

```text
NovaAI/
├── app.py                    # FastAPI server, browser UI, Gemini bridge, ESP32 endpoint
├── NovaAI.ino                # ESP32 firmware for mic, speaker, LEDs, IR, WebSocket client
├── .env.example              # Example configuration values for a future env-based setup
├── .gitignore                # Python, Arduino, secret, and generated-file ignores
└── docs/
    ├── API.md
    ├── Architecture.md
    ├── Deployment.md
    ├── Diagrams.md
    ├── FutureIdeas.md
    ├── GitHubPolish.md
    ├── ProjectStructure.md
    ├── Protocol.md
    ├── Security.md
    └── Troubleshooting.md
```

## Installation

Clone the repository and install the Python dependencies:

```bash
pip install fastapi uvicorn google-genai
```

For the ESP32 firmware, install these Arduino libraries:

- `WebSocketsClient`
- `Adafruit NeoPixel`
- `IRremoteESP8266`
- ESP32 board support package

## Configuration

For simplicity during development, some configuration values are currently stored in source files.

Before production or wider deployment, these should be moved to environment variables or external configuration files.

- Wi-Fi SSID/password in `NovaAI.ino`
- server IP and port in `NovaAI.ino`

For open-source use, move these values into environment variables or local config files before publishing secrets. See [.env.example](.env.example) and [docs/Security.md](docs/Security.md).

## Running Locally

Start the Python server:

```bash
python app.py
```

Open the browser interface:

```text
http://localhost:8000
```

The browser client connects to:

```text
ws://localhost:8000/ws
```

The ESP32 connects to:

```text
ws://<server-ip>:8000/esp32
```

## ESP32 Setup

Hardware used by the current firmware:

| Component | Purpose | Pins |
| --- | --- | --- |
| INMP441 | I2S microphone | WS `5`, SCK `18`, SD `32` |
| MAX98357A | I2S speaker amplifier | LRC `19`, BCLK `21`, DIN `22` |
| WS2812B ring | Nova state indicator | DATA `4` |
| IR LED | RGB remote control | DATA `25` |

Upload `NovaAI.ino` with the Arduino IDE after setting:

- Wi-Fi SSID
- Wi-Fi password
- server IP address
- server port

## AI Server Setup

The server creates a Gemini Live session per WebSocket client:

- `/ws` for the browser client
- `/esp32` for the hardware client

Both sessions use audio responses and the `Aoede` voice. The ESP32 session also exposes RGB control tools so Gemini can ask the server to send `RGB_ON`, `RGB_OFF`, `RGB_RED`, `RGB_GREEN`, and `RGB_BLUE` commands to the device.

## Environment Variables

The current code does not yet read environment variables. These are the recommended variables for the next cleanup pass:

```bash
GEMINI_API_KEY=your_gemini_api_key_here
NOVA_HOST=0.0.0.0
NOVA_PORT=8000
NOVA_MODEL=gemini-3.1-flash-live-preview
NOVA_VOICE=Aoede
ESP32_WIFI_SSID=your_wifi_name
ESP32_WIFI_PASSWORD=your_wifi_password
ESP32_SERVER_HOST=192.168.1.100
ESP32_SERVER_PORT=8000
```

## Usage Examples

Browser voice session:

1. Start the server.
2. Open `http://localhost:8000`.
3. Click `Connect & Listen`.
4. Speak naturally.
5. Say `Goodbye` to close the session gracefully.

ESP32 hardware session:

1. Start the Python server on the same network as the ESP32.
2. Upload the firmware with the correct server IP.
3. Power the ESP32.
4. Speak into the INMP441 microphone.
5. Ask Nova to change room lights, for example: `Turn the lights blue`.

## Screenshots

> Suggested screenshots to add:

- Browser voice hub connected
- ESP32 hardware build
- Serial Monitor connection logs
- LED ring in listening, thinking, and speaking states
- RGB light control demo

## Demo Video

> Suggested video timeline:

- `0:00` - Hardware overview
- `0:20` - Server startup
- `0:35` - Browser voice demo
- `1:10` - ESP32 wake and connection
- `1:40` - Real-time voice conversation
- `2:20` - Barge-in interruption
- `2:45` - RGB light control
- `3:10` - Goodbye and sleep flow

## Documentation

- [Architecture](docs/Architecture.md)
- [API](docs/API.md)
- [Protocol](docs/Protocol.md)
- [Deployment](docs/Deployment.md)
- [Troubleshooting](docs/Troubleshooting.md)
- [Project Structure](docs/ProjectStructure.md)
- [Developer Guide](docs/DeveloperGuide.md)
- [Future Ideas](docs/FutureIdeas.md)
- [Security](docs/Security.md)
- [Diagrams](docs/Diagrams.md)
- [GitHub Polish](docs/GitHubPolish.md)

## Future Roadmap

- Split server, prompts, tools, and frontend into separate modules
- Add structured logging
- Add authentication for WebSocket endpoints
- Add a proper memory/session store
- Add health checks and diagnostics
- Add Docker deployment
- Add PlatformIO support
- Add CI for linting and docs checks
- Add audio format negotiation and protocol versioning

## Known Limitations

- API keys and Wi-Fi credentials are currently hardcoded and must be removed before public release.
- WebSocket endpoints are unauthenticated.
- Memory helper functions exist but are not currently wired into the session flow.
- Browser and ESP32 sessions are independent Gemini sessions.
- The ESP32 sleeps after a `SLEEP` command and does not currently implement a wake-word flow.
- Audio tuning values are hardware and room dependent.

## Credits

Built by Parth as an ESP32 + AI voice assistant prototype.

Powered by FastAPI, Gemini Live, ESP32 Arduino, and the open-source embedded hardware ecosystem.

## License

No license file is currently included. MIT is recommended if the goal is broad open-source adoption.
