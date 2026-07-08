# Future Ideas

These ideas intentionally avoid changing the current behavior. They describe where the project could go next.

## Folder Restructuring

- Move Python server code into a `server/` package.
- Move the embedded HTML into `server/static/index.html`.
- Move the ESP32 sketch into `firmware/NovaAI/`.
- Add `requirements.txt` or `pyproject.toml`.
- Add `platformio.ini` for reproducible firmware builds.

## File Organization

- Extract Gemini prompts into `prompts.py`.
- Extract tool declarations and command mappings into `tools.py`.
- Extract browser and ESP32 WebSocket handlers into separate route modules.
- Keep audio protocol constants in one place.

## Naming

- Rename `/esp32` docs-facing name to "hardware endpoint".
- Rename `nova_is_talking` to `assistant_audio_active`.
- Rename `speaker_primed` to `playback_prebuffer_ready`.
- Rename `full_response_buffer` to `assistant_text_buffer`.

## Modularization

- Create a `GeminiSessionBridge` abstraction for common session setup.
- Create separate browser and hardware bridge classes.
- Add a small command enum for text commands.
- Add a protocol constants module.

## Code Cleanup

- Remove duplicate imports.
- Remove unused `urllib.response` import.
- Remove duplicated prompt lines.
- Move hardcoded configuration to environment variables.
- Replace broad `except Exception` blocks with narrower exception handling where practical.
- Add structured logs instead of raw `print()` calls.

## Security Improvements

- Remove committed secrets.
- Rotate exposed credentials before publishing.
- Add WebSocket authentication.
- Add browser origin checks.
- Add rate limits and maximum frame sizes.
- Keep `.env` and firmware secrets out of Git.

## Performance Improvements

- Make ESP32 ring buffer size configurable.
- Tune chunk sizes based on measured latency.
- Add metrics for queue depth and underruns.
- Avoid printing every partial transcript in production.
- Add binary protocol version negotiation.

## Documentation Improvements

- Add photos of the wiring.
- Add a pinout diagram.
- Add a short demo video.
- Add screenshots of the browser interface.
- Add architecture decision records.
- Add a hardware bill of materials.

## README Improvements

- Add real demo GIF.
- Add repository social preview image.
- Add supported hardware table.
- Add "How it works in 60 seconds" section.
- Add a "Build log" link if this becomes a public project story.
