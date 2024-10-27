#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#define IN1 23 // GPIO 23
#define IN2 24 // GPIO 24
#define IN3 25 // GPIO 25
#define IN4 16 // GPIO 16

#define STEPS_PER_REV 2048 // Full revolution for 28BYJ-48

void initializeMotor();
void rotateMotor(int steps, int direction);

#endif // MOTOR_CONTROL_H
