# Steiner tree solver for PACE 2018

This is a source code of submission to the PACE 2018 competition, made by the FIT CTU in
Prague team.
It contains our solutions for both Track 1 (Exact with few terminals) and Track 2 (Exact with
low treewidth) tasks.

### Algorithms

Track 1 implementation uses the classical dynamic programming solution given by
[Erickson, Monma, and Veinott](https://link.springer.com/article/10.1007/BF02283688).

Track 2 implementation uses a dynamic programming solution with the reduce method, as
described by [Bodlaender et. al.](http://arxiv.org/abs/1211.1505v1), using the union-find
 partition representation, and edge bitsets for solution backtracking.

### Installation and running

The solver utilizes a standard CMake installation process, which defaultly builds executables
for both Track 1 and 2.

You can compile the code by cloning the repository, and running then following commands from 
the root directory of the project:

```bash
mkdir bin
cd bin
cmake ..
make
```

This process should build executables `pace2018-problemA` (for Track 1 of the competition)
and `pace2018-problemB` (for Track 2 of the competition) inside the `bin` folder.

Both solvers use the standard input for reading the graph and tree decomposition, the result
is printed to the standard output. Expected input and given output format should follow the description
given by Appendix A and B in the problem statement.

Please note, we have always used the given executables with the *Static Binary* option
while testing on [optil.io](https://optil.io).

### Authors

Peter Mitura and Ondřej Suchý
FIT CTU in Prague

Licensed under the MIT License.

