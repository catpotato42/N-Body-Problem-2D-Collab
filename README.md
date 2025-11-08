# N-Body-Simulation
## A two-dimensional graphical simulation of the n body problem in c++ using an initial value problem solver created from scratch as well as the win32 API library
### Created by Alex Burns and Simon Harrington.
### Use instructions
1. Clone the main or exe branch to your WINDOWS PC.
2. If you downloaded the exe, simply double click and run. Otherwise, open the solution with visual studio and click the unfilled play button (start without debugging).
3. Input a number of planets from 2-10, desired fps (simulation speed), and simulation length. Press Create.
4. Then, enter initial values (x-pos, y-pos, x-vel, y-vel, mass) for each planet and begin the simulation. 
5. Position variables are in pixels from the top left of the screen (positive down and right), which has its width and height printed at the top left when entering initial values. This is not dynamic so resizing the window after the first screen is not recommended.
6. Velocity is in m/s. 500 thousand-5 million should give movement that looks reasonable.
7. Mass is in 10^24 kg. While you may notice these values are relatively consistent to the real world, issues with distances appearing too small on screen led us to decrease the gravitational constant from real world 6.67e-11 to 6.67e-5 to make the planets appear far apart while still being a good size.
