
import numpy as np
from matplotlib import pyplot as plt, dates as mdates
from datetime import datetime as dt

# Main graph options
TITLE = "DAX"
AXISROTATION = 45
AXISFONTSIZE = 10
DAYINTERVAL = 14   # frequency of date ticks on x-axis

# Color options
CLOSECOLOR = 'tab:blue'
RSICOLOR = 'tab:orange'
disagreementColor = 'black'
consDisagreementColor = 'red'

# Scatter plot shapes options
disagreementShape = 'x'
consDisagreementShape = 'X'
MARKERSIZE = 100


# Data directories:
datesDirectory = "/Users/roby/Desktop/EC/graphs/data/dates.txt"
closesDirectory = "/Users/roby/Desktop/EC/graphs/data/closes.txt"
RSIDirectory = "/Users/roby/Desktop/EC/graphs/data/RSI.txt"
disagreementDirectory = "/Users/roby/Desktop/EC/graphs/data/disagreement.txt"
consDisagreementDirectory = "/Users/roby/Desktop/EC/graphs/data/consDisagreement.txt"

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
RSI_data = [float(x) for x in getData(RSIDirectory)]

disagreement_data = [int(x) for x in getData(disagreementDirectory)]
consDisagreement_data = [int(x) for x in getData(consDisagreementDirectory)]

x_values = [dt.strptime(d, "%m/%d/%Y").date() for d in dates_data]

# Plotting closing price graph:
fig, ax = plt.subplots()  
ax.set_xlabel('Date')
ax.set_ylabel('Close price', color=CLOSECOLOR)
ax.plot(x_values, closes_data, color = CLOSECOLOR)

# Scatter plots on closing price graph:
plt.scatter(restrictedXvalues(x_values, disagreement_data),
            restrictedYvalues(closes_data, disagreement_data), 
            marker= disagreementShape, label = "disagreement", color = disagreementColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, consDisagreement_data),
            restrictedYvalues(closes_data, consDisagreement_data), 
            marker= consDisagreementShape, label = "consDisagreement", color = consDisagreementColor, s=MARKERSIZE)
plt.legend()

# Formatting x-axis labels:
ax.tick_params(axis='y', labelcolor=CLOSECOLOR)
formatter = mdates.DateFormatter("%d-%m-%Y")
ax.xaxis.set_major_formatter(formatter)
ax.xaxis.set_major_locator(mdates.DayLocator(interval = DAYINTERVAL))
plt.xticks(rotation = AXISROTATION)
plt.xticks(fontsize = AXISFONTSIZE)

# Setting up second y-axis for plotting RSI data:
ax2 = ax.twinx()
ax2.set_ylabel('RSI', color=RSICOLOR)
ax2.plot(x_values, RSI_data, color = RSICOLOR)
ax2.tick_params(axis='y', labelcolor=RSICOLOR)

# Adding title and plotting graph:
fig.tight_layout()
plt.title(TITLE, fontsize=14)
plt.show()
