
import numpy as np
from matplotlib import pyplot as plt, dates as mdates
from datetime import datetime as dt


datesDirectory = "/Users/roby/Desktop/EC/graphs/data/dates.txt"
closesDirectory = "/Users/roby/Desktop/EC/graphs/data/closes.txt"
RSIDirectory = "/Users/roby/Desktop/EC/graphs/data/RSI.txt"
volumesDirectory = "/Users/roby/Desktop/EC/graphs/data/volumes.txt"

bullEngulfDirectory = "/Users/roby/Desktop/EC/graphs/data/bullEngulf.txt"
bullRelEngulfDirectory = "/Users/roby/Desktop/EC/graphs/data/bullRelEngulf.txt"
bearEngulfDirectory = "/Users/roby/Desktop/EC/graphs/data/bearEngulf.txt"
bearRelEngulfDirectory = "/Users/roby/Desktop/EC/graphs/data/bearRelEngulf.txt"
morningStarDirectory = "/Users/roby/Desktop/EC/graphs/data/morningStar.txt"
eveningStarDirectory = "/Users/roby/Desktop/EC/graphs/data/eveningStar.txt"
hammerDirectory = "/Users/roby/Desktop/EC/graphs/data/hammer.txt"
llDojiDirectory = "/Users/roby/Desktop/EC/graphs/data/llDoji.txt"
dfDojiDirectory = "/Users/roby/Desktop/EC/graphs/data/dfDoji.txt"
gsDojiDirectory = "/Users/roby/Desktop/EC/graphs/data/gsDoji.txt"

disagreementDirectory = "/Users/roby/Desktop/EC/graphs/data/disagreement.txt"
consDisagreementDirectory = "/Users/roby/Desktop/EC/graphs/data/consDisagreement.txt"


AXISROTATION = 45
AXISFONTSIZE = 10
DAYINTERVAL = 14

CLOSECOLOR = 'tab:blue'
RSICOLOR = 'tab:orange'
VOLUMECOLOR = 'tab:gray'

bullEngulfColor = 'red'
bullRelEngulfColor = 'black'
bearEngulfColor = 'red'
bearRelEngulfColor = 'black'
morningStarColor = 'yellow'
eveningStarColor = 'black'
hammerColor = 'gray'
llDojiColor = 'orange'
dfDojiColor = 'green'
gsDojiColor = 'red'
disagreementColor = 'black'
consDisagreementColor = 'red'

bullEngulfShape = '<'
bullRelEngulfShape = '<'
bearEngulfShape = '>'
bearRelEngulfShape = '>'
morningStarShape = '*'
eveningStarShape = '*'
hammerShape = 'v'
llDojiShape = '+'
dfDojiShape = '+'
gsDojiShape = '+'
disagreementShape = 'x'
consDisagreementShape = 'X'

MARKERSIZE = 100

TITLE = "DAX"
SCALEFACTOR = 0.25


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


dates_data = getData(datesDirectory)
closes_data = [float(x) for x in getData(closesDirectory)]
RSI_data = [float(x) for x in getData(RSIDirectory)]
volume_data = [float(x) for x in getData(volumesDirectory)]
volume_data_adjusted = [SCALEFACTOR*(x - min(volume_data)) for x in volume_data]

bullEngulf_data = [int(x) for x in getData(bullEngulfDirectory)]
bullRelEngulf_data = [int(x) for x in getData(bullRelEngulfDirectory)]
bearEngulf_data = [int(x) for x in getData(bearEngulfDirectory)]
bearRelEngulf_data = [int(x) for x in getData(bearRelEngulfDirectory)]
morningStar_data = [int(x) for x in getData(morningStarDirectory)]
eveningStar_data = [int(x) for x in getData(eveningStarDirectory)]
hammer_data = [int(x) for x in getData(hammerDirectory)]
llDoji_data = [int(x) for x in getData(llDojiDirectory)]
dfDoji_data = [int(x) for x in getData(dfDojiDirectory)]
gsDoji_data = [int(x) for x in getData(gsDojiDirectory)]
disagreement_data = [int(x) for x in getData(disagreementDirectory)]
consDisagreement_data = [int(x) for x in getData(consDisagreementDirectory)]

x_values = [dt.strptime(d, "%m/%d/%Y").date() for d in dates_data]

fig, ax = plt.subplots()  # gca
ax.set_xlabel('Date')
ax.set_ylabel('Close price', color=CLOSECOLOR)
ax.plot(x_values, closes_data, color = CLOSECOLOR)


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


ax.tick_params(axis='y', labelcolor=CLOSECOLOR)

formatter = mdates.DateFormatter("%d-%m-%Y")
ax.xaxis.set_major_formatter(formatter)
ax.xaxis.set_major_locator(mdates.DayLocator(interval = DAYINTERVAL))
plt.xticks(rotation = AXISROTATION)
plt.xticks(fontsize = AXISFONTSIZE)

ax2 = ax.twinx()
ax2.set_ylabel('RSI', color=RSICOLOR)
ax2.plot(x_values, RSI_data, color = RSICOLOR)
ax2.tick_params(axis='y', labelcolor=RSICOLOR)

# Disagreement & consDisagreement only two scatters plotted on RSI_data rather than closes_data:
plt.scatter(restrictedXvalues(x_values, disagreement_data),
            restrictedYvalues(RSI_data, disagreement_data), 
            marker= disagreementShape, label = "disagreement", color = disagreementColor, s=MARKERSIZE)
plt.scatter(restrictedXvalues(x_values, consDisagreement_data),
            restrictedYvalues(RSI_data, consDisagreement_data), 
            marker= consDisagreementShape, label = "consDisagreement", color = consDisagreementColor, s=MARKERSIZE)
plt.legend()

plt.bar(x_values, volume_data_adjusted, color = VOLUMECOLOR)

fig.tight_layout()
plt.title(TITLE, fontsize=14)

plt.show()

