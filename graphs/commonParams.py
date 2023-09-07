
import numpy as np
from datetime import datetime as dt
import sys

strategy_code = "DaxInd_S1_5secs_longDiv"
data_dir = "/Users/roby/Desktop/EC/graphs/data/"
day_interval = 1

AXISROTATION = 45
AXISFONTSIZE = 10

# Color options (divergences)
CLOSECOLOR = 'black'
BBLOWCOLOR  = 'tab:red'
BBMIDCOLOR  = 'tab:blue'
RSICOLOR = 'tab:orange'
singleColor = 'black'
doubleColor = 'orange'
tripleColor = 'red'
quadColor = 'red'

# Scatter plot shapes options
divShape = 'X'
MARKERSIZE = 100

# Data directories:
datesDirectory      = data_dir + strategy_code + "-" + "dateTime.txt"
closesDirectory     = data_dir + strategy_code + "-" + "close.txt"
PLDirectory         = data_dir + strategy_code + "-" + "PL.txt"
divergenceDirectory = data_dir + strategy_code + "-" + "LongDivergence.txt"
BBDirectory         = data_dir + strategy_code + "-" + "BB.txt"
RSIDirectory        = data_dir + strategy_code + "-" + "RSI.txt"

# Functions:
def getData(directory):
    dataFile = open(directory,"r")
    data = dataFile.readlines()
    dataFile.close()

    for i in range(len(data)):
        string = data[i][:-1]
        data[i] = string
    return data

def parseFloat(directory, ind):
    dataFile = open(directory,"r")
    data = dataFile.readlines()
    dataFile.close()

    for i in range(len(data)):
        string = float(data[i].split()[ind])
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

def get_min(values):
    min = sys.float_info.max
    for value in values:
        if value < min:
            min = value 
    return min 

def get_max(values):
    max = sys.float_info.min
    for value in values:
        if value > max:
            max = value 
    return max 
 

# Parsing data:
dates_data      = getData(datesDirectory)
closes_data     = [float(x) for x in getData(closesDirectory)]
PL_data         = [float(x) for x in getData(PLDirectory)]
divergence_data = [int(x) for x in getData(divergenceDirectory)]

BBlow_data      = [float(x) for x in parseFloat(BBDirectory, 0)]
BBmid_data      = [float(x) for x in parseFloat(BBDirectory, 1)]
BBhigh_data     = [float(x) for x in parseFloat(BBDirectory, 2)]

RSI_data        = [float(x) for x in getData(RSIDirectory)]

########## DATETIME VALUES ##################
x_values = [dt.strptime(d, "%d/%m/%Y %H:%M:%S %Z") for d in dates_data[1:]]
########## DATETIME VALUES ##################


