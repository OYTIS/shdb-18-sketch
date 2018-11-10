
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>

const int OUTPUT_TYPE = SERIAL_PLOTTER;

const int PULSE_INPUT = A0;
const int PULSE_BLINK = 13;    // Pin 13 is the on-board LED
const int PULSE_FADE = 5;
const int THRESHOLD = 400;   // Adjust this number to avoid noise when idle

//char receive_string[] = {};
const int PULSE_INCOMING = 7; // ZUm anderen Arduino
const int PULSE_OUT = 6;     // Kommend vom anderen Arduino

const int VIBROTOR_OUT = 8;


PulseSensorPlayground pulseSensor;

const int PIN_SPEAKER = 2;    // speaker on pin2 makes a beep with heartbeat

#include "LedControl.h"
#include "heart.h"

/*
  LED
*/

LedControl lc = LedControl(12, 10, 11, 2); // Pins: DIN,CLK,CS, # of Display connected

unsigned long animation_ts;
bool animation_finished = true;

void setup() {

  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.fadeOnPulse(PULSE_FADE);

  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  // Now that everything is ready, start reading the PulseSensor signal.
  if (!pulseSensor.begin()) {

    for (;;) {
      // Flash the led to show things didn't work.
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
      digitalWrite(PULSE_BLINK, LOW);
      delay(50);
    }
  }
  pinMode(PULSE_INCOMING, INPUT);
  pinMode(PULSE_OUT, OUTPUT);
  digitalWrite(PULSE_OUT, LOW);
  pinMode(VIBROTOR_OUT, OUTPUT);
  digitalWrite(VIBROTOR_OUT, LOW);

  lc.shutdown(0, false); // Wake up displays
  lc.shutdown(1, false);
  lc.setIntensity(0, 1); // Set intensity levels
  lc.setIntensity(1, 1);
  lc.clearDisplay(0); // Clear Displays
  lc.clearDisplay(1);
}

//  Take values in Arrays and Display them
void drawImg(const byte byteArray[])
{
  for (int i = 0; i < 8; i++)
  {
    lc.setRow(0, i, byteArray[i]);
  }
}

void animate(const animation_frame *animation, int num_frames) {
  static int frame = 0;
  static unsigned long ts = 0;

  if (millis() - ts < animation[frame].duration) {
    return;
  }

  lc.setIntensity(0, animation[frame].brightness);
  lc.setIntensity(1, animation[frame].brightness);
  drawImg(animation[frame].frame);
  if(++frame == num_frames) {
    animation_finished = true;
    frame = 0;
  }
}

#define ANIMATE(anim) animate(anim, (sizeof((anim))/sizeof((anim)[0])))

// ========== ANIMATIONLOGIC ========== //
// if animation in loop, then do that otherwise,
// basic animation

const int max_animations = 20;
int first_animation = 0;
int last_animation = 0;
int num_animations = 0;
boolean experience_started = false;

animation animationEvents[max_animations];
void push_animation(enum animation an) {
  if (num_animations >= max_animations) {
    return;
  }

  animationEvents[last_animation] = an;
  last_animation++;
  if (last_animation == max_animations) {
    last_animation = 0;
  }
  num_animations++;
}

animation pop_animation() {
  if (num_animations == 0) {
    return WAITING;
  }

  animation ret = animationEvents[first_animation];
  first_animation++;
  if (first_animation == max_animations) {
    first_animation = 0;
  }

  num_animations--;

  return ret;
}

bool remote_heartbeat_detected() {
  static int beat_prev = LOW;
  int beat = digitalRead(PULSE_INCOMING);
  bool res = false;

  if ((beat != beat_prev) && (beat == HIGH)) {
    res = true;
  }
  beat_prev = beat;

  return res;
}

unsigned long process_remote_heartbeat_ts = 0;
bool process_remote_heartbeat_done = true;
void process_remote_heartbeat_start() {
  if (!process_remote_heartbeat_done) {
    Serial.print("Remote heartbeat lost");
    return;
  }

  process_remote_heartbeat_ts = millis(); 
  digitalWrite(PULSE_BLINK, HIGH);
  digitalWrite(VIBROTOR_OUT, HIGH);
  tone(PIN_SPEAKER, 200);             // tone(pin,frequency)
  push_animation(HEARTBEAT);

  process_remote_heartbeat_done = false;
}

void process_remote_heartbeat_loop() {
  if (millis() - process_remote_heartbeat_ts < 50) {
    return;
  }
  digitalWrite(PULSE_BLINK, LOW);
  digitalWrite(VIBROTOR_OUT, LOW);
  noTone(PIN_SPEAKER);

  process_remote_heartbeat_done = true;
}

bool local_heartbeat_detected () {
  return pulseSensor.sawStartOfBeat();
}

unsigned long process_local_heartbeat_ts = 0;
bool process_local_heartbeat_done = true;
void process_local_heartbeat_start() {
  if (!process_local_heartbeat_done) {
    Serial.print("Local heartbeat lost");
    return;
  }

  process_local_heartbeat_ts = millis(); 
  digitalWrite(PULSE_OUT, HIGH);

  process_local_heartbeat_done = false;
}

void process_local_heartbeat_loop () {
  if (millis() - process_local_heartbeat_ts < 50) {
    return;
  }
  digitalWrite(PULSE_OUT, LOW);
  process_local_heartbeat_done = true;
}

void loop() {
  // ========== TRIGGERING ANIMATIONS ========== //
  static animation nextAnimation;

  if (animation_finished) {
    nextAnimation = pop_animation();
    animation_finished = false;
  }

  switch (nextAnimation)
  {
    case HEARTBEAT:
      ANIMATE(heartbeat_animation);
      break;
    case WAITING:
      ANIMATE(waiting_animation);
      break;
  }

  if (remote_heartbeat_detected()) {
    process_remote_heartbeat_start();
  }

  if (local_heartbeat_detected()) {
    process_local_heartbeat_start();
  }

  process_remote_heartbeat_loop();
  process_local_heartbeat_loop();
}
