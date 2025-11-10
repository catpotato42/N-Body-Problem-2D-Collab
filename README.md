# N-Body-Simulation
## A two-dimensional graphical simulation of the n body problem in c++ using an initial value problem solver created from scratch, as well as the win32 API library
### Created by Alex Burns and Simon Harrington.
### Use instructions
1. Clone the **Experimental** branch to your Windows PC.
2. ~~If you downloaded the exe, simply double click and run.~~ Otherwise, open the solution with visual studio and click the unfilled play button (start without debugging).
3. The initial values produce a very visually interesting result, so if you want to just click through the "Create" and "Start" buttons, you will get a standard version of the three body problem with planets of the same mass. You can raise the relative speed a little as well so that it doesn't take a minute and a half to play out.
4. Input a number of planets from 2-15. if you input >12 planets, depending on your screen size they may run off the screen.
5. Input desired fpss (frames per simulation second). This affects the accuracy of the simulation, as 2 fpss will calculate two states every in-simulation second. You can also put decimal values in this field, but only in the form x.xx (there must be a number before the decimal). This just means if you wish for .5 fpss you must enter 0.5. This is true for every input box with capability for decimals. 
6. Input desired simulation length. This is the length of time the computer will simulate the orbit of the planets for, not the real time the simulation lasts. This is in units of 10,000 seconds.
7. Input relative speed. This is the frames per 1,000 seconds of simulation we output. So, at 100k seconds (a value of 10 input into Sim Length) a relative speed of 1 gets us (100,000 s calculated / 1,000 s per frame output) * 1 (relative speed) = 100 frames output.
8. Press Create.
9. Then, enter initial values (x-pos, y-pos, x-vel, y-vel, mass) for each planet.
10. Position variables are in pixels from the top left of the screen (positive down and right), which has its width and height printed at the top left when entering initial values. This is not dynamic so resizing the window after the first screen is not recommended. pixels are translated 1 pixel per 1e6 (1 million) meters. 
11. Velocity is in m/s, + is right/down, - is left/up. a couple thousand should give movement that looks reasonable.
12. Mass is in 1e24 kg. The gravitational constant is the same as the real world, at 6.6743e-11 m^3/kgs^2.
13. Press Start Simulation when ready! Don't worry if it says not responding or shows a black screen for more than 30 seconds, just don't click anything for a while. Longer simulations where fpss*simulation length > 1000 may take over a minute to load.

### Notes
Simulation speed is much higher than the real world (1,000 seconds per each real-world frame at a Relative Speed of 1)
Try simulating the Earth and Moon! The moon is ~7e22 kgs, while the earth is ~6e24 kgs. The moon orbits around the earth at around 1000 m/s tangential velocity, and is about 380,000 km (380 million m) away from the earth, though due to its elliptical orbit this isn't exactly accurate. With the earth's velocity set to 0, this should give you a pretty stable orbit, although one that needs a larger relative speed to be interesting.

### Procedure/How we created this
This simulation uses Euler integration, which is a method of approximating ODEs (Ordinary Differential Equations) that is more simple to implement and sometimes less costly than other methods. Another commonly used ODE solver you might use if you did this program in python, for example, would be the Runge-Kutta 4. The Euler method is technically a Runge-Kutta 1, as it falls under the classification of a first-order Runge-Kutta equation, but Euler came first so he got to name it after himself. The major disadvantage to Euler integration vs the commonly used RK4 is that rounding errors on a previous step wildly throw off calculations on a successive step. This means that many more steps are often required to achieve the same accuracy as RK4, leading to Euler sometimes being more costly even though RK4 has more calculations required per step (4 vs 1).
We decided to use Win32 API for user interface, mostly due to the ease of creation on a Windows computer. However, the Calculations.h file is intentionally built as a standalone n-body ODE stepper, and we could very feasibly port it to qt if we wished to.

### Sources for further research
A paper that focuses on comparison of efficiency for Euler & Runge-Kutta:
Lanlege I., Kehinde R., Sobanke D. A., Garba U. M. Comparison of Euler and Runge-Kutta methods in solving ordinary differential equations of order two and four. Lagos Journal of Science, vol. A32, pp. 37–??, 2018. Available at: http://ljs.academicdirect.org/A32/010_037.htm
A simple guide to implementing this in python using common libraries:
https://medium.com/better-programming/2-d-three-body-problem-simulation-made-simpler-with-python-40d74217a42a

### Future Updates
We're pretty happy with the current state of the project, visuals may be the only thing we'd change. However, our first priority for any future update would be a button to rerun the simulation so you don't have to calculate it again.
