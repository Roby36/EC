
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
bollUpperCOLOR = 'tab:red'
bollLowerCOLOR = 'tab:red'
bollMiddleCOLOR = 'tab:blue'
CLOSECOLOR = 'black'
AXISCOLOR = 'black'

bollBuySigColor = 'blue'
bollSellSigColor = 'red'

# Scatter plot shapes options
bollBuySigShape = "^"
bollSellSigShape = "v"

MARKERSIZE = 100

# Data directories
datesDirectory = "/Users/roby/Desktop/EC/graphs/data/dates.txt"
closesDirectory = "/Users/roby/Desktop/EC/graphs/data/closes.txt"

bollMiddleDirectory = "/Users/roby/Desktop/EC/graphs/data/bollMiddle.txt"
bollUpperDirectory = "/Users/roby/Desktop/EC/graphs/data/bollUpper.txt"
bollLowerDirectory = "/Users/roby/Desktop/EC/graphs/data/bollLower.txt"
bollBuySigDirectory = "/Users/roby/Desktop/EC/graphs/data/bollBuySig.txt"
bollSellSigDirectory = "/Users/roby/Desktop/EC/graphs/data/bollSellSig.txt"


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

bollMiddle_data = [float(x) for x in getData(bollMiddleDirectory)]
bollUpper_data = [float(x) for x in getData(bollUpperDirectory)]
bollLower_data = [float(x) for x in getData(bollLowerDirectory)]
bollBuySig_data = [int(x) for x in getData(bollBuySigDirectory)]
bollSellSig_data = [int(x) for x in getData(bollSellSigDirectory)]

x_values = [dt.strptime(d, "%m/%d/%Y").date() for d in dates_data]

# Plotting closing price graph, and bollinger bands all on same axis:
fig, ax = plt.subplots()  
ax.set_xlabel('Date')
ax.set_ylabel('Prices', color=CLOSECOLOR)
ax.plot(x_values, closes_data, color = CLOSECOLOR, label="Close price")
ax.plot(x_values, bollLower_data, color = bollLowerCOLOR, label="Lower band")
ax.plot(x_values, bollMiddle_data, color = bollMiddleCOLOR, label="Middle band")
ax.plot(x_values, bollUpper_data, color = bollUpperCOLOR, label="Upper band")

# Formatting x-axis labels:
ax.tick_params(axis='y', labelcolor=AXISCOLOR)
formatter = mdates.DateFormatter("%d-%m-%Y")
ax.xaxis.set_major_formatter(formatter)
ax.xaxis.set_major_locator(mdates.DayLocator(interval = DAYINTERVAL))
plt.xticks(rotation = AXISROTATION)
plt.xticks(fontsize = AXISFONTSIZE)

# Scatter plots of buy and sell signals:
plt.scatter(restrictedXvalues(x_values, bollBuySig_data),
            restrictedYvalues(closes_data, bollBuySig_data), 
            marker=bollBuySigShape, label = "bollBuySig", color = bollBuySigColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, bollSellSig_data),
            restrictedYvalues(closes_data, bollSellSig_data), 
            marker=bollSellSigShape, label = "bollSellSig", color = bollSellSigColor, s=MARKERSIZE)
plt.legend()

# Adding title and plotting graph:
fig.tight_layout()
plt.title(TITLE, fontsize=14)
plt.show()