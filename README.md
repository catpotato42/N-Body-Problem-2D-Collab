# N-Body-Simulation
## A two-dimensional graphical simulation of the n body problem in c++ using an initial value problem solver created from scratch, as well as the win32 API library
### Created by Alex Burns and Simon Harrington.
### Use instructions
1. Clone the main or exe branch to your WINDOWS PC.
2. If you downloaded the exe, simply double click and run. Otherwise, open the solution with visual studio and click the unfilled play button (start without debugging).
3. Input a number of planets from 2-10
4. Input desired fpss (frames per simulation second). This affects the actual speed the simulation runs at. To find the approximate time your computer will run the simulation for, find fpss*simulation length / ~10, where 10 is the **frames per second** your computer draws the simulation at.
5. Input desired simulation length. This is the length of time the computer will simulate the orbit of the planets for, not the real time the simulation lasts. Press Create.
6. Then, enter initial values (x-pos, y-pos, x-vel, y-vel, mass) for each planet and begin the simulation. 
7. Position variables are in pixels from the top left of the screen (positive down and right), which has its width and height printed at the top left when entering initial values. This is not dynamic so resizing the window after the first screen is not recommended. pixels are translated 1 pixel per 1e6 (1 million) meters. 
8. Velocity is in m/s, + is right/down, - is left/up. a couple thousand should give movement that looks reasonable.
9. Mass is in 1e24 kg. The gravitational constant is the same as the real world, at 6.6743e-11 m^3/kgs^2.
10. Press Start Simulation when ready! Don't worry if it says not responding, just don't click anything for a while. Longer simulations where fpss*simulation length > 1000 may take a couple minutes to load.

### Notes
Simulation speed is much higher than the real world (put simulation speed factor here), as orbital periods are measured in the spans of days to years in the real world - some planets in our solar system take thousands of years to orbit the sun.
Try simulating the Earth and Moon! The moon is ~7e22 kgs, while the earth is ~6e24 kgs. The moon orbits around the earth at around 1000 m/s tangential velocity, and is about 380,000 km (380 million m) away from the earth, though due to its elliptical orbit this isn't exactly accurate. With the earth's velocity set to 0, this should give you a pretty stable orbit. 
