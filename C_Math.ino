//Function to convert from degrees to radians
float toRad(float convert){
    return convert*PI/180;
}

//Function to convert from radians to degrees
float toDeg(float convert){    
    return convert*180/PI;
}

//Function to convert a given Euler rotation to a quaternion
imu::Quaternion euler_to_quat(double pitch, double roll, double yaw){ 
//REMEMBER TO CONVERT TO RADIANS
  
//Abbreviations for the various angular functions
  double cy = cos(yaw * 0.5);
  double sy = sin(yaw * 0.5);
  double cr = cos(roll * 0.5);
  double sr = sin(roll * 0.5);
  double cp = cos(pitch * 0.5);
  double sp = sin(pitch * 0.5);

//Calculate the quaternion parameters
  double w = cy * cr * cp + sy * sr * sp;
  double x = cy * sr * cp - sy * cr * sp;
  double y = cy * cr * sp + sy * sr * cp;
  double z = sy * cr * cp - cy * sr * sp;

//Assign parameters to quaternion object
  imu::Quaternion q(w,x,y,z);
  return q;
}

//Function to calculate the error quaternion based on two input quaternions
imu::Quaternion calculate_error_quat(imu::Quaternion q_ref, imu::Quaternion q_current){   
   imu::Quaternion q_conj = q_current.conjugate();
   imu::Quaternion error_quat = q_ref*(q_conj);
   return error_quat;
}

/*PID class, used in each of the PIDs implemented in the code
  Inputs: timestep for integration, maximum and minimum limits, gains Kp, Kd and Ki, and a maximum integral term.
  Methods: calculate_output(float, float) - returns the output of a PID based on the error and the rate of change of the input
  reset_integrator() - void, resets integral output and integral to 0 when called
*/
 
class PIDLoop{
    public:
        PIDLoop(float dt, int maximum, int minimum, float Kp, float Kd, float Ki, float I_MAX);
        float calculate_output(float error, float rate);
        void reset_integrator();
    private:
        float _dt;
        int _maximum;
        int _minimum;
        float _Kp;
        float _Kd;
        float _Ki;
        float _pre_error;
        float _integral;
        float _I_MAX;
        float Iout;
};

PIDLoop::PIDLoop(float dt, int maximum, int minimum, float Kp, float Kd, float Ki, float I_MAX):
    _dt(dt),
    _maximum(maximum),
    _minimum(minimum),
    _Kp(Kp),
    _Kd(Kd),
    _Ki(Ki),
    _pre_error(0),
    _integral(0),
    _I_MAX(I_MAX){}

float PIDLoop::calculate_output(float error, float rate){
    
// Proportional term
    float Pout = _Kp * error;

// Integral term
        _integral += error * _dt;
        Iout = _Ki * _integral;

//Clamp the integral term
        if(Iout > _I_MAX){
            Iout = _I_MAX;
        }
        
        if(Iout < -_I_MAX){
            Iout = -_I_MAX;
        } 

 // Derivative term
    float derivative = rate;
    float Dout = _Kd * derivative;

// Calculate total output
    float output = Pout + Iout + Dout;

// Restrict to max/min
    if(output > _maximum )
        output = _maximum;
    else if( output < _minimum )
        output = _minimum;

// Save error to previous error
    _pre_error = error;

    return output;
}

void PIDLoop::reset_integrator(){
    Iout = 0;
    _integral = 0;
}


//Calculate the yaw based on an input quaternion
float calculate_gamma(imu::Quaternion q){
    float x = q.x()*q.y() + q.z()*q.w();
    float gamma;

      if(x > 0.499){
          gamma = 2*atan2(q.x(),q.w());
      }
      
      if(x < -0.499){
          gamma = -2*atan2(q.x(),q.w());
      }
      
      else{
        float sqx = q.x()*q.x();
        float sqy = q.y()*q.y();
        float sqz = q.z()*q.z();

        gamma  = atan2(2*q.x()*q.w() - 2*q.x()*q.z(), 1 - 2*sqy - 2*sqz);
      }
      
      return gamma;
}

//Detect a discontinuity in the yaw
boolean detect_discontinuity(float desired_yaw_previous, float desired_yaw){
  
    if(abs(desired_yaw - desired_yaw_previous) > 300){
      return true;
    }
      return false;
}








