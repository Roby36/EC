
import numpy as np
from matplotlib import pyplot as plt, dates as mdates
from datetime import datetime as dt

strategy_code = "S1_hourly_shortDiv"
data_dir = "/Users/roby/Desktop/EC/graphs/data/"
day_interval = 120

# Main graph options
TITLE = "P&L" + "-" + strategy_code
AXISROTATION = 45
AXISFONTSIZE = 10

# Color options (divergences)
CLOSECOLOR = 'tab:blue'
RSICOLOR = 'tab:orange'
singleColor = 'black'
doubleColor = 'orange'
tripleColor = 'red'
quadColor = 'red'

# Scatter plot shapes options
divShape = 'X'
MARKERSIZE = 100

# Data directories:
datesDirectory  = data_dir + strategy_code + "-" + "dateTime.txt"
closesDirectory = data_dir + strategy_code + "-" + "close.txt"
PLDirectory     = data_dir + strategy_code + "-" + "PL.txt"
divergenceDirectory =  data_dir + strategy_code + "-" + "Divergence.txt"

# Functions:
def getData(directory):
    dataFile = open(directory,"r")
    data = dataFile.readlines()
    dataFile.close()

    for i in range(len(data)):
        string = data[i][:-1]
        data[i] = string
    return data

def restrictedXvalues(x_values, bool_data, num):
    restr_x_values = []
    for i in range(len(bool_data)):
        if bool_data[i] == num:
            restr_x_values.append(x_values[i])
    return restr_x_values

def restrictedYvalues(y_values, bool_data, num):
    restr_y_values = []
    for i in range(len(bool_data)):
        if bool_data[i] == num:
            restr_y_values.append(y_values[i])
    return restr_y_values

# Parsing data:
dates_data = getData(datesDirectory)
closes_data = [float(x) for x in getData(closesDirectory)]
PL_data = [float(x) for x in getData(PLDirectory)]
divergence_data = [int(x) for x in getData(divergenceDirectory)]

########## DATETIME VALUES ##################
x_values = [dt.strptime(d, "%d/%m/%Y %H:%M:%S %Z").date() for d in dates_data[1:]]
########## DATETIME VALUES ##################

# Plotting closing price graph:
fig, ax = plt.subplots()  
ax.set_xlabel('Date')
ax.set_ylabel('Close price', color=CLOSECOLOR)
ax.plot(x_values, closes_data[1:], color = CLOSECOLOR) # ensure lists of same length here!

# Scatter plots on closing price graph (divergences):
"""
plt.scatter(restrictedXvalues(x_values, divergence_data, 2),
            restrictedYvalues(closes_data, divergence_data, 2), 
            marker= divShape, label = "Single div", color = singleColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, divergence_data, 3),
            restrictedYvalues(closes_data, divergence_data, 3), 
            marker= divShape, label = "Double div", color = doubleColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, divergence_data, 4),
            restrictedYvalues(closes_data, divergence_data, 4), 
            marker= divShape, label = "Triple div", color = tripleColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, divergence_data, 5),
            restrictedYvalues(closes_data, divergence_data, 5), 
            marker= divShape, label = "Quad div", color = quadColor, s=MARKERSIZE)
"""

plt.legend()

# Formatting x-axis labels:
ax.tick_params(axis='y', labelcolor=CLOSECOLOR)
formatter = mdates.DateFormatter("%d-%m-%Y")
ax.xaxis.set_major_formatter(formatter)
ax.xaxis.set_major_locator(mdates.DayLocator(interval = day_interval))
plt.xticks(rotation = AXISROTATION)
plt.xticks(fontsize = AXISFONTSIZE)

# Setting up second y-axis for plotting RSI data:
ax2 = ax.twinx()
ax2.set_ylabel('Profit/Loss', color=RSICOLOR)
ax2.plot(x_values, PL_data[1:], color = RSICOLOR) # ensure lists of same length here!
ax2.tick_params(axis='y', labelcolor=RSICOLOR)

# Adding title and plotting graph:
fig.tight_layout()
plt.title(TITLE, fontsize=14)
plt.show()
