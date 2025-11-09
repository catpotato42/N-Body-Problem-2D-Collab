#pragma once
#include <iostream>
#include <vector>
#include <cmath>

//both structs are small so don't strictly need their own header files, so included here for simplicity.
//used in main file when user inputs initial values, is packaged into State struct.
struct PlanetInfo {
	PlanetInfo(double xPos, double yPos, float xVel, float yVel, double mass) 
	: xPos(xPos), yPos(yPos), xVel(xVel), yVel(yVel), mass(mass) {}

	PlanetInfo() : xPos(0), yPos(0), xVel(0.0f), yVel(0.0f), mass(0.0f) {}

	double xPos;
	double yPos;
	double xVel;
	double yVel;
	double mass; // >0, enforce this.
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
	Calculations(int planets, float frameTime, int simLength) //simLength in ms
		: initialState(planets) //construct initial state
	{
		this->planets = planets;
		this->timeStep = frameTime;
		this->simLength = simLength;
	};
	//Function that sets initial values for each planet based on user input.
	void setInitialValues(std::vector<PlanetInfo> initVals) {
		initialState.states = initVals;
		std::cout << initialState.states[0].xPos << std::endl;
	}
	void setMetersPerPixel(double metersPerPixel) { this->metersPerPixel = metersPerPixel; }


	//Function to step our ODE outputting an array of State structs for each timestep (using our state structs, time intervals, and simulation length).
	std::vector<std::vector<std::pair<float, float>>> solve() {
		std::vector<std::vector<std::pair<float, float>>> solution(planets);
		for(float k = 0; k < simLength; k += timeStep) {
			for(int i = 0; i < planets; i++) {
				std::vector<std::pair<float, float>> planetPosition;
				double netAccelerationX = 0;
				double netAccelerationY = 0;
				for (int j = 0; j < planets; j++) {
					double distance = distanceCalculation(initialState.states[j], initialState.states[i]);
					if (distance != 0) {
						//calculate accelerations
						netAccelerationY += G * (initialState.states[j].mass * (initialState.states[j].yPos - initialState.states[i].yPos) / std::pow(distance, 2));
						netAccelerationX += G * (initialState.states[j].mass * (initialState.states[j].xPos - initialState.states[i].xPos) / std::pow(distance, 2));
					
					}
				}
				//update velocities
				initialState.states[i].xVel += netAccelerationX * (timeStep / 1000.0);
				initialState.states[i].yVel += netAccelerationY * (timeStep / 1000.0);
				std::cout << "velocity: " << initialState.states[i].xVel << ", " << initialState.states[i].yVel << std::endl;
				//update positions
				initialState.states[i].xPos += initialState.states[i].xVel * (timeStep / 1000.0);
				initialState.states[i].yPos += initialState.states[i].yVel * (timeStep / 1000.0);
				std::cout << "new position " << initialState.states[i].xPos << ", " << initialState.states[i].yPos << std::endl;
				float xPosPixel = initialState.states[i].xPos / metersPerPixel;
				float yPosPixel = initialState.states[i].yPos / metersPerPixel;
				std::pair<float, float> insert(xPosPixel, yPosPixel);
				solution[i].push_back(insert);
			}
			
		}
		return solution;
	}
private:
	double G = 6.67430e-11; //gravitational constant
	State initialState;
	float timeStep; //in ms
	int simLength; //in ms
	int planets;
	double metersPerPixel;
	double distanceCalculation(PlanetInfo p1, PlanetInfo p2) {
		return std::sqrt(std::pow(p2.xPos - p1.xPos, 2) + std::pow(p2.yPos - p1.yPos, 2));
	}
	
};