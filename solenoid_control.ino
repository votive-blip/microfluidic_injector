const int MEDIA_PIN_1 = 4;
const int BASE_PIN_1 = 3;

const int TIME_PER_CYCLE = 500;  // time in milliseconds for each cycle, cannot be 0, should not be <250
const int NUMBER_OF_WAVES = 1;   // number of waves to make, cannot be 0

const float MEDIA_CONCENTRATION = 0.4;     // concentration of experimental media, cannot be 0
float current_output_concentration = 0.0;  // current output concentration

void setup() {

  pinMode(MEDIA_PIN_1, OUTPUT);
  pinMode(BASE_PIN_1, OUTPUT);

  digitalWrite(MEDIA_PIN_1, LOW);  // set pins to low voltage (valves closed)
  digitalWrite(BASE_PIN_1, LOW);
}

void loop() {

  delay(1500);  // prevents weird hiccup at start of program from IDE upload

  for (byte wave = 0; wave < NUMBER_OF_WAVES; wave++) {  // advances WAVES

    // ** profile functions here **
    
    //FORMAT: ramp(int duration, float initial_conc, float final_conc, optional String "quadratic")
    //        wait(int duration, float set_conc)
    //        repeated_step(int interval, int gap, int steps, float baseline_conc, float elevated_conc)

    ramp(25, 0.1, 0.4);
    wait(2, 0.4);

    // ** end of wave here **
  }

  // ** end of ALL waves here **
  digitalWrite(MEDIA_PIN_1, LOW);
  digitalWrite(BASE_PIN_1, LOW);
  exit(0);  // exit main loop
}


void pulse(float fraction_duty) {  // intended fraction of duty cycle, generates (count em,) 1 combined pulse for 2 valves with duration governed by TIME_PER_CYCLE

  if (fraction_duty == 0) {  // if no media intended, put no media duh
    digitalWrite(MEDIA_PIN_1, LOW);
    digitalWrite(BASE_PIN_1, HIGH);
    delay(TIME_PER_CYCLE);
  }

  else if (fraction_duty >= 1) {  // no turn off media, only on
    digitalWrite(MEDIA_PIN_1, HIGH);
    digitalWrite(BASE_PIN_1, LOW);
    delay(TIME_PER_CYCLE);

  } else {
    digitalWrite(MEDIA_PIN_1, HIGH);
    digitalWrite(BASE_PIN_1, LOW);
    delay((long)(TIME_PER_CYCLE * fraction_duty));  // delays media's % portion of duty cycle, truncated to integer by type conversion
    digitalWrite(MEDIA_PIN_1, LOW);
    digitalWrite(BASE_PIN_1, HIGH);
    delay((long)(TIME_PER_CYCLE * (1 - fraction_duty)));  // delays by base's % portion (inverse of media) of duty cycle, truncated to integer
  }
}

void wait(int duration, float set_conc) {  // duration in minutes
  for (int i = 0; i <= ctime(duration); i++) {
    current_output_concentration = set_conc;
    float fraction_duty = current_output_concentration / MEDIA_CONCENTRATION;
    pulse(fraction_duty);
  }
}

// performs calculations for linear ramp rate
void ramp(int duration, float initial_conc, float final_conc) {  // duration in minutes, overloaded
  for (int i = 0; i <= ctime(duration); i++) {
    current_output_concentration = ((final_conc - initial_conc) / (ctime(duration)) * i + initial_conc);  // linear y = mx + b
    float fraction_duty = current_output_concentration / MEDIA_CONCENTRATION;
    pulse(fraction_duty);
  }
}

void ramp(int duration, float initial_conc, float final_conc, String linetype) {  // duration in minutes, overload of ramp
  if (linetype == "quadratic") {
    for (int i = 0; i <= ctime(duration); i++) {
      current_output_concentration = ((final_conc - initial_conc) / pow(ctime(duration), 2) * pow(i, 2) + initial_conc);  // quadratic y = ax^2 + bx + c
      float fraction_duty = current_output_concentration / MEDIA_CONCENTRATION;
      pulse(fraction_duty);
    }
  }
}

void repeated_step(int interval, int gap, int steps, float baseline_conc, float elevated_conc) {  // this will form a rectangular wave of concentrations
  for (int i = 1; i <= steps; i++) {                                                              
    wait(gap, baseline_conc);                                                                     
    wait(interval, elevated_conc);                                                                
  }
}

int ctime(float time_minutes) {  // converts minutes to cycles
  return (time_minutes * 60000 / TIME_PER_CYCLE);
}
