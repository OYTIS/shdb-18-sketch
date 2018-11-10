enum animation {
  HEARTBEAT,
  WAITING,
  EMPTY,
};

void push_animation(enum animation an);

struct animation_frame {
  const byte* frame; // 8 bytes
  float brightness;
  long duration;
};

const byte emptyHeart[][8] = {
    {B01100110,
     B10011001,
     B10000001,
     B10000001,
     B01000010,
     B00100100,
     B00011000,
     B00000000}};

// Heartbeat animation
const byte heartbeat[][8] = {
    {B01100110,
     B10011001,
     B10000001,
     B10011001,
     B01011010,
     B00100100,
     B00011000,
     B00000000},
    {B01100110,
     B10011001,
     B10111101,
     B10111101,
     B01111110,
     B00100100,
     B00011000,
     B00000000},
    {B01100110,
     B11111111,
     B11111111,
     B11111111,
     B01111110,
     B00111100,
     B00011000,
     B00000000},
    {B01100110,
     B10011001,
     B10000001,
     B10000001,
     B01000010,
     B00100100,
     B00011000,
     B00000000}
};

const byte waitingHeartBytes[][8] = {
    {B01000100,
     B00010001,
     B10000000,
     B00000001,
     B01000000,
     B00000100,
     B00010000,
     B00000000},
    {B00100010,
     B10001000,
     B00000001,
     B10000000,
     B00000010,
     B00100000,
     B00001000,
     B00000000}};

const struct animation_frame heartbeat_animation[] = {
    {heartbeat[0], 1, 50},
    {heartbeat[1], 5, 50},
    {heartbeat[2], 7, 100},
    {heartbeat[3], 0.4, 50},
};

const struct animation_frame waiting_animation[] = {
    {waitingHeartBytes[0], 0.01, 100},
    {waitingHeartBytes[1], 0.01, 100},
};

//Wating animation
