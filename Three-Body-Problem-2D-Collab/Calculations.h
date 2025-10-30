#pragma once
#include <iostream>
#include <vector>

//both structs are small so don't strictly need their own header files, so included here for simplicity.
//used in main file when user inputs initial values, is packaged into State struct.
struct PlanetInfo {
	int xPos;
	int yPos;
	float xVel;
	float yVel;
	float mass; // >0, enforce this.
};

struct State {
	State(int numPlanets) {
		//not necessary, would max out at 10 planets anyways so memory isn't a big issue here. However, this lets
		//me show off that I know how to construct member variables in constructor.
		states.resize(numPlanets);
	}
	std::vector<PlanetInfo> states; //vector is good cause need dynamic sizing
};

class Calculations {
public:
	//initial constructor of our calculations, after this our values for each planet are set by user input.
	Calculations(int planets, int fps, int simLength) //simLength in seconds
		: initialState(planets) //construct initial state
	{
		this->timeStep = 1000 / fps; //fps to ms
		this->simLength = 1000 * simLength; //seconds to ms
	};
	int timeStep; //in ms
	int simLength; //in ms
	//Function that sets initial values for each planet based on user input.
	void setInitialValues(std::vector<PlanetInfo> initVals);
	//Function to step our ODE outputting an array of State structs for each timestep (using our state structs, time intervals, and simulation length).

private:
	//struct holding an array of planet infos.
	State initialState;
};