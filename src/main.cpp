#include "main.h"

/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */
void on_center_button() {
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
		pros::lcd::set_text(2, "I was pressed!");
	} else {
		pros::lcd::clear_line(2);
	}
}

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::lcd::initialize();
	pros::lcd::set_text(1, "Hello PROS User!");

	pros::lcd::register_btn1_cb(on_center_button);
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {}

class controller_joystick_input {
	public:
		int analog_value=0;
		int curved_value=0;
		
		controller_joystick_input(int analog_input){
			analog_value=analog_input;
		}

		int curve_by_exponent(double exponential_factor=2){
			int negative_multiplier = analog_value < 0 ? -1 : 1;

			// normalize to range [0, 1]
			double normalized = static_cast<double>(fabs(analog_value)) / 127.0;

			// calculate output using exponential curve
			double output = 127.0 * (pow(exponential_factor, normalized) - 1.0) / (exponential_factor - 1.0);

			// map to range [-127, 127]
			return static_cast<int>(output)*negative_multiplier;
		}

		int curve_by_power(double power_factor=2){
			// normalize
			double normalized = static_cast<double>(analog_value) / 127.0;

			// apply power function
			double output = pow(fabs(normalized), power_factor) * (normalized >= 0 ? 1 : -1);

			// map output back to range [-127, 127]
			curved_value=static_cast<int>(output * 127.0);
			return curved_value;
		}
};
/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
	pros::Controller master(pros::E_CONTROLLER_MASTER);
	pros::Motor_Group left_motors({8, 5, 7});
	pros::Motor_Group right_motors({13, 14, 15});

	left_motors.set_reversed(false);
	left_motors.set_brake_modes(pros::E_MOTOR_BRAKE_COAST);
	left_motors.set_gearing(pros::E_MOTOR_GEAR_600);
	left_motors.set_zero_position(0);

	right_motors.set_reversed(true);
	right_motors.set_brake_modes(pros::E_MOTOR_BRAKE_COAST);
	right_motors.set_gearing(pros::E_MOTOR_GEAR_600);
	right_motors.set_zero_position(0);
	
	

	while (true) {
		pros::lcd::print(0, "%d %d %d", (pros::lcd::read_buttons() & LCD_BTN_LEFT) >> 2,
		                 (pros::lcd::read_buttons() & LCD_BTN_CENTER) >> 1,
		                 (pros::lcd::read_buttons() & LCD_BTN_RIGHT) >> 0);

		controller_joystick_input left_joystick(master.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y));
		controller_joystick_input right_joystick(master.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X));

		left_joystick.curve_by_power();
		right_joystick.curve_by_power(); 

		int left_motor_moves = left_joystick.curved_value+right_joystick.curved_value;
		int right_motor_moves = left_joystick.curved_value-right_joystick.curved_value;

		left_motors.move_velocity(left_motor_moves*(600.0/127.0));
		right_motors.move_velocity(right_motor_moves*(600.0/127.0));

		printf("Joystick Inputs - Left: %d, Right: %d | Motor Outputs - Left: %d, Right: %d\n",
           left_joystick.analog_value, right_joystick.analog_value, left_motor_moves, right_motor_moves);

		pros::delay(20);
	}
}
