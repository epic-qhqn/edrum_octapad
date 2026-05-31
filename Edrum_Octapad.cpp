// OctaPad E-Drum Firmware - Pro Edition
// Optimized for LGT8F328P / Arduino Nano

#include <Arduino.h>

// --- 1. SYSTEM CONFIGURATION ---
#define LED_PIN 13
#define PIEZ_CNT 8 
#define DRUM_CHANNEL 9 // Standard MIDI Drum Channel 10 (0-indexed as 9)

// Special pad indices mapping
#define CRASH_PAD_INDEX 0
#define CRASH_CHOKE_PIN 9 
#define HIHAT_PAD_INDEX 4 
#define HIHAT_PEDAL_PIN 10 

// --- 2. SENSITIVITY & DAMPING COMPENSATION ---
// Pad material thickness (cm). Adjust this to calibrate overall sensitivity.
constexpr float MATERIAL_THICKNESS_CM = 1.0; 

// Dynamic Range Compression: Lowers the peak ADC required for max MIDI velocity (127).
// E.g., 1.0cm thickness -> Max ADC drops to ~409, making light hits sound fuller.
constexpr int MAX_HIT_ADC = 1023 / (1 + MATERIAL_THICKNESS_CM * 1.5); 

// --- 3. CROSSTALK (XTALK) FILTERING ---
// Pad physical dimensions (cm) for accurate wave propagation math.
constexpr float PAD_WIDTH_CM       = 14.5;
constexpr float PAD_HEIGHT_CM      = 10.0;
constexpr float SENSOR_DISTANCE_CM = 10.5;

// Strictness multiplier (Higher = filters more aggressively)
constexpr float XTALK_STRICTNESS   = 1.2; 
constexpr uint32_t XTALK_WINDOW_MS = 40; 

// Microseconds to capture the peak wave. Scales dynamically with material thickness.
constexpr uint32_t CAPTURE_WINDOW_US = 500 + (MATERIAL_THICKNESS_CM * 500); 
constexpr uint32_t LED_BLINK_MS      = 10;   

// --- 4. DRUM MAP & HARDWARE PINS ---
// Trigger thresholds (0-1023). Keep low (e.g., 150) for maximum responsiveness.
const int kick_threshold[PIEZ_CNT]      = {150, 150, 150, 150, 150, 150, 150, 150};

// Mute duration (Microseconds) to prevent double-triggering.
const uint32_t kick_mute_time[PIEZ_CNT] = {50000, 50000, 50000, 50000, 50000, 50000, 50000, 50000}; 

// Analog pins connected to Piezo sensors
// Layout: [0]:A3, [1]:A2, [2]:A1, [3]:A0, [4]:A7, [5]:A6, [6]:A5, [7]:A4
const uint8_t kick_pin[PIEZ_CNT]        = {A3, A2, A1, A0, A7, A6, A5, A4};

// Default General MIDI (GM) Drum Map for VSTs like FPC / EZDrummer.
// Tweak these values (35-81) if your DAW triggers unintended sounds.
// [0] 49: Crash Cymbal     [1] 48: Hi-Tom          [2] 45: Mid-Tom     [3] 51: Ride Cymbal
// [4] 46: Hi-Hat Open      [5] 38: Snare Drum      [6] 41: Floor Tom   [7] 36: Kick Drum
const byte kick_note[PIEZ_CNT]          = {49, 48, 45, 51, 46, 38, 41, 36}; 

// --- 5. DEBUGGING ---
// Uncomment the line below to monitor raw ADC data from a specific pin.
// IMPORTANT: Close Hairless MIDI before enabling Debug mode!
//#define DEBUG_TEST_PIN A6

// --- CORE ENGINE VARIABLES ---
constexpr float XTALK_MULTIPLIER = 0.15 * (15.0 / SENSOR_DISTANCE_CM) * XTALK_STRICTNESS;

enum PadState { STATE_IDLE, STATE_CAPTURING, STATE_MUTED };
PadState pad_state[PIEZ_CNT];

int kick_val[PIEZ_CNT]; 
uint32_t state_timer[PIEZ_CNT]; 
uint32_t tm_led = 0;

uint32_t global_last_hit_time = 0;
int global_last_hit_peak = 0;

// Fast MIDI packet transmitter
inline void sendMIDI(int status, int note, int velocity) {
    Serial.write(status);
    Serial.write(note);
    Serial.write(velocity);
}

void setup() {
    Serial.begin(115200); 
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Internal pullups to eliminate floating noise when hardware is disconnected
    pinMode(CRASH_CHOKE_PIN, INPUT_PULLUP);
    pinMode(HIHAT_PEDAL_PIN, INPUT_PULLUP); 
   
    for (byte i = 0; i < PIEZ_CNT; i++) {
        pinMode(kick_pin[i], INPUT);
        pad_state[i] = STATE_IDLE;
        kick_val[i] = 0;
        state_timer[i] = 0;
    }

// Unleash full 10-bit ADC resolution for LGT8F chips
#if defined(__LGT8F__)
    analogReadResolution(10); 
#endif
}

void loop() {
    // Cache timing variables to prevent drift during loop execution
    uint32_t current_micros = micros();
    uint32_t current_millis = millis();

    // 1. Crash Choke Evaluation
    if (pad_state[CRASH_PAD_INDEX] != STATE_MUTED && digitalRead(CRASH_CHOKE_PIN) == LOW) { 
        sendMIDI((0x90 | DRUM_CHANNEL), kick_note[CRASH_PAD_INDEX], 0); // Send Note-Off equivalent
        pad_state[CRASH_PAD_INDEX] = STATE_MUTED; 
        state_timer[CRASH_PAD_INDEX] = current_micros;
    } 

    // 2. Multi-pad Finite State Machine (FSM)
    for (byte i = 0; i < PIEZ_CNT; i++) {
        
        int adc = 0;
        // Bypassing analogRead() while muted maximizes the FSM scanning speed
        if (pad_state[i] != STATE_MUTED) {
            adc = analogRead(kick_pin[i]); 
        }

        switch (pad_state[i]) {
            
            case STATE_IDLE:
                // Strike threshold crossed -> Initialize capture sequence
                if (adc > kick_threshold[i]) {
                    pad_state[i] = STATE_CAPTURING;
                    state_timer[i] = current_micros;
                    kick_val[i] = adc;
                    
                    digitalWrite(LED_PIN, HIGH);
                    tm_led = current_millis;
                }
                break;

            case STATE_CAPTURING:
                // Continuously track the highest peak of the analog waveform
                if (adc > kick_val[i]) {
                    kick_val[i] = adc; 
                }

                // Capture window elapsed -> Analyze and transmit
                if (current_micros - state_timer[i] >= CAPTURE_WINDOW_US) {
                    
                    bool is_crosstalk = false;
                    
                    // Crosstalk logic: Rejects sympathetic vibrations from adjacent pads
                    if (current_millis - global_last_hit_time <= XTALK_WINDOW_MS) {
                        int expected_noise = global_last_hit_peak * XTALK_MULTIPLIER;
                        if (kick_val[i] <= expected_noise) {
                            is_crosstalk = true; 
                        }
                    }

                    // Proceed if the signal is confirmed as a genuine physical strike
                    if (!is_crosstalk) {
                        // Map clamped ADC to standard MIDI velocity [30-127]
                        int velocity = map(kick_val[i], kick_threshold[i], MAX_HIT_ADC, 30, 127);
                        velocity = constrain(velocity, 30, 127); 
                        
                        // Hi-Hat state mapping (Open vs Closed)
                        if (i == HIHAT_PAD_INDEX && digitalRead(HIHAT_PEDAL_PIN) == LOW) { 
                            sendMIDI((0x90 | DRUM_CHANNEL), 42, velocity); // Note 42: Closed Hi-Hat
                        } else {           
                            sendMIDI((0x90 | DRUM_CHANNEL), kick_note[i], velocity); 
                        }

                        // Register global hit signatures for future crosstalk calculations
                        global_last_hit_time = current_millis;
                        global_last_hit_peak = kick_val[i];
                    }

                    // ACTIVE DAMPING: Ground the pin momentarily to flush residual piezo capacitance
                    pinMode(kick_pin[i], OUTPUT); 
                    digitalWrite(kick_pin[i], LOW);

                    pad_state[i] = STATE_MUTED;
                    state_timer[i] = current_micros; 
                }
                break;

            case STATE_MUTED:
                // Cooldown period elapsed -> Restore input mode and await next strike
                if (current_micros - state_timer[i] >= kick_mute_time[i]) {
                    pinMode(kick_pin[i], INPUT); 
                    pad_state[i] = STATE_IDLE;   
                }
                break;
        }
    }

    // 3. Status LED Management
    if (tm_led > 0 && (current_millis - tm_led >= LED_BLINK_MS)) {
        digitalWrite(LED_PIN, LOW);
        tm_led = 0;
    }

#ifdef DEBUG_TEST_PIN
    // 4. Diagnostic Telemetry Engine
    static uint32_t last_debug_print = 0;
    if (current_millis - last_debug_print >= 10) { 
        int debug_val = analogRead(DEBUG_TEST_PIN);
        if (debug_val > 5) { 
            Serial.print("Test Pin ADC (Raw): ");
            Serial.println(debug_val);
        }
        last_debug_print = current_millis;
    }
#endif
}
