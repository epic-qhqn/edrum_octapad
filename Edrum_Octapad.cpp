#define LED 13
#define PIEZ_CNT 8 
#define DRUM_CHANEL 8

#define CRASH 0 
#define PIN_CRASH_STOP 9 
#define KANAL_HAT 1 
#define KNOPKA_HAT 10 

constexpr uint32_t CAPTURE_WINDOW_US = 2000; // 2ms peak sampling window
constexpr uint32_t LED_BLINK_MS = 10;        // LED blink duration

// Configuration arrays
const int kick_treshold[PIEZ_CNT]       = {200, 200, 200, 200, 200, 600, 200, 200};
const uint32_t kick_mute_time[PIEZ_CNT] = {60000, 50000, 50000, 50000, 50000, 50000, 50000, 50000}; 
const byte kick_note[PIEZ_CNT]          = {10, 20, 30, 40, 50, 60, 70, 80}; 
const uint8_t kick_pin[PIEZ_CNT]        = {A0, A1, A2, A3, A4, A5, A6, A7};

// Pad State Machine
enum PadState { STATE_IDLE, STATE_CAPTURING, STATE_MUTED };
PadState pad_state[PIEZ_CNT];

int kick_val[PIEZ_CNT]; 
uint32_t state_timer[PIEZ_CNT]; // Shared timer for Capture and Mute phases
uint32_t tm_led = 0;

void noteOn(int chan, int note, int velocity) {
    Serial.write(0x90 | chan);
    Serial.write(note);
    Serial.write(velocity);
}

void noteOff(int chan, int note, int velocity) {
    Serial.write(0x80 | chan);
    Serial.write(note);
    Serial.write(velocity);
}

void setup() {
    Serial.begin(115200); 
    
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    // INPUT_PULLUP ensures stability even if pedals are unplugged
    pinMode(PIN_CRASH_STOP, INPUT_PULLUP);
    pinMode(KNOPKA_HAT, INPUT_PULLUP); 
   
    for (byte i = 0; i < PIEZ_CNT; i++) {
        pinMode(kick_pin[i], INPUT);
        pad_state[i] = STATE_IDLE;
        kick_val[i] = 0;
        state_timer[i] = 0;
    }

#if defined(__LGT8F__)
    analogReadResolution(10); // Native 10-bit resolution for LGT8F328P
#endif
}

void loop() {
    uint32_t current_micros = micros();
    uint32_t current_millis = millis();

    // 1. Crash Choke Logic
    if (pad_state[CRASH] != STATE_MUTED && digitalRead(PIN_CRASH_STOP) == LOW) { 
        noteOn(DRUM_CHANEL, kick_note[CRASH], 0); 
        pad_state[CRASH] = STATE_MUTED; 
        state_timer[CRASH] = current_micros;
    } 

    // 2. Multi-pad FSM Processing
    for (byte i = 0; i < PIEZ_CNT; i++) {
        
        // Skip ADC reading if muted to save CPU cycles
        int adc = 0;
        if (pad_state[i] != STATE_MUTED) {
            adc = analogRead(kick_pin[i]);
        }

        switch (pad_state[i]) {
            case STATE_IDLE:
                if (adc > kick_treshold[i]) {
                    pad_state[i] = STATE_CAPTURING;
                    state_timer[i] = current_micros;
                    kick_val[i] = adc;
                    
                    digitalWrite(LED, HIGH);
                    tm_led = current_millis;
                }
                break;

            case STATE_CAPTURING:
                // Update peak value
                if (adc > kick_val[i]) {
                    kick_val[i] = adc; 
                }

                // Sampling window ended -> Send MIDI and switch to Mute
                if (current_micros - state_timer[i] >= CAPTURE_WINDOW_US) {
                    int velocity = map(kick_val[i], kick_treshold[i], 1023, 30, 127);
                    velocity = constrain(velocity, 1, 127);
                    
                    // Handle Hi-hat pedal state
                    if (i == KANAL_HAT && digitalRead(KNOPKA_HAT) == LOW) { 
                        noteOn(DRUM_CHANEL, kick_note[i] + 2, velocity); // Closed Hi-hat note
                    } else {           
                        noteOn(DRUM_CHANEL, kick_note[i], velocity); // Open Hi-hat / Normal note
                    }

                    // Active drain to kill residual voltage
                    pinMode(kick_pin[i], OUTPUT); 
                    digitalWrite(kick_pin[i], LOW);

                    pad_state[i] = STATE_MUTED;
                    state_timer[i] = current_micros; 
                }
                break;

            case STATE_MUTED:
                // Restore pin to INPUT after mute time
                if (current_micros - state_timer[i] >= kick_mute_time[i]) {
                    pinMode(kick_pin[i], INPUT); 
                    pad_state[i] = STATE_IDLE;   
                }
                break;
        }
    }

    // 3. Status LED control
    if (tm_led > 0 && (current_millis - tm_led >= LED_BLINK_MS)) {
        digitalWrite(LED, LOW);
        tm_led = 0;
    }
}
