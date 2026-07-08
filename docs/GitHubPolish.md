# GitHub Polish

## Repository Description

Real-time ESP32 voice assistant that streams I2S microphone audio to Gemini Live, plays AI speech responses, and controls RGB room lighting over IR.

## Repository Tagline

An ESP32-powered voice AI assistant with real-time Gemini Live audio, WebSockets, I2S playback, and physical room control.

## Suggested GitHub Topics

- `esp32`
- `gemini-live`
- `voice-assistant`
- `fastapi`
- `websockets`
- `i2s`
- `arduino`
- `embedded-ai`
- `real-time-audio`
- `home-automation`
- `generative-ai`
- `hardware`

## Suggested Release Version

`v0.1.0-alpha`

Reasoning:

- the project has a working prototype shape
- APIs are not yet stabilized
- secrets/config still need cleanup before a production release

## Suggested Badges

```markdown
![Python](https://img.shields.io/badge/Python-3.10+-3776AB?style=for-the-badge&logo=python&logoColor=white)
![FastAPI](https://img.shields.io/badge/FastAPI-WebSocket-009688?style=for-the-badge&logo=fastapi&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-I2S%20Voice-000000?style=for-the-badge&logo=espressif&logoColor=white)
![Gemini](https://img.shields.io/badge/Gemini-Live%20API-4285F4?style=for-the-badge&logo=google&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)
```

## Suggested Screenshots

- Browser voice dashboard connected
- ESP32 wiring top-down photo
- LED ring listening state
- LED ring speaking state
- RGB room light control result
- Serial monitor showing connection and tool calls
- Terminal showing FastAPI/Gemini bridge logs

## Suggested Demo GIF

Recommended length: 8 to 12 seconds.

Show:

1. user says "Nova, turn the lights blue"
2. server logs tool call
3. room lights change
4. Nova responds by voice

## Suggested Demo Video Timestamps

| Time | Segment |
| --- | --- |
| `0:00` | What NovaAI is |
| `0:15` | Hardware parts |
| `0:35` | Server startup |
| `0:50` | Browser voice test |
| `1:20` | ESP32 connection |
| `1:45` | Conversation demo |
| `2:20` | Barge-in demo |
| `2:45` | RGB light control |
| `3:10` | Goodbye and sleep |
| `3:25` | Architecture summary |

## Open-Source Readiness Checklist

- Add real license file.
- Rotate all exposed secrets.
- Replace hardcoded credentials.
- Add requirements file.
- Add PlatformIO project metadata.
- Add wiring diagram image.
- Add demo GIF.
- Add contribution guidelines if accepting PRs.
