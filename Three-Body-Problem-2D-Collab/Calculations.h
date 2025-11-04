#pragma once
#include <iostream>
#include <vector>
#include <cmath>

//both structs are small so don't strictly need their own header files, so included here for simplicity.
//used in main file when user inputs initial values, is packaged into State struct.
struct PlanetInfo {
	PlanetInfo(int xPos, int yPos, float xVel, float yVel, float mass) 
	: xPos(xPos), yPos(yPos), xVel(xVel), yVel(yVel), mass(mass) {}
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
	Calculations(int planets, int frameTime, int simLength) //simLength in ms
		: initialState(planets) //construct initial state
	{
		this->planets = planets;
		this->timeStep = frameTime;
		this->simLength = simLength;
	};
	//Function that sets initial values for each planet based on user input.
	void setInitialValues(std::vector<PlanetInfo> initVals) {
		initialState.states = initVals;
	}
	//Function to step our ODE outputting an array of State structs for each timestep (using our state structs, time intervals, and simulation length).
	std::vector<std::vector<std::pair<double, double>>> solve() {
		std::vector<std::vector<std::pair<double, double>>> solution;
		
		//do calculations for x&y-pos in floats for accuracy, then round to int ONLY for output.
		for(int k = 0; k < simLength; k += timeStep) {
			for(int i = 0; i < planets; i++) {
				for (int j = 0; j < planets; j++) {
					double netAccelerationX = 0;
					double netAccelerationY = 0;
					double distance = distanceCalculation(initialState.states[j], initialState.states[i]);
					if (distance != 0) {
						//calculate accelerations
						netAccelerationY += G * (initialState.states[j].mass * (initialState.states[j].yPos - initialState.states[i].yPos) / std::pow(distance, 3));
						netAccelerationX += G * (initialState.states[j].mass * (initialState.states[j].xPos - initialState.states[i].xPos) / std::pow(distance, 3));
						
						//update velocities
						initialState.states[i].xVel += netAccelerationX * (timeStep / 1000);
						initialState.states[i].yVel += netAccelerationY * (timeStep / 1000);

						//update positions
						initialState.states[i].xPos += initialState.states[i].xVel * (timeStep / 1000);
						initialState.states[i].yPos += initialState.states[i].yVel * (timeStep / 1000);
						solution[i].push_back({ initialState.states[i].xPos, initialState.states[i].yPos });

					}
					
				}
			}
			
			
		}
		return solution;
	}
private:
	//struct holding an array of planet infos.
	double G = 6.67430e-11; //gravitational constant
	State initialState;
	int timeStep; //in ms
	int simLength; //in ms
	int planets;
	double distanceCalculation(PlanetInfo p1, PlanetInfo p2) {
		return std::sqrt(std::pow(p2.xPos - p1.xPos, 2) + std::pow(p2.yPos - p1.yPos, 2));
	}
	
};