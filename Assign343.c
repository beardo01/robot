#pragma config(Sensor, S1,     touch,          sensorEV3_Touch)
#pragma config(Sensor, S3,     lightSensor,    sensorEV3_Color)
#pragma config(Sensor, S4,     sonar,          sensorEV3_Ultrasonic)
#pragma config(Motor,  motorB,          mL,            tmotorEV3_Large, PIDControl, encoder)
#pragma config(Motor,  motorC,          mR,            tmotorEV3_Large, PIDControl, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

/*
A list of constants for the program to use. This can be configured on the day of
the assignment.
*/

// Colors
#define BLACK_UPPER 14
#define WHITE_LOWER 52
#define GREY_LOWER 30
#define GREY_UPPER 52
#define LIGHT_TILE 20

// Turning
#define SPIN 690
#define QUARTER_TURN 350
#define WHEEL_ROTATION 200
#define SQUARE_CENTRE 190 // wheel rotation from center of tile to edge of tile
#define SQUARE_BIAS 0.75
#define MIN_RANGE 1.3

/*
Checks if the sensor currently senses black or not.

@return true if the light reflected is less than or equal to 14, otherwise false.
*/
bool isBlack() {
	return (getColorReflected(lightSensor) <= BLACK_UPPER);
}

/*
Sets the speed of both motors.

@param speed = rate the motors work at (0 to 100)
*/
void setSpeed(int speed) {
	motor[mL] = speed;
	motor[mR] = speed;
}

/*
Turns the robot by activating a servo motor and stopping the opposite motor

@param m = which motor we use to turn the robot
    0 = right motor
    1 = left motor
@param rotations = how many 90 degree turns to do
@param speed = the speed to set the chosen servo motor to.
*/
void turn(int m, int rotations, int speed) {
	setSpeed(0);
	if (m) {
		moveMotorTarget(mL, rotations * QUARTER_TURN, speed);
		waitUntilMotorStop(mL);
	} else {
		moveMotorTarget(mR, rotations * QUARTER_TURN, speed);
		waitUntilMotorStop(mR);
	}
}

/*
Drives the robot forward a set number of full wheel rotations.

@param num The number of wheel rotations to drive.
@param speed The speed to drive at.
*/
void drive(float num, int speed) {
	int current = getMotorEncoder(mL);
	while(getMotorEncoder(mL) <= current + (WHEEL_ROTATION * num)) {
		setSpeed(speed);
	}
	setSpeed(0);
}

/*
Spins the robot, on the point, in a certain direction at a certain speed.

@param speed The speed at which to turn the robot.
@param dir The direction to spin the robot.
	0 = clockwise
	1 = counter clockwise
*/
void spin(int speed, int dir) {
	if(dir == 0) {
		motor[mL] = speed;
		motor[mR] = speed - (speed * 2);
	} else {
		motor[mL] = speed - (speed * 2);
		motor[mR] = speed;
	}
}

/*
Starts on the black starting square, drives forward then detects and turn on
the first black tile.
*/
void findRow() {
	int white = 0;
	bool findingRow = true;

	// Start driving
	setSpeed(10);

	while(findingRow) {

		// Detect that we have hit a light tile and update white
		if (getColorReflected(lightSensor) >= LIGHT_TILE) {
			white = 1;
		}

		// Detect that we have hit a black tile, do a right turn and break.
		if (getColorReflected(lightSensor) <= BLACK_UPPER && white == 1) {
			playSound(soundBlip);
			turn(1, 1, 20); // 90 degree left turn
			findingRow = false;
		}
	}

	// End driving
	setSpeed(0);
}

/*
Drives along a row of tiles, counting black ones. The function stops after it
counts 15 tiles.
*/
void driveRow() {
	int count = 1;
	int lightTile = 0;

	// Start driving
	setSpeed(40);

	// Drive and count black tiles until we have counted 15.
	while(count < 15) {

		// Detect a light tile and set light flag.
		if (getColorReflected(lightSensor) >= LIGHT_TILE) {
			lightTile = 1;
		}

		// Detect a black tile and act accordingly.
		if (getColorReflected(lightSensor) <= BLACK_UPPER && lightTile == 1) {
			playSound(soundBlip);
			count++;

			// Reset our lightTile flag.
			lightTile = 0;
			drive(0.4, 15);
			int distance = getMotorEncoder(mL);
			//wait untill we hit an edge
			while(isBlack()){
				motor[mL] = 15;
			}
			setSpeed(0);

			//After hiting the edge turn the robot until we are in the centre of a square
			int current = getMotorEncoder(mL);
			distance = current - distance;
			while(getMotorEncoder(mL) >= current - ( distance * SQUARE_BIAS + SQUARE_CENTRE*(1-SQUARE_BIAS))) {
				motor[mL] = -15;
			}
			setSpeed(40);
		}
	}
	// Stop driving
	setSpeed(0);

}

/*
Makes a right turn and moves three quarters of the way to the tower.
*/
void moveCloser() {
	// Make a right hand turn
	turn(1, 1, 20);

	// Drive towards the tower
	moveMotorTarget(mL, 4500, 40);
	moveMotorTarget(mR, 4500, 40);
	waitUntilMotorStop(mL);
	waitUntilMotorStop(mR);
}

/*
 Finds the closest object through it, using sonar and faces towards it.
*/
 void findTower() {
 	float min = 255;
	int min_rotation = 0;
 	// Set current to the value of the Motor Encoder
 	int current = getMotorEncoder(mL);
 	// Do a 90degree left turn
 	while (getMotorEncoder(mL) >= current - SPIN / 4) {
 	spin(20, 1);
  }
  current = getMotorEncoder(mL);
 	while (getMotorEncoder(mL) <= current + SPIN/2) { //SPIN = 690         = 180degree spin
 		motor[mL] = 10;
 		motor[mR] = -10;
 		// While turning, find the closest thing and record the distance to it
 		if (getUSDistance(sonar) < min) {
 			min = getUSDistance(sonar);
 			min_rotation = getMotorEncoder(mL);
 		}
 	}
 	// Check to see if we found the tower, or if we need to drive and retry.
 	if (min > 100) {
 		drive(5, 30);
 		findTower();
 		return;
 	}

 	while (getMotorEncoder(mL) >= min_rotation) {
 	  motor[mL] = -10;
 	  motor[mR] = 10;
	}
 	// We are now facing the closest object so stop both motors
 	setSpeed(0);
}

/*
Drives the remaining distance to the tower, pushes it off the black and then
drives a set distance.

add bumper or conditonal
make sure our sensor value is good
*/
void pushTower() {
	bool pushedOff = false;
	while(pushedOff == false) {
		// Drive towards the tower
		setSpeed(10);

		// Detect that we are close and speed up for a certain amount of time
		if ((getUSDistance(sonar) < 7) && isBlack() || getTouchValue(touch) == 1){
				drive(2, 40);
				pushedOff = true;
		}
	}

	playSound(soundUpwardTones);
	setSpeed(0);
	wait1Msec(1000); //let sound play
}

task main() {
	// Let the robot get it's sh*t together
	wait1Msec(1000);
	// Drive from the start block, onto the row of tiles, then turn right.
	findRow();
	// Drive along the row of tiles, use pathing and count to 15.
	driveRow();
	// Make a right turn, drive towards the tower.
	moveCloser();
	// Find the tower and face it.
	displayTextLine(6, "findTower");
	findTower();
	// Push the tower off of the black block, then stop.
	displayTextLine(6, "pushTower");
	pushTower();

}
/*
Finds the closest object through it, using sonar and faces towards it.

void findTower() {
	float min = 255;

	// Set current to the value of the Motor Encoder
	int current = getMotorEncoder(mL);
	// Do a 360 degree turn
	while (getMotorEncoder(mL) <= current + SPIN) {
		motor[mL] = 20;
		motor[mR] = -20;
		// While turning, find the closest thing and record the distance to it
		if (getUSDistance(sonar) < min) {
			min = getUSDistance(sonar);
		}
	}

	// Check to see if we found the tower, or if we need to drive and retry.
	if (min > 100) {
		drive(5, 30);
		findTower();
		return;
	}
/*
	// Spin the robot to where the sonar recored the closest object
	current = getMotorEncoder(mL);
	while ((getUSDistance(sonar) > (min + 1)) && getMotorEncoder(mL) <= current + SPIN) {
		motor[mL] = 20;
		motor[mR] = -20;
	}
*/
	/*

  int start = 0;
	int end = 0;

	motor[mL] = 10;
	motor[mR] = -10;

	current = getMotorEncoder(mL);
	//find the start of the object
	while (getUSDistance(sonar) > MIN_RANGE * min || getMotorEncoder(mL) <= current + (SPIN/2)) {
	}
	//setLEDColor(ledRedFlash);
	start = getMotorEncoder(mL);


	//find the end of the object
	while (getUSDistance(sonar) < MIN_RANGE * min ) {
	}
	end = getMotorEncoder(mL);
	//setLEDColor(ledRed);

	setSpeed(0);

	//turn to the centre of the object
	current = getMotorEncoder(mR);
	while(getMotorEncoder(mR) < current + (end-start)/2){
		motor[mR] = 5;
		motor[mL] = -5;
	}
	// We are now facing the closest object so stop both motors
	setSpeed(0);
}
*/
