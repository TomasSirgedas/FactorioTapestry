# FactorioTapestry

## Interpretting results

In the [results](results) folder you'll see entries like this:
```
XXX.X.XXX..XXX.XX.XX.X.XX.XX.XXX..
4...4...4...4..4...4...4...4..4...
..5...5....5....5....5....5....5..
.6.....6.....6....6.....6....6....
```

This corresponds to 3 belts woven together in Factorio like this:
![image](https://github.com/user-attachments/assets/abf8b79e-ee0f-4051-8f59-0f84cd1e5586)
(This pattern happens to have the lowest 3-belt density: only 22 out of 34 available "cells" are needed by the pattern.)

## Results with color-changes

Color-changing belts open up some new extra possibilities.

Here's an example:
```
X.X..X..XX...
5....4...6.....6.....5....
..6.....5....5....4...6...
```

This corresponds to 2 belts which look like this:
![image](https://github.com/user-attachments/assets/1c92daec-05c4-4fd1-833f-8b86ff57e19e)

They can coexist on the same row like this:
![image](https://github.com/user-attachments/assets/dd7766a1-3577-4c7c-a332-726a7fd2e855)

Notice that only half of the full pattern is important, but that "belt A" on one tile connects to "belt B" on neighboring tiles. The text output is doubled to make this behavior easier to understand.

