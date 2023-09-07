
from matplotlib import pyplot as plt, dates as mdates
from commonParams import *

TITLE = strategy_code

fig, ax = plt.subplots()  
ax.set_xlabel('Date')
ax.set_ylabel('Close price', color=CLOSECOLOR)
ax.plot(x_values, closes_data[1:], color = CLOSECOLOR) # ensure lists of same length here!

ax.plot(x_values, BBlow_data[1:], color = BBLOWCOLOR) # ensure lists of same length here!
ax.plot(x_values, BBmid_data[1:], color = BBMIDCOLOR)
ax.plot(x_values, BBhigh_data[1:], color = BBLOWCOLOR)

# Scatter plots on closing price graph (divergences):

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

plt.legend()

# Formatting x-axis labels:
ax.tick_params(axis='y', labelcolor=CLOSECOLOR)
formatter = mdates.DateFormatter("%d-%m-%Y %H:%M:%S")
ax.xaxis.set_major_formatter(formatter)
ax.xaxis.set_major_locator(mdates.MinuteLocator(interval = 1))
plt.xticks(rotation = AXISROTATION)
plt.xticks(fontsize = AXISFONTSIZE)
ax.set_ylim(get_min(closes_data), get_max(closes_data))

# Setting up second y-axis for plotting RSI data:
ax2 = ax.twinx()
ax2.set_ylabel('RSI', color=RSICOLOR)
ax2.plot(x_values, RSI_data[1:], color = RSICOLOR) # ensure lists of same length here!
ax2.tick_params(axis='y', labelcolor = RSICOLOR)

# Adding title and plotting graph:
fig.tight_layout()
plt.title(TITLE, fontsize=14)
plt.show()

