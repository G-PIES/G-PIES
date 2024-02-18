#!/usr/bin/python

import pandas as pd #install pandas via 'pip3 install pandas'
import matplotlib.pyplot as plt #install matplotlib via 'pip3 install matplotlib'

csv = pd.read_csv('out/cd-output.csv')

# take the log of interstitials here and plot it
df = pd.DataFrame(csv,
    columns =['Time (s)', 'Cluster Size', 'Interstitials / cm^3', 'Vacancies / cm^3'])

# adjust x and y axis for specific values
df.plot.scatter(x = 'Time (s)', y = 'Cluster Size')
plt.show()