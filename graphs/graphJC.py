
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
CLOSECOLOR = 'tab:blue'

bullEngulfColor = 'red'
bullRelEngulfColor = 'black'
bearEngulfColor = 'red'
bearRelEngulfColor = 'black'
bullHaramiColor = 'red'
bullRelHaramiColor = 'black'
bearHaramiColor = 'red'
bearRelHaramiColor = 'black'
morningStarColor = 'yellow'
eveningStarColor = 'black'
hammerColor = 'gray'
llDojiColor = 'orange'
dfDojiColor = 'green'
gsDojiColor = 'red'

# Scatter plot shapes options
bullEngulfShape = '<'
bullRelEngulfShape = '<'
bearEngulfShape = '>'
bearRelEngulfShape = '>'

bullHaramiShape = "3"
bullRelHaramiShape = "3"
bearHaramiShape = "4"
bearRelHaramiShape = "4"

morningStarShape = '*'
eveningStarShape = '*'
hammerShape = 'v'
llDojiShape = '+'
dfDojiShape = '+'
gsDojiShape = '+'

MARKERSIZE = 100


# Data directories:
datesDirectory = "/Users/roby/Desktop/EC/graphs/data/dates.txt"
closesDirectory = "/Users/roby/Desktop/EC/graphs/data/closes.txt"


bullEngulfDirectory = "/Users/roby/Desktop/EC/graphs/data/bullEngulf.txt"
bullRelEngulfDirectory = "/Users/roby/Desktop/EC/graphs/data/bullRelEngulf.txt"
bearEngulfDirectory = "/Users/roby/Desktop/EC/graphs/data/bearEngulf.txt"
bearRelEngulfDirectory = "/Users/roby/Desktop/EC/graphs/data/bearRelEngulf.txt"
bullHaramiDirectory = "/Users/roby/Desktop/EC/graphs/data/bullHarami.txt"
bullRelHaramiDirectory = "/Users/roby/Desktop/EC/graphs/data/bullRelHarami.txt"
bearHaramiDirectory = "/Users/roby/Desktop/EC/graphs/data/bearHarami.txt"
bearRelHaramiDirectory = "/Users/roby/Desktop/EC/graphs/data/bearRelHarami.txt"
morningStarDirectory = "/Users/roby/Desktop/EC/graphs/data/morningStar.txt"
eveningStarDirectory = "/Users/roby/Desktop/EC/graphs/data/eveningStar.txt"
hammerDirectory = "/Users/roby/Desktop/EC/graphs/data/hammer.txt"
llDojiDirectory = "/Users/roby/Desktop/EC/graphs/data/llDoji.txt"
dfDojiDirectory = "/Users/roby/Desktop/EC/graphs/data/dfDoji.txt"
gsDojiDirectory = "/Users/roby/Desktop/EC/graphs/data/gsDoji.txt"


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

bullEngulf_data = [int(x) for x in getData(bullEngulfDirectory)]
bullRelEngulf_data = [int(x) for x in getData(bullRelEngulfDirectory)]
bearEngulf_data = [int(x) for x in getData(bearEngulfDirectory)]
bearRelEngulf_data = [int(x) for x in getData(bearRelEngulfDirectory)]
bullHarami_data = [int(x) for x in getData(bullHaramiDirectory)]
bullRelHarami_data = [int(x) for x in getData(bullRelHaramiDirectory)]
bearHarami_data = [int(x) for x in getData(bearHaramiDirectory)]
bearRelHarami_data = [int(x) for x in getData(bearRelHaramiDirectory)]
morningStar_data = [int(x) for x in getData(morningStarDirectory)]
eveningStar_data = [int(x) for x in getData(eveningStarDirectory)]
hammer_data = [int(x) for x in getData(hammerDirectory)]
llDoji_data = [int(x) for x in getData(llDojiDirectory)]
dfDoji_data = [int(x) for x in getData(dfDojiDirectory)]
gsDoji_data = [int(x) for x in getData(gsDojiDirectory)]


x_values = [dt.strptime(d, "%m/%d/%Y").date() for d in dates_data]

# Plotting closing price graph:
fig, ax = plt.subplots()  
ax.set_xlabel('Date')
ax.set_ylabel('Close price', color=CLOSECOLOR)
ax.plot(x_values, closes_data, color = CLOSECOLOR)

# Scatter plots on closing price graph:
plt.scatter(restrictedXvalues(x_values, bullEngulf_data),
            restrictedYvalues(closes_data, bullEngulf_data), 
            marker=bullEngulfShape, label = "bullEngulf", color = bullEngulfColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, bullRelEngulf_data),
            restrictedYvalues(closes_data, bullRelEngulf_data), 
            marker= bullRelEngulfShape, label = "bullRelEngulf", color = bullRelEngulfColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, bearEngulf_data),
            restrictedYvalues(closes_data, bearEngulf_data), 
            marker= bearEngulfShape, label = "bearEngulf", color = bearEngulfColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, bearRelEngulf_data),
            restrictedYvalues(closes_data, bearRelEngulf_data), 
            marker= bearRelEngulfShape, label = "bearRelEngulf", color = bearRelEngulfColor, s=MARKERSIZE)

plt.scatter(restrictedXvalues(x_values, bullHarami_data),
            restrictedYvalues(closes_data, bullHarami_data), 
            marker=bullHaramiShape, label = "bullHarami", color = bullHaramiColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, bullRelHarami_data),
            restrictedYvalues(closes_data, bullRelHarami_data), 
            marker= bullRelHaramiShape, label = "bullRelHarami", color = bullRelHaramiColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, bearHarami_data),
            restrictedYvalues(closes_data, bearHarami_data), 
            marker= bearHaramiShape, label = "bearHarami", color = bearHaramiColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, bearRelHarami_data),
            restrictedYvalues(closes_data, bearRelHarami_data), 
            marker= bearRelHaramiShape, label = "bearRelHarami", color = bearRelHaramiColor, s=MARKERSIZE)

plt.scatter(restrictedXvalues(x_values, morningStar_data),
            restrictedYvalues(closes_data, morningStar_data), 
            marker= morningStarShape, label = "morningStar", color = morningStarColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, eveningStar_data),
            restrictedYvalues(closes_data, eveningStar_data), 
            marker= eveningStarShape, label = "eveningStar", color = eveningStarColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, hammer_data),
            restrictedYvalues(closes_data, hammer_data), 
            marker= hammerShape, label = "hammer", color = hammerColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, llDoji_data),
            restrictedYvalues(closes_data, llDoji_data), 
            marker= llDojiShape, label = "llDoji", color = llDojiColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, dfDoji_data),
            restrictedYvalues(closes_data, dfDoji_data), 
            marker= dfDojiShape, label = "dfDoji", color = dfDojiColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, gsDoji_data),
            restrictedYvalues(closes_data, gsDoji_data), 
            marker= gsDojiShape, label = "gsDoji", color = gsDojiColor, s=MARKERSIZE)
plt.legend()


# Formatting x-axis labels:
ax.tick_params(axis='y', labelcolor=CLOSECOLOR)
formatter = mdates.DateFormatter("%d-%m-%Y")
ax.xaxis.set_major_formatter(formatter)
ax.xaxis.set_major_locator(mdates.DayLocator(interval = DAYINTERVAL))
plt.xticks(rotation = AXISROTATION)
plt.xticks(fontsize = AXISFONTSIZE)

# Adding title and plotting graph:
fig.tight_layout()
plt.title(TITLE, fontsize=14)
plt.show()

