#/bin/python
import sys

yellowGroupH = int(sys.argv[1], 16)
yellowGroupL = int(sys.argv[2], 16)
orangeGroupH = int(sys.argv[3], 16)
orangeGroupL = int(sys.argv[4], 16)
redGroupH    = int(sys.argv[5], 16)
redGroupL    = int(sys.argv[6], 16)

grid = [0] * 64

YELLOW = 1
ORANGE = 2
RED    = 3

def bitSet(value, bit):
    return (value & (1 << bit)) != 0

for i in range(0, 32):
    if bitSet(yellowGroupL, i):
        grid[i] = YELLOW
    elif bitSet(orangeGroupL, i):
        grid[i] = ORANGE
    elif bitSet(redGroupL, i):
        grid[i] = RED

for i in range(32, 64):
    if bitSet(yellowGroupH, i-32):
        grid[i] = YELLOW
    elif bitSet(orangeGroupH, i-32):
        grid[i] = ORANGE
    elif bitSet(redGroupH, i-32):
        grid[i] = RED


for y in range(0, 8):
    line = ''
    for x in range(0, 8):
        colour = grid[y * 8 + x]
        if colour == YELLOW:
            line += 'Y '
        elif colour == ORANGE:
            line += 'O '
        elif colour == RED:
            line += 'R '
        else:
            line += '- '
    print line
