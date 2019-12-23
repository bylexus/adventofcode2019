# adventofcode2019
My adventofcode.com solutions for 2019

This year I challenge myself by using C++ as solving language - whenever possible :-)


## Compile

I'm using cmake for the build infrastructure. So run:

```bash
$ cmake -S . -B build
$ cd build
$ make
```

As always, it's a lot of work, and a lot of fun! I draw my hat before [Eric Wastl](http://was.tl/). The
puzzles are challenging, well-prepared and creative.

## Diary

### Day 1

Simple one, for preparing my dev env and implement some basic libs like parsing files etc.
A bit of looping, a simple recursive algorithm.

### Day 2

Introduction of the Intcode processor - Implement a basic processor-in-software (a little VM) that
can execute some few Opcodes like adding numbers and addressing memory.

It turns out that this Intcode processor is used in many more riddles. Doing it right the first time
pays off :-)

### Day 3

Find intersections and run length of twisted cables. I solved it by just storing the begin / end points
of each cable segment, and calculate the intersections while laying them out - at the same time
calculating the distance.

My solutions needs very less memory, as it does not store all the cable "points", but just the start/end coordinates.


### Day 4

Crack a password by iterating through numbers and follow the rules - easy, I count them up as numbers and
parse them by converting to strings.

### Day 5

Intcode processor enhancements - This one took a while, until I had it all correct. A lot of fiddling with
direct / indirect memory addressing, but in the end a solid Intcode VM evolved that can execute
something like a very simple Assembler language. That was fun!

### Day 6

Orbital Traversal - In the end, a Tree data structure that needed to be traversed and intersection to be found.

My first attempt to build it as a tree was the right decision, and I solved it in about an hour.

### Day 7

Intcode VM again - This time with multiple instances and some in-/output fiddling. The Intcode processor of mine
needed some adaption to fetch input values, and stop the VM execution on output values (so that I can process them).

It feels a bit like interrupts: The VM stops on a specific event (here, data output), and after the interrupt routine
did its job (my main program), the VM starts executing again.

The first part was easy, but the 2nd part took me a while until everything was in place correctly.

### Day 8

That was a simple one - calculate an image by x/y/z (layers) coordinates. In the end, an ASCII art showed the
solution for the 2nd part. I decided to create a png image, and found the C++ library "CImg" as a very simple solution.

That was a fun riddle, and I learned how to create images in C++ and to add an external library.

### Day 9

Intcode Processor again - some more Opcodes for processing relative memory addresses. My little VM evolves,
and it't really impressive what it can already do.

I also changed the memory model to use the C++ Boost Library's cpp_int, an arbitary size integer class, to support
(very9 large numbers and (very) large memory addresses.

Learned to implement the Boos library, which is painless.

### Day 10

This problem turned out as a geometry problem - calculating angles and beams. In the end, it was easy,
but again, fiddling until everything was correct took some time.

The 2nd part was easy, once I built an angle map with a list of the corresponding planets.

### Day 11

That was a fun one - The Intcode processor was being used to solve a somewhat "real-world" problem -
steering a robot with directions (inputs) and feedback (output).

I also created an ascii art and image output for that one.

### Day 12

That was a hard one - My first attempt tried to find the LCM for the coordinates of a single planet -
How many "rounds" will it take for the x,y,z coordinate to repeat - Then find the LCM for all the planets.

This turned out to be completely wrong, and it took me some hours and a lot of nerves.

The final solution was not so hard - Just look at the single axis for ALL planets together (how long does it take
until all the X-positions are back at square one, then x, z), and take the LCM for the 3 separate values. Phew!

### Day 13

Play Breakout! That one was really funny! The Intcode processor is being used to play a game of Breakout.
One has to give Joystick commands (input), and the Intcode program draws the (changed) play field blocks (output).

I decided to give inputs to the Intcode program whenever it asks for: I created a function that is called by the
Intcode processor when the program requests an input - The function then checks if the paddle need to be moved to the left
or right, depending on the current ball position. That turned out to be a very wise decision.

Watch my final solution:

<figure>
<img src="day-13.small.gif" />
<figcaption>Day 13 - Solution in action</figcaption>
</figure>

### Day 14

I found that one hard - The first part was some recursive lookups with some bookkeeping, while the 2nd part
could easily be solved with a Binary Search algorithm.

It turned out that my first part solution was WAY too slow - I have too many recursive lookups in there somewere.
I brute-forced the 2nd part with it anyway, but took hours - Other solutions ran in miliseconds. I did not find the
error until today.


### Day 15

A classical Backtrack problem - Map an unknown maze, and traverse it.

The first part was more trickier this time: It was not enough to just find the Oxygen (the maze target), but you
had to continue to map the whole maze - This is needed for the 2nd part.

The backtrack algorithm took me a while until I did everything correctly - in the end it was staight forward, but took
a lot of fiddling, outputting, debugging.

After part 1 was done, I could finally draw the map on the screen, just for fun :-)

For Part 2 I took another approach: I started at the one Oxygen tile, built a "future" oxygen list to be processed,
and processed them round for round until no more oxygen needed to be created.

The key for the 2nd part was the proper mapping in part 1. This made the 2nd part a lot easier. Watch the oxygen
distribution live:

<figure>
<img src="day-15.small.gif" />
<figcaption>Day 15 - Solution in action</figcaption>
</figure>

### Day 16

This is the first time I don't finish both puzzles.

Part 1 was pretty straight forward, and I did it in the realization that it *might* be
not an optimal approach for the 2nd part, as I guessed would be some magnitues
longer to process...

And so it was :-)) It was just not possible to use the brute force approach on the 2nd part.
So there need to be some kind of pattern in how the numbers will change over time.

I just did not get it. Afer reading some of the subreddit entries, I just came to the conclusion
that I never would have found out that by myself, so I let the 2nd part unresolved...

Sad...

### Day 17

Step 1 was simple: Printing an ASCII map, generated with the Intcode processor and a given
input program.

Step 2 was hard: I processed the scaffold and let the robot walk, turn, walk, until I reach
the end. I collected all commands during that process.

Then one needed to split those into 3 groups, which I really did on pen-and-paper.
I didn't had the nerve to find groups of strings programmatically.

Worked in the end :-))


### Day 18

My oh my... This is the first puzzle I have to skip. I /think/ this might be some kind of
Travelling Salesman / Path finding problem, but I just don't get a feasible idea how to solve this.
Besides, it looks like a lot of work, which I just don't have now...

:-(

### Day 19

Problem 1 was easy, when I finally noticed that I had to reload the program for each point I want to probe:
As soon as the program ran, and tested one point, it dit not deliver a 2nd point. So for each point I reloaded the whole program.

Problem 2 at first seem to be hard, as it was not possible to simply calculate a NxM field, as it took way too long to probe one point.
So I had to narrow the values: Where do X begin for a given Y, and do the ship's coordinate fit exactly into the beam.
Then it was a simple bisect to find the correct y.

### Day 20

Another maze - this time with teleporters, which do not cause much trouble besides parsing.
For solution 1, it took a while to parse the input, then it was a straight-forward diykstra in the end:
walk through the whole maze with dfs, update distances to the start on each point (the smaller the better).
While walking, consider to follow a path as follows:
- not yet visited: walk!
- visited, but actual distance is shorter than already noted distance: walk!
- visited, but actual distance is longer that already noted distance: not walk!

The 2nd part introduces another leven, or z-axis, to the maze. I will do that later. It does not
seem to be much more trouble, but who knows.... My idea:
- copy the maze for each level
- walk through a lower-level maze until leave (up or down)
- limit the level to max. nr of teleports (you cannot go deeper than # of teleports)

### Day 21

This one was a tricky logic puzzle. Solved it not fully by myself (had to inspire me by taking a look)
at the solution subredit.

### Day 22

OK, I *guess* the naïve solution will work for part 1, but not part 2: Just instantiate an array and
deal the cards in the array.

Aaaand yes, that's what I guessed :-)) 2nd part is ways out of bounds for the naïve solution.
Have to think about that for a while...

### Day 23

The Intcode Processor goes multi-threaded! Because in- and outputs of the Intcode Network Interface Cards need to
run async, I learned about C++ threads - those are really simple to use, almost Java-like :-))

I realized that reading / putting input values into the input buffer need to be thread safe - I used a mutex lock
to accomplish that.

For some reason the Part 1 programs always locked up after some message in-/output. So I slept the threads
for some miliseconds when requesting values from an empty queue - which helped in the end.
Why? I have no idea....

Part 2 was a bit of a timing issue in the end. Detecting "idleness" needed some fiddling with
locking the input queue while accessing it. It took a while to run through (~ 1 minute), but
worked finally.

I'm not really happy with that solution (arbitary thread sleep). I'm sure there is a more elegant and faster way to
achieve that.

