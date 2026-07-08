# Security

NovaAI is currently a local prototype. Before publishing or deploying it beyond a private network, address the items below.

## Critical Issues

### Hardcoded Wi-Fi Credentials

`NovaAI.ino` currently includes Wi-Fi credentials. These should not be published.

Recommended action:

1. Move credentials to a local `secrets.h` file.
2. Add `secrets.h` to `.gitignore`.
3. Provide `secrets.example.h` for contributors.

## WebSocket Security

The current WebSocket endpoints are unauthenticated.

Recommended improvements:

- require a local token for `/ws` and `/esp32`
- reject unknown origins for browser clients
- limit frame size
- disconnect idle clients
- add per-client rate limits

## Local Network Exposure

The server binds to `0.0.0.0`, which makes it available on the local network. This is required for ESP32 access, but it also means other devices on the network may reach it.

Recommended improvements:

- run only on trusted networks
- use a firewall rule limited to the ESP32 device IP
- avoid port-forwarding this service to the internet

## AI Tool Safety

Gemini tool calls are mapped to hardware commands. Today the available commands are low-risk RGB lighting commands and session termination.

If future tools control higher-impact devices, add:

- allowlisted commands
- confirmation for risky actions
- audit logs
- hardware failsafes

## Data Privacy

Audio is streamed to Gemini Live. Users should understand that speech audio leaves the local device and is processed by the AI provider.

Recommended README note:

> NovaAI streams microphone audio to Gemini Live for real-time AI responses. Do not use it for private conversations unless you are comfortable with that processing model.

## Publishing Checklist

- Rotate API key.
- Remove real Wi-Fi credentials.
- Check commit history for secrets.
- Add `.env.example`.
- Add firmware `secrets.example.h` if firmware config is extracted.
- Add a license.
- Add a security policy if accepting public contributions.
