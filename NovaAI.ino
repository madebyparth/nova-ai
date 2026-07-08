#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <driver/i2s.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

// NETWORK
const char* ssid = "your_wifi_name";
const char* password = "your_wifi_password";

const char* websocket_server = "192.168.29.232";
const uint16_t websocket_port = 8000;

WebSocketsClient webSocket;

// MIC (INMP441)
#define MIC_WS   5
#define MIC_SCK  18
#define MIC_SD   32

// SPEAKER (MAX98357A)
#define SPK_LRC  19
#define SPK_BCLK 21
#define SPK_DIN  22
#define BUFFER_SAMPLES 128

// RING LED (WS2812B 12 LED Ring)
#define LED_PIN 4
#define NUM_LEDS 12

//IR
#define IR_PIN 25
IRsend irsend(IR_PIN);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

int16_t audio_buffer[BUFFER_SAMPLES];

// STATE
volatile bool nova_is_talking = false;
unsigned long last_speaker_time = 0;

// JITTER BUFFER
#define RING_BUFFER_SIZE (32 * 1024)
uint8_t ring_buffer[RING_BUFFER_SIZE];
volatile size_t ring_head = 0;
volatile size_t ring_tail = 0;
volatile size_t ring_count = 0;
SemaphoreHandle_t ring_mutex;

#define PREBUFFER_BYTES (6000)
volatile bool speaker_primed = false;

enum NovaState {
  BOOTING,
  LISTENING,
  THINKING,
  SPEAKING,
  SLEEPING
};

NovaState novaState = BOOTING;
volatile bool is_sleeping = false;

void ring_buffer_write(const uint8_t* data, size_t len) {
  if (xSemaphoreTake(ring_mutex, portMAX_DELAY) == pdTRUE) {
    for (size_t i = 0; i < len; i++) {
      if (ring_count >= RING_BUFFER_SIZE) {
        ring_tail = (ring_tail + 1) % RING_BUFFER_SIZE;
        ring_count--;
      }
      ring_buffer[ring_head] = data[i];
      ring_head = (ring_head + 1) % RING_BUFFER_SIZE;
      ring_count++;
    }
    xSemaphoreGive(ring_mutex);
  }
}

size_t ring_buffer_read(uint8_t* out, size_t max_len) {
  size_t read_len = 0;
  if (xSemaphoreTake(ring_mutex, portMAX_DELAY) == pdTRUE) {
    read_len = (max_len < ring_count) ? max_len : ring_count;
    for (size_t i = 0; i < read_len; i++) {
      out[i] = ring_buffer[ring_tail];
      ring_tail = (ring_tail + 1) % RING_BUFFER_SIZE;
    }
    ring_count -= read_len;
    xSemaphoreGive(ring_mutex);
  }
  return read_len;
}

size_t ring_buffer_available() {
  size_t avail = 0;
  if (xSemaphoreTake(ring_mutex, portMAX_DELAY) == pdTRUE) {
    avail = ring_count;
    xSemaphoreGive(ring_mutex);
  }
  return avail;
}

void ring_buffer_clear() {
  if (xSemaphoreTake(ring_mutex, portMAX_DELAY) == pdTRUE) {
    ring_head = 0;
    ring_tail = 0;
    ring_count = 0;
    xSemaphoreGive(ring_mutex);
  }
  speaker_primed = false;
}

// SPEAKER TASK
void speakerTask(void* param) {
  uint8_t play_chunk[1024];

  for (;;) {
    if (!speaker_primed) {
      if (ring_buffer_available() >= PREBUFFER_BYTES) {
        speaker_primed = true;
      } else {
        vTaskDelay(pdMS_TO_TICKS(5));
        continue;
      }
    }

    size_t got = ring_buffer_read(play_chunk, sizeof(play_chunk));

    if (got > 0) {
      nova_is_talking = true;
      last_speaker_time = millis();
      size_t bytes_written;
      i2s_write(I2S_NUM_1, play_chunk, got, &bytes_written, portMAX_DELAY);
    } else {
      memset(play_chunk, 0, sizeof(play_chunk));
      size_t bytes_written;
      i2s_write(I2S_NUM_1, play_chunk, sizeof(play_chunk), &bytes_written, portMAX_DELAY);
      speaker_primed = false;
    }
  }
}

// I2S SETUP
void setup_i2s() {
  // ----- MIC -----
  i2s_config_t mic_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 128,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t mic_pins = {
    .mck_io_num = I2S_PIN_NO_CHANGE,
    .bck_io_num = MIC_SCK,
    .ws_io_num = MIC_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = MIC_SD
  };

  i2s_driver_install(I2S_NUM_0, &mic_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &mic_pins);

  // ----- SPEAKER -----
  i2s_config_t spk_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 24000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 16,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };

  i2s_pin_config_t spk_pins = {
    .mck_io_num = I2S_PIN_NO_CHANGE,
    .bck_io_num = SPK_BCLK,
    .ws_io_num = SPK_LRC,
    .data_out_num = SPK_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM_1, &spk_config, 0, NULL);
  i2s_set_pin(I2S_NUM_1, &spk_pins);
}

// WEBSOCKET EVENTS
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch(type) {
    case WStype_CONNECTED:
      Serial.println("[+] Successfully Linked to Python Brain!");
      novaState = LISTENING;
      nova_is_talking = false;
      ring_buffer_clear();
      break;

    case WStype_DISCONNECTED:
      Serial.println("[-] Disconnected from Brain.");
      nova_is_talking = false;
      novaState = BOOTING;
      ring_buffer_clear();
      break;

    case WStype_TEXT: {

      String msg = String((char*)payload).substring(0, length);

      Serial.print("[TEXT CMD] ");
      Serial.println(msg);

      if (msg == "CLEAR_AUDIO") {

          Serial.println("[i] Barge-in: clearing speaker buffer.");
          ring_buffer_clear();

      }

      else if (msg == "SLEEP") {

          Serial.println("[i] AI commanded sleep.");
          novaState = SLEEPING;
          is_sleeping = true;
          webSocket.disconnect();

      }

      // RGB LIGHT CONTROLS

      else if (msg == "RGB_ON") {

          irsend.sendNEC(0xF740BF);
          Serial.println("RGB ON");

      }

      else if (msg == "RGB_OFF") {

          irsend.sendNEC(0xF7C03F);
          Serial.println("RGB OFF");

      }

      else if (msg == "RGB_RED") {

          irsend.sendNEC(0xF720DF);
          Serial.println("RGB RED");

      }

      else if (msg == "RGB_GREEN") {

          irsend.sendNEC(0xF7A05F);
          Serial.println("RGB GREEN");

      }

      else if (msg == "RGB_BLUE") {

          irsend.sendNEC(0xF7609F);
          Serial.println("RGB BLUE");

      }

      break;
    }

    case WStype_BIN:
      if (length > 0) {
        novaState = SPEAKING;
        ring_buffer_write(payload, length);
      }
      break;
  }
}

// LED UPDATE LOGIC
void updateLEDs() {
  static int brightness = 20;
  static int direction = 5;

  switch(novaState) {
    case LISTENING:
      // Solid Green
      for(int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 255, 0));
      }
      break;

    case THINKING:
      // Solid Yellow
      for(int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 0));
      }
      break;

    case SPEAKING:
      brightness += direction;
      if(brightness >= 200) {
          brightness = 200;
          direction = -5;
      }
      if(brightness <= 20) {
          brightness = 20;
          direction = 5;
      }
      for(int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(brightness / 2, 0, brightness));
      }
      break;

    case SLEEPING:
      strip.clear();
      break;

    default:
      for(int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 255));
      }
      break;
  }

  strip.show();
}

// SETUP
void setup() {
  strip.begin();
  strip.setBrightness(40);
  
  for(int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 255));
  }
  strip.show();

  Serial.begin(115200);
  Serial.println("\n>>> Booting Nova Core...");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n[+] Wi-Fi Connected!");
  novaState = LISTENING;
  setup_i2s();
  irsend.begin();
  Serial.println("[+] Audio Hardware Initialized");

  ring_mutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    speakerTask,
    "SpeakerTask",
    4096,
    NULL,
    2,
    NULL,
    1
  );

  webSocket.begin(websocket_server, websocket_port, "/esp32");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

// LOOP WITH SMART NOISE GATE
void loop() {
  updateLEDs();
  if (is_sleeping) {
    delay(100);
    return; 
  }
  webSocket.loop();

  if (nova_is_talking && (millis() - last_speaker_time > 500)) {
      nova_is_talking = false;
      novaState = LISTENING;
  }

  size_t bytes_read = 0;

  i2s_read(
    I2S_NUM_0,
    audio_buffer,
    sizeof(audio_buffer),
    &bytes_read,
    portMAX_DELAY
  );

  if (webSocket.isConnected() && bytes_read > 0) {
      int16_t peak_amplitude = 0;
      int sample_count = bytes_read / 2;

      for (int i = 0; i < sample_count; i++) {
          int16_t abs_sample = abs(audio_buffer[i]);
          if (abs_sample > peak_amplitude) {
              peak_amplitude = abs_sample;
          }
      }

      const int16_t THRESHOLD_IDLE = 600;    
      const int16_t THRESHOLD_TALKING = 7000; 

      bool should_send_audio = false;

      if (nova_is_talking) {
          if (peak_amplitude > THRESHOLD_TALKING) {
              should_send_audio = true;
          }
      } else {
          if (peak_amplitude > THRESHOLD_IDLE) {
              should_send_audio = true;
          }
      }

      if (should_send_audio) {
          if(!nova_is_talking) {
              novaState = THINKING;
          }
          webSocket.sendBIN(
            (uint8_t*)audio_buffer,
            bytes_read
          );
      }
  }
}