# Codenames
A [codenames](http://czechgames.com/en/codenames/) bot playing the part of a spymaster.

To use this program, you need to do the following:

1. Download the C++ source.

2. Compile it using `make`.

3. Take any binary word2vec-like model from `models/` and copy it to `data.bin`.
   Alternatively, download one in text format from e.g. http://nlp.stanford.edu/projects/glove/ (glove.840B.300d works well), and convert it to binary format using `preprocess.cpp`.

4. Run the program!

## Example run
```
Loading word2vec (200000 words, 300 dimensions)... done!
Type "help" for help
My color (b/r): b
b yard
a string
b face
r fighter
r contract
r scorpion
b mexico
g robot
g shot
b date
b undertaker
b straw
g parachute
g bark
r washington
r button
g theater
b france
r duck
r track
r charge
g lock
b plate
g kangaroo
b time
go
Thinking...
Printing statistics for "holidays"
0.412264	time (My)
0.386905	france (My)
0.297992	mexico (My)
0.293810	date (My)
0.229417	charge (Opponent)
0.220076	washington (Opponent)
0.205730	duck (Opponent)
0.179613	contract (Opponent)
0.171409	yard (My)
0.163325	face (My)
0.161205	string (Assassin)
0.155902	track (Opponent)
0.138476	theater (Civilian)
0.097219	bark (Civilian)
0.088466	straw (My)
0.088020	button (Opponent)
0.082675	parachute (Civilian)
0.077072	shot (Civilian)
0.069174	lock (Civilian)
0.059632	kangaroo (Civilian)
0.051771	plate (My)
0.041790	robot (Civilian)
0.025796	scorpion (Opponent)
0.018091	fighter (Opponent)
0.001693	undertaker (My)

1	3.536	holidays 4
2	2.968	clothes 4
3	2.944	holiday 4
4	2.934	vacation 3
5	2.909	middle 4
6	2.904	birth 4
7	2.897	till 3
8	2.884	walked 4
9	2.868	glasses 3
10	2.855	everywhere 4
11	2.855	shade 4
12	2.848	sitting 3
13	2.788	walls 4
14	2.780	glass 3
15	2.764	mound 4
16	2.747	laid 3
17	2.714	marks 4
18	2.679	sun 3
19	2.669	throwing 5
20	2.644	stood 4
The best clue found is holidays 4
holidays is the 4406th most popular word
```

## License

TODO. (Probably MIT or something?)

`models/glove.840B.330d.bin` is from https://nlp.stanford.edu/projects/glove/, available under [Public Domain Dedication and License](https://opendatacommons.org/licenses/pddl/1.0/).

`models/conceptnet.bin` is from https://github.com/commonsense/conceptnet-numberbatch, available under [CC-By-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/).

`models/word2gm.bin` is from https://github.com/benathi/word2gm, available under [BSD 3-Clause license](https://opensource.org/licenses/BSD-3-Clause).

