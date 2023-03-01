
import numpy as np
from matplotlib import pyplot as plt, dates as mdates
from datetime import datetime as dt


# Main graph options
TITLE = "DAX"
AXISROTATION = 45
AXISFONTSIZE = 10
DAYINTERVAL = 14   # frequency of date ticks on x-axis
SCALEFACTOR = 0.25 # bar charts relative sizes

# Color options
MACDCOLOR = 'tab:blue'
SIGCOLOR = 'tab:red'
CLOSECOLOR = 'orange'
AXISCOLOR = 'black'

buySigColor = 'blue'
sellSigColor = 'red'

# Scatter plot shapes options
buySigShape = "^"
sellSigShape = "v"

MARKERSIZE = 100


# Data directories:
datesDirectory = "/Users/roby/Desktop/EC/graphs/data/dates.txt"
closesDirectory = "/Users/roby/Desktop/EC/graphs/data/closes.txt"

MACDDirectory = "/Users/roby/Desktop/EC/graphs/data/MACD.txt"
sigMACDDirectory = "/Users/roby/Desktop/EC/graphs/data/sigMACD.txt"
buySigDirectory = "/Users/roby/Desktop/EC/graphs/data/buySig.txt"
sellSigDirectory = "/Users/roby/Desktop/EC/graphs/data/sellSig.txt"


# Functions:
def getData(directory):

    dataFile = open(directory,"r")
    data = dataFile.readlines()
    dataFile.close()

    for i in range(len(data)):
        string = data[i][:-1]
        data[i] = string

    return data


def restrictedXvalues(x_values, bool_data):
    restr_x_values = []
    for i in range(len(bool_data)):
        if bool_data[i] == 1:
            restr_x_values.append(x_values[i])
    return restr_x_values


def restrictedYvalues(y_values, bool_data):
    restr_y_values = []
    for i in range(len(bool_data)):
        if bool_data[i] == 1:
            restr_y_values.append(y_values[i])
    return restr_y_values


# Parsing data:
dates_data = getData(datesDirectory)
closes_data = [float(x) for x in getData(closesDirectory)]

MACD_data = [float(x) for x in getData(MACDDirectory)]
sigMACD_data = [float(x) for x in getData(sigMACDDirectory)]
buySig_data = [int(x) for x in getData(buySigDirectory)]
sellSig_data = [int(x) for x in getData(sellSigDirectory)]

x_values = [dt.strptime(d, "%m/%d/%Y").date() for d in dates_data]

# Plotting closing price graph:
fig, ax = plt.subplots()  
ax.set_xlabel('Date')
ax.set_ylabel('Close price', color=CLOSECOLOR)
ax.plot(x_values, closes_data, color = CLOSECOLOR)

# Formatting x-axis labels:
ax.tick_params(axis='y', labelcolor=AXISCOLOR)
formatter = mdates.DateFormatter("%d-%m-%Y")
ax.xaxis.set_major_formatter(formatter)
ax.xaxis.set_major_locator(mdates.DayLocator(interval = DAYINTERVAL))
plt.xticks(rotation = AXISROTATION)
plt.xticks(fontsize = AXISFONTSIZE)

# Setting up second y-axis for MACD & sigMACD data:
ax2 = ax.twinx()
ax2.set_ylabel('MACD', color=AXISCOLOR)
ax2.plot(x_values, MACD_data, color = MACDCOLOR, label="MACD")
ax2.plot(x_values, sigMACD_data, color = SIGCOLOR, label="Signal line")
ax2.tick_params(axis='y', labelcolor=AXISCOLOR)

# Scatter plots of buy and sell signals:
plt.scatter(restrictedXvalues(x_values, buySig_data),
            restrictedYvalues(MACD_data, buySig_data), 
            marker=buySigShape, label = "buySig", color = buySigColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, sellSig_data),
            restrictedYvalues(MACD_data, sellSig_data), 
            marker=sellSigShape, label = "sellSig", color = sellSigColor, s=MARKERSIZE)
plt.legend()

# Adding title and plotting graph:
fig.tight_layout()
plt.title(TITLE, fontsize=14)
plt.show()

