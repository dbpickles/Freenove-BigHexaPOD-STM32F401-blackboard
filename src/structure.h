
#define SEP     ':'   // Command seperator

#define UNUSED  99
#define KNEE    2
#define HIPY    1
#define HIPX    0
#define FRONT   0
#define MIDDLE  1
#define BACK    2
#define RIGHT   0
#define LEFT    1
#define I2Cx40  0
#define I2Cx41  1


struct ServoSettings
    { 
    char desc[30]     ;       
    int pwm_id        ;     // PWMServoDriver 0 = I2C 0x40, 1 = i2C 0x41
    int servo_id      ;     // Servo number on the HexaPod board 0 > 15
    int servo_min     ;     // Pulse length min - MG996R default  80 - Micro Servo default  70 
    int servo_max     ;     // Pulse length min - MG996R default 480 - Micro servo default 470  
    int side          ;     // 0 = Right 1 = Left Side
    int leg           ;     // 0 = Front 1 = Middle 2 = Back
    int joint         ;     // 0 = HipX 1 = HipY 2 = Knee
    int min_movement  ;     // degrees - Minimum movement    
    int max_movement  ;     // degrees - Maximun movement    
    int default_pos   ;     // degrees - default starting posstion
    int default_sleep ;     // degrees - default sleep position    
    } Servo[32];            // 32 Servos in total.

