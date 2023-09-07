
from matplotlib import pyplot as plt, dates as mdates
from commonParams import *

TITLE = "P&L" + "-" + strategy_code

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
formatter = mdates.DateFormatter("%d-%m-%Y %H:%M:%S")
ax.xaxis.set_major_formatter(formatter)
ax.xaxis.set_major_locator(mdates.DayLocator(interval = day_interval))
plt.xticks(rotation = AXISROTATION)
plt.xticks(fontsize = AXISFONTSIZE)

# Setting up second y-axis for plotting PL data:
ax2 = ax.twinx()
ax2.set_ylabel('Profit/Loss', color=RSICOLOR)
ax2.plot(x_values, PL_data[1:], color = RSICOLOR) # ensure lists of same length here!
ax2.tick_params(axis='y', labelcolor=RSICOLOR)

# Adding title and plotting graph:
fig.tight_layout()
plt.title(TITLE, fontsize=14)
plt.show()

