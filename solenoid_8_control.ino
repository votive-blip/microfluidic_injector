const byte BASE_PIN_1 = 3;
const byte MEDIA_PIN_1 = 4;
const byte BASE_PIN_2 = 5;
const byte MEDIA_PIN_2 = 6;
const byte BASE_PIN_3 = 7;
const byte MEDIA_PIN_3 = 8;
const byte BASE_PIN_4 = 9;
const byte MEDIA_PIN_4 = 10;

const byte VALVE_PAIRS[4][2] = { { BASE_PIN_1, MEDIA_PIN_1 }, { BASE_PIN_2, MEDIA_PIN_2 }, { BASE_PIN_3, MEDIA_PIN_3 }, { BASE_PIN_4, MEDIA_PIN_4 } };
const byte VALVE_PAIRS_ENABLED = 4;  // number of valve pairs we have
const byte TASKS = 10;               // maximum number of tasks to carry out

// if SCHEDULES is type byte (to save memory), single task times cannot be longer than 4.25hrs and concentrations cannot be above 2.55M unless normalized to percents. 
byte SCHEDULES[VALVE_PAIRS_ENABLED][TASKS][4];               // for each of 4 pins, each set of 4 values is a task, initialized empty as 10 tasks - default task will be to do absolutely nothing. 4 values are profile, duration, initial, final
byte scheduled_items[VALVE_PAIRS_ENABLED] = { 0 };           // # of items that have been added to each valve pair's list of tasks
byte current_task_index[VALVE_PAIRS_ENABLED] = { 0 };        // index of schedule in SCHEDULES[4][10]
uint32_t task_start_time[VALVE_PAIRS_ENABLED] = { 0 };   // time a task starts in ms, reset to 0 each task
uint32_t cycle_start_time[VALVE_PAIRS_ENABLED] = { 0 };  // time a cycle starts in ms, reset to 0 each cycle
uint16_t current_cycles[VALVE_PAIRS_ENABLED] = { 0 };    // total time in cycles for a task, reset to 0 each task

const unsigned short TIME_PER_CYCLE = 500;                        // time in milliseconds for each cycle, cannot be 0
float media_cycle_portion[VALVE_PAIRS_ENABLED] = { 0 };           // portion of cycle dedicated to outputting media for each valve pair
float current_output_concentration[VALVE_PAIRS_ENABLED] = { 0 };  // current output concentration for each pair of valves

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





// ** set concentration of input media **
const float MEDIA_CONCENTRATION[VALVE_PAIRS_ENABLED] = { 0.4, 0.4, 0.4, 0.4 };  // concentration of media for each pair of valves, cannot be 0
// ** set concentration of input media **

void setup() {
  delay(2000); // delay to prevent upload hiccup disrupting valve control at start
  
  // ** setup schedule here, maximum of 10 tasks per valve pair **

  // flat(valve pair # (0-3), duration (minutes), desired concentration);
  // linear_ramp(valve pair # (0-3), duration (minutes), initial concentration, final concentration);
  // quadratic_ramp(valve pair # (0-3), duration (minutes), initial concentration, final concentration);

  // ** valve pair 0 **
  flat(0, 1, 0.2);

  // ** valve pair 1 **
    linear_ramp(1, 1, 0.0, 0.4);

  // ** valve pair 2 **
  flat(2, 1, 0.4);


  // ** valve pair 3 **
  quadratic_ramp(3, 1, 0.1, 0.4);
  flat(3, 1, 0.1);
  linear_ramp(3, 1, 0.0, 0.4);

  

  // sets all pins to output mode and closes valves
  for (byte valve_pair = 0; valve_pair < VALVE_PAIRS_ENABLED; valve_pair++) {
    pinMode(VALVE_PAIRS[valve_pair][0], OUTPUT);
    pinMode(VALVE_PAIRS[valve_pair][1], OUTPUT);

    digitalWrite(VALVE_PAIRS[valve_pair][0], LOW);  // set all pins to low voltage (valves closed)
    digitalWrite(VALVE_PAIRS[valve_pair][1], LOW);
    cycle_start_time[valve_pair] = { millis() }; // set start time for first cycles and tasks to current time
    task_start_time[valve_pair] = { millis() };
  }

  pinMode(LED_BUILTIN, OUTPUT);

}

void loop() {

  for (byte valve_pair = 0; valve_pair < VALVE_PAIRS_ENABLED; valve_pair++) {  // iterates over each of the 4 pairs of valves

    if (current_task_index[valve_pair] < TASKS) {  // until we finish our maximum list of tasks

      if (millis() - cycle_start_time[valve_pair] >= TIME_PER_CYCLE) {  // if cycle is done, update cycle count and begin new cycle
        cycle_start_time[valve_pair] = millis();
        current_cycles[valve_pair]++; // incremented to track current progress through a task and adjust calculation
      }

      // this evaluates to true for non-activated valves, including finished valve_pairs (because empty tasks have length 0), resulting in rapid conclusion of non-filled schedules.
      if (millis() - task_start_time[valve_pair] >= mtime(SCHEDULES[valve_pair][current_task_index[valve_pair]][1])) {  // conclude a task if it reaches its time limit and initiate the next one
        current_task_index[valve_pair]++;
        task_start_time[valve_pair] = millis();
        current_cycles[valve_pair] = 0;
      }

      // select correct calculation for given schedule
      switch (SCHEDULES[valve_pair][current_task_index[valve_pair]][0]) {
        case 'F': // these char values are interpreted as integers
          pulse(valve_pair, flat_calc(valve_pair, (float)SCHEDULES[valve_pair][current_task_index[valve_pair]][2] / 100));  // division converts stored integer value for concentration back to float
          break;
        case 'L': // SCHEDULES[x][x][2] and [x][x][3] are initial and final concentrations for a task on a valve pair
          pulse(valve_pair, linear_calc(valve_pair, SCHEDULES[valve_pair][current_task_index[valve_pair]][1], (float)SCHEDULES[valve_pair][current_task_index[valve_pair]][2] / 100, (float)SCHEDULES[valve_pair][current_task_index[valve_pair]][3] / 100));
          break;
        case 'Q':
          pulse(valve_pair, quadratic_calc(valve_pair, SCHEDULES[valve_pair][current_task_index[valve_pair]][1], (float)SCHEDULES[valve_pair][current_task_index[valve_pair]][2] / 100, (float)SCHEDULES[valve_pair][current_task_index[valve_pair]][3] / 100));
          break;
        default:                                          // nothing is scheduled, no fluid will output
          digitalWrite(VALVE_PAIRS[valve_pair][0], LOW);  // set all pins to low voltage (valves closed)
          digitalWrite(VALVE_PAIRS[valve_pair][1], LOW);
          break;
      }

    } else {

      digitalWrite(VALVE_PAIRS[valve_pair][0], LOW);
      digitalWrite(VALVE_PAIRS[valve_pair][1], LOW);
    }
  }
}

// ** loop() complete **

// accepts valve pair and intended fraction of media, sets voltages based on progress through pulse cycle handled in loop()
void pulse(byte valve_pair, float fraction_media) {

  // splits pulse cycle into two subsequent portions, media and base
  if (millis() <= (cycle_start_time[valve_pair] + fraction_media * TIME_PER_CYCLE)) {  // media fraction of cycle, e.g., 0-300ms
    digitalWrite(VALVE_PAIRS[valve_pair][0], LOW);                                     // media on
    digitalWrite(VALVE_PAIRS[valve_pair][1], HIGH);                                    // base off
  } else if (millis() <= cycle_start_time[valve_pair] + TIME_PER_CYCLE) {  // base fraction of cycle, e.g., 301-1000ms
    digitalWrite(VALVE_PAIRS[valve_pair][0], HIGH);
    digitalWrite(VALVE_PAIRS[valve_pair][1], LOW);
  }
}

// fills in schedule values for a single task and increases number of tasks currently scheduled for a pair of valves.
void schedule(byte valve_pair, char profile, int duration, float initial_conc, float final_conc) {
  SCHEDULES[valve_pair][scheduled_items[valve_pair]][0] = profile;
  SCHEDULES[valve_pair][scheduled_items[valve_pair]][1] = duration;
  SCHEDULES[valve_pair][scheduled_items[valve_pair]][2] = initial_conc * 100;  // scales float values up for storage into byte, limited to 2.55f
  SCHEDULES[valve_pair][scheduled_items[valve_pair]][3] = final_conc * 100;
  scheduled_items[valve_pair]++;
}

// user command for scheduling a flat gradient
void flat(int valve_pair, int duration, float set_conc) {
  schedule(valve_pair, 'F', duration, set_conc, set_conc);
}

// user command for scheduling a linear ramp
void linear_ramp(int valve_pair, int duration, float initial_conc, float final_conc) {
  schedule(valve_pair, 'L', duration, initial_conc, final_conc);
}

// user command for scheduling a quadratic ramp
void quadratic_ramp(int valve_pair, int duration, float initial_conc, float final_conc) {
  schedule(valve_pair, 'Q', duration, initial_conc, final_conc);
}

// performs "calculations" for a flat gradient
float flat_calc(int valve_pair, float set_conc) {  
  return set_conc / MEDIA_CONCENTRATION[valve_pair];
}

// performs calculations for a linear ramp
float linear_calc(int valve_pair, int duration, float initial_conc, float final_conc) {
  return ((final_conc - initial_conc) / (ctime(duration)) * current_cycles[valve_pair] + initial_conc) / MEDIA_CONCENTRATION[valve_pair];  // change current_cycles for millis maybe, cycles is fine too but depends on valve_pair;
}

// performs calculations for a quadratic ramp
float quadratic_calc(int valve_pair, int duration, float initial_conc, float final_conc) {
  return ((final_conc - initial_conc) / (pow(ctime(duration), 2)) * pow(current_cycles[valve_pair], 2) + initial_conc) / MEDIA_CONCENTRATION[valve_pair];  // change current_cycles to current time in millis maybe;
}

// converts minutes to cycles
uint32_t ctime(int time_minutes) {
  return (time_minutes * 60000 / TIME_PER_CYCLE);
}

// converts minutes to milliseconds
uint32_t mtime(int time_minutes) {
  return (time_minutes * 60000);
}
