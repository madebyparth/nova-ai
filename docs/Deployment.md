# Deployment

NovaAI is currently designed for local-network deployment.

## Local Development

Install Python dependencies:

```bash
pip install fastapi uvicorn google-genai
```

Run the server:

```bash
python app.py
```

Open:

```text
http://localhost:8000
```

## Network Deployment

For ESP32 use, the Python server must be reachable from the ESP32 over Wi-Fi.

1. Connect the computer and ESP32 to the same network.
2. Find the computer's local IP address.
3. Set `websocket_server` in `NovaAI.ino` to that IP.
4. Start the Python server on `0.0.0.0:8000`.
5. Upload and boot the ESP32 firmware.

## Firewall

Allow inbound TCP traffic on port `8000` for local-network ESP32 connections.

## Production Hardening Checklist

- Move secrets out of source code.
- Add `.env` loading on the server.
- Add a local config header or generated secrets file for firmware.
- Require authentication for WebSocket endpoints.
- Add origin checks for browser WebSocket traffic.
- Add structured logging.
- Add `/health` and `/version` endpoints.
- Put the server behind a reverse proxy only after WebSocket and TLS settings are tested.
- Avoid exposing the server directly to the public internet.

## Suggested Dockerfile

This project does not currently include Docker support, but a future Dockerfile would be straightforward:

```dockerfile
FROM python:3.12-slim

WORKDIR /app
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY app.py .
EXPOSE 8000
CMD ["python", "app.py"]
```

## Suggested Requirements File

```text
fastapi
uvicorn
google-genai
```

## ESP32 Deployment Notes

The firmware is Arduino-based. For a cleaner open-source setup, PlatformIO is recommended because it can pin board configuration and library versions in `platformio.ini`.
