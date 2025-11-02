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
		//not necessary, would max out at 10 planets anyways so memory isn't a big issue here.
		states.resize(numPlanets);
	}
	std::vector<PlanetInfo> states; //vector is good cause need dynamic sizing
};

class Calculations {
public:
	//initial constructor of our calculations, after this our values for each planet are set by user input.
	Calculations(int planets, int fps, float simLength) //simLength in seconds
		: initialState(planets) //construct initial state
	{
		this->planets = planets;
		this->timeStep = 1000 / fps; //fps to ms
		this->simLength = (int)(1000 * simLength); //seconds to ms
		this->planets = planets;
	};
	//Function that sets initial values for each planet based on user input.
	void setInitialValues(std::vector<PlanetInfo> initVals) {
		initialState.states = initVals;
	}
	//Function to step our ODE outputting an array of State structs for each timestep (using our state structs, time intervals, and simulation length).
	/*std::vector<State> solve() {
		for(int k = 0; k < simLength; k += timeStep) {
			for(int i = 0; i < planets; i++) {
				for (int j = 0; j < planets; j++) {
					
					double netAccelerationX = 0;
					double netAccelerationY = 0;
					double distance = distanceCalculation(initialState.states[j], initialState.states[i]);
					if (distance != 0) {
						//calculate accelerations
						netAccelerationY += G * (initialState.states[j].mass * (initialState.states[j].yPos - initialState.states[i].yPos) / pow(distance, 3));
						netAccelerationX += G * (initialState.states[j].mass * (initialState.states[j].xPos - initialState.states[i].xPos) / pow(distance, 3));
						
						//update velocities
						initialState.states[i].xVel += netAccelerationX * (timeStep / 1000);
						initialState.states[i].yVel += netAccelerationY * (timeStep / 1000);

						//update positions
						initialState.states[i].xPos += initialState.states[i].xVel * (timeStep / 1000);
						initialState.states[i].yPos += initialState.states[i].yVel * (timeStep / 1000);
					}
					
				}
			}
			
		}
		//return
		return std::vector<State>{initialState};
	}*/
private:
	//struct holding an array of planet infos.
	double G = 6.67430e-11; //gravitational constant
	State initialState;
	int timeStep; //in ms
	int simLength; //in ms
	int planets;
	double distanceCalculation(PlanetInfo p1, PlanetInfo p2) {
		return sqrt(pow(p2.xPos - p1.xPos, 2) + pow(p2.yPos - p1.yPos, 2));
	}
	
};