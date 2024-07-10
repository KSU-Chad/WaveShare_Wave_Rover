
// #define HEART_BEAT 3000
// long lastCmdTime;

// // the MAC address of the device you want to ctrl.
// uint8_t broadcastAddress[] = {0x08, 0x3A, 0xF2, 0x93, 0x5F, 0xA8};


// === === === MOTOR PIN DEFINITION === === ===
const uint16_t PWMA = 25;  // Motor A PWM control  Orange
const uint16_t AIN2 = 17;  // Motor A input 2      Brown
const uint16_t AIN1 = 21;  // Motor A input 1      Green
const uint16_t BIN1 = 22;  // Motor B input 1       Yellow
const uint16_t BIN2 = 23;  // Motor B input 2       Purple
const uint16_t PWMB = 26;  // Motor B PWM control   White

int freq = 5000;

const int ledChannel1 = 2;
const int ledChannel2 = 4;
const int resolution = 8;

DifferentialSteering DiffSteer;
int fPivYLimit = 20;
int idle = 0;

void pinInit();

void motorDrive(int speed, int channel, int motorPin1, int motorPin2);
void differential_steer(int XValue, int YValue) ;