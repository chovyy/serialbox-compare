# serialbox-compare
Tools for dumping and comparing `SerialBox` files, generated by https://github.com/C2SM-RCM/serialbox or https://github.com/fortesg/serialbox-ftg.

## Build
1. Install SerialBox from https://github.com/C2SM-RCM/serialbox or https://github.com/fortesg/serialbox-ftg
2. Clone this repo: `git clone https://github.com/fortesg/serialbox-compare.git ; cd serialbox-compare`
3. Modify the [Makefile](Makefile) to your needs.
4. Run `make`.

## Usage

```
./dump [-h] [-i interval] [-j interval] [-k interval] [-l interval] [-o] [-q] FILE [SAVEPOINT]

Dumps SerialBox field in FILE at SAVEPOINT.
FILE has to be a .dat file and the field name has to be the last part of the file name, separated by _.
If no savepoint is given, the right savepoint is guessed.

Options: 
-h : Print this help
-i/j/k/l INTERVAL : Limit the output to the given interval in the i/j/k/l dimension. The INTERVAL is given by START:END, START:, or :END, e.g. -i 23:42
-o : Dump only field meta data
-q : Alias for -o
```

```
./compare [-h] [-i INTERVAL] [-j INTERVAL] [-k INTERVAL] [-l INTERVAL] [-t TOLERANCE] [-o] [-q] FILE1 FILE2 [SAVEPOINT1 [SAVEPOINT2]]

Compares SerialBox fields in FILE1 and FILE2 at SAVEPOINT1 and SAVEPOINT2.
FILE1 and FILE2 can be .dat or .json files, but need to be of the same type.
When they are .json files, all field are compared. When they are .dat files, the field name has to be the last part of the file name, separated by _.
If no savepoint is given, the right savepoint is guessed. If only SAVEPOINT1 is given, it has to exist in both files.

Options: 
-h : Print this help
-i/j/k/l INTERVAL : Limit the comparison to the given interval in the i/j/k/l dimension. The INTERVAL is given by START:END, START:, or :END, e.g. -i 23:42
-t TOLERANCE : Tolerance for the comparison, e.g. -t 0.0001
-o : Compare only field meta data
-q : Only print the name of the field(s) which contain(s) deviations
```

## License

[GNU General Public License v3.0](LICENSE)
