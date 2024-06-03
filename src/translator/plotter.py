import pandas as pd
from matplotlib import pyplot as plt
import os
import sys

name = sys.argv[1]if len(sys.argv) > 1 else str('system.txt')
os.system(f'./generator {name} && make && ./solver > out.csv')

plt.rcParams["figure.figsize"] = [7.00, 3.50]
plt.rcParams["figure.autolayout"] = True

df = pd.read_csv("out.csv")

df.plot(x=df.columns[0])
plt.show()