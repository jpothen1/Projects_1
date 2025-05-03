#ifndef ENGR122_ArduinoBoat_PID_h
#define ENGR122_ArduinoBoat_PID_h

#include <PID_v1.h>

// PID variables
extern double Setpoint;
extern double Input;
extern double Output;

//double Kp_rs = 15, Ki_rs = 0.05, Kd_rs = 0.5; // PID coefficients for stop rotation  (large target like 90 deg)
extern double Kp_rs = 300, Ki_rs = 0.05, Kd_rs = 0.5; // PID coefficients for stop rotation  (small target like 10 deg)
//extern double Kp_s = 100, Ki_s = 0.05, Kd_s = 0.5; // PID coefficients for stop rotation
extern double Kp_f = 100, Ki_f = 0.05, Kd_f = 0.5; // PID coefficients, adjust these based on testing

// Define output limit for stop logic
extern int output_limit = 180;
// Define output limit for moving forward
extern int output_limit_f = 60;

// Create PID object
PID myPID_rstop(&Input, &Output, &Setpoint, Kp_rs, Ki_rs, Kd_rs, DIRECT);
//PID myPID_stop(&Input, &Output, &Setpoint, Kp_s, Ki_s, Kd_s, DIRECT);
PID myPID_forward(&Input, &Output, &Setpoint, Kp_f, Ki_f, Kd_f, DIRECT);

void pid_init(){
  myPID_forward.SetMode(AUTOMATIC); // Turn on the PID
  myPID_forward.SetOutputLimits(-output_limit_f, output_limit_f); // Limit output to adjust speeds within the motor control range
  //myPID_stop.SetMode(AUTOMATIC); // Turn on the PID
  //myPID_stop.SetOutputLimits(-output_limit, output_limit); // Limit output to adjust speeds within the motor control range
  myPID_rstop.SetMode(AUTOMATIC); // Turn on the PID
  myPID_rstop.SetOutputLimits(-output_limit, output_limit); // Limit output to adjust speeds within the motor control range
}




#endif
