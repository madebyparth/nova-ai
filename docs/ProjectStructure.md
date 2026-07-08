# Project Structure

The repository is intentionally small.

```text
NovaAI/
├── app.py
├── NovaAI.ino
├── .env.example
├── .gitignore
├── README.md
└── docs/
```

## `app.py`

Contains:

- FastAPI app creation
- embedded browser HTML
- browser WebSocket endpoint at `/ws`
- ESP32 WebSocket endpoint at `/esp32`
- Gemini Live session setup
- tool declarations
- audio forwarding logic
- browser audio interruption commands
- ESP32 audio pacing and RGB command mapping
- unused `memory.json` helper functions

## `NovaAI.ino`

Contains:

- Wi-Fi and WebSocket setup
- INMP441 microphone I2S configuration
- MAX98357A speaker I2S configuration
- WS2812B LED state display
- IR RGB control
- fixed-size audio ring buffer
- FreeRTOS speaker playback task
- microphone noise gate
- reconnect interval configuration
- sleep/disconnect behavior

## Recommended Future Structure

Without changing behavior, the project could be reorganized into:

```text
NovaAI/
├── server/
│   ├── app.py
│   ├── config.py
│   ├── prompts.py
│   ├── tools.py
│   ├── routes/
│   │   ├── browser.py
│   │   └── esp32.py
│   └── static/
│       └── index.html
├── firmware/
│   └── NovaAI/
│       └── NovaAI.ino
├── docs/
├── README.md
├── requirements.txt
└── .env.example
```

## Why The Current Structure Works

The current two-file structure is excellent for fast prototyping:

- low setup friction
- easy mental model
- no framework ceremony
- all runtime behavior visible in one Python file and one firmware file

For a showcase repository, documentation now fills the gap by explaining the system clearly without forcing a refactor.
