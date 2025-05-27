
# Technical Analysis

## Indicators

### Local Maximum/Minimum

The `LocStat` class identifies local maximum and minimum points within a price dataset. This is fundamental for detecting trend reversals and significant price movements.

```python
def LocStat.compute(bars: pd.DataFrame, barAgg: BarAgg, m: LocStat.typ) -> pd.DataFrame
```

#### Parameters:
- **bars** (`pd.DataFrame`): The source dataframe containing price bars (e.g., open, high, low, close).
- **barAgg** (`BarAgg`): A callable aggregator function to extract a specific value from each bar (e.g., the closing price).
- **m** (`LocStat.typ`): An enumeration indicating whether to compute local maxima (`LocStat.typ.MAX`) or local minima (`LocStat.typ.MIN`).

#### Returns:
- **pd.DataFrame**: A dataframe containing details of each identified local maximum or minimum, including:
  - **bar index**: The index of the bar where the local extreme occurs.
  - **left depth**: The number of consecutive bars to the left that form a decreasing (for maxima) or increasing (for minima) sequence.
  - **right depth**: The number of consecutive bars to the right that form a decreasing (for maxima) or increasing (for minima) sequence.
  - **left change (%)**: The percentage change from the left depth to the local extreme.
  - **right change (%)**: The percentage change from the local extreme to the right depth.

#### Computation Steps:

1. **Initialization**: The method initializes necessary variables, including the total number of bars and a list to store identified statistical points.

2. **Iterative Analysis**: It iterates through the bars, comparing each bar's aggregated value to its neighbors based on the specified type (`MAX` or `MIN`).

3. **Local Extreme Detection**:
   - **Left Depth Calculation**: For each potential local extreme, the method calculates how many consecutive bars to the left maintain the extremum condition (e.g., for a maximum, each preceding bar must be lower).
   - **Right Depth Calculation**: Similarly, it calculates the right depth by ensuring that consecutive bars to the right continue the extremum condition.

4. **Percentage Change Computation**: The method calculates the percentage change from the local extreme to its surrounding depths, providing insight into the strength of the extremum.

5. **Result Compilation**: All identified local extremes are compiled into a dataframe, which is then returned.


### RSI (Relative Strength Index)

The `RSI` class calculates the Relative Strength Index, a momentum oscillator that measures the speed and change of price movements. RSI oscillates between 0 and 100 and is typically used to identify overbought or oversold conditions.

```python
def RSI.compute(bars: pd.DataFrame, barAgg: BarAgg, period: int) -> pd.DataFrame
```

#### Parameters:
- **bars** (`pd.DataFrame`): The source dataframe containing price bars.
- **barAgg** (`BarAgg`): A callable aggregator function to extract a specific value from each bar (e.g., the closing price).
- **period** (`int`): The number of periods (bars) over which to calculate the RSI.

#### Returns:
- **pd.DataFrame**: A dataframe containing RSI values and computation details for each bar, including:
  - **change**: The price change from the previous bar.
  - **average up**: The average of all upward price changes over the specified period.
  - **average down**: The average of all downward price changes over the specified period.
  - **RSI**: The computed RSI value.

#### Computation Steps:

1. **Change Calculation**: For each bar, compute the change in the aggregated value (e.g., closing price) compared to the previous bar.

2. **Average Gain and Loss**:
   - **Initial Averages**: At the point where enough data is available (i.e., after `period` bars), calculate the initial average gain and average loss.
   - **Subsequent Averages**: For each subsequent bar, update the average gain and average loss using a smoothing technique:
     - **Average Gain**: `((previous average gain * (period - 1)) + current gain) / period`
     - **Average Loss**: `((previous average loss * (period - 1)) + current loss) / period`

3. **RSI Calculation**: Compute the RSI using the formula:
   \[
   RSI = 100 - \left( \frac{100}{1 + \frac{\text{Average Gain}}{\text{Average Loss}}} \right)
   \]
   - **Edge Cases**:
     - If the average loss is zero, RSI is set to 100.
     - If not enough data is available (i.e., before the initial period), RSI is set to 0.

4. **Result Compilation**: Compile all RSI values and related computation details into a dataframe aligned with the original bars.


### Divergences

The `Div` class detects divergences between price movements and the RSI oscillator, based on the following inputs:

```python
def Div.compute(bars: pd.DataFrame, barAgg: BarAgg, RSI_period: int, max_div_period: int, max_neg_period: int) -> pd.DataFrame
```

The computation is performed in the following steps:

1. **Local Extremes Identification**: First, the `Div` class utilizes the `LocStat` class to isolate local maximums and minimums in the price data.

2. **RSI Computation**: Concurrently, the Relative Strength Index (RSI) is calculated for the price data over the specified `RSI_period`. 

3. **Divergence Points Detection**: For each identified local extreme (either a maximum or minimum), the algorithm iterates backwards and identifies other extreme points within `max_div_period` such that the former qualifies as a new extreme with respect to the latter. A divergence is confirmed if the direction of the RSI contradicts the direction of the price at these local extremes. 

4. **Negation of Divergence**: Concurrently, applying the same new-extreme-point-locating procedure within `max_div_period`, the algorithm identifies negated divergences by backtracing each backwards-located extreme point to its "root" and checking if the new extreme invalidates the previous divergence. 


### Bollinger Bands

The `BollBands` class computes Bollinger Bands, which consist of a middle band (a simple moving average), and upper and lower bands that are typically two standard deviations away from the middle band. Bollinger Bands are used to identify overbought or oversold conditions and to assess market volatility.

```python
def BollBands.compute(bars: pd.DataFrame, barAgg: BarAgg, period: int, mult: float) -> pd.DataFrame
```

#### Parameters:
- **bars** (`pd.DataFrame`): The source dataframe containing price bars.
- **barAgg** (`BarAgg`): A callable aggregator function to extract a specific value from each bar (e.g., the closing price).
- **period** (`int`): The number of periods (bars) over which to calculate the moving average and standard deviation.
- **mult** (`float`): The multiplier for the standard deviation to determine the distance of the upper and lower bands from the middle band (commonly set to 2).

#### Returns:
- **pd.DataFrame**: A dataframe containing Bollinger Bands values for each bar, including:
  - **middle band**: The simple moving average over the specified period.
  - **upper band**: The middle band plus the product of the multiplier and the standard deviation.
  - **lower band**: The middle band minus the product of the multiplier and the standard deviation.

#### Computation Steps:

1. **Data Aggregation**: For each bar, aggregate the desired value using `barAgg` (e.g., closing price).

2. **Moving Average Calculation**: Compute the simple moving average (`middle band`) over the specified `period`.

3. **Standard Deviation Calculation**: Calculate the standard deviation of the aggregated values over the same period.

4. **Upper and Lower Bands**:
   - **Upper Band**: `middle band + (mult * standard deviation)`
   - **Lower Band**: `middle band - (mult * standard deviation)`

5. **Edge Cases**: 
   - If there are fewer than two data points in the period, set the standard deviation to 0 to prevent computation errors.

6. **Result Compilation**: Compile the middle, upper, and lower bands into a dataframe aligned with the original bars.

#### Additional Method: Cross Detection

The `BollBands` class also provides a method to detect crossings of the price through the Bollinger Bands, which can signal potential trading opportunities.

```python
def BollBands.cross(bars: pd.DataFrame, barAgg: BarAgg, BollBands: pd.DataFrame, index: int, band: fields, dir: crossDir) -> bool
```

##### Parameters:
- **bars** (`pd.DataFrame`): The source dataframe containing price bars.
- **BollBands** (`pd.DataFrame`): The dataframe containing computed Bollinger Bands.
- **barAgg** (`BarAgg`): A callable aggregator function to extract a specific value from each bar (e.g., the closing price).
- **index** (`int`): The specific bar index to check for a crossing.
- **band** (`fields`): Specifies which Bollinger Band to check against (`BOLL_UPPER`, `BOLL_LOWER`, or `BOLL_MID`).
- **dir** (`crossDir`): The direction of the crossing (`UP` for crossing upwards, `DOWN` for crossing downwards).

##### Returns:
- **bool**: Returns `True` if a crossing occurs at the specified index in the specified direction; otherwise, `False`.

##### Computation Steps:

1. **Boundary Check**: Ensure the specified index is within the valid range of the dataframe.

2. **Current and Previous Comparison**:
   - **Upward Crossing**: 
     - Current bar's aggregated value > upper band.
     - Previous bar's aggregated value ≤ upper band.
   - **Downward Crossing**:
     - Current bar's aggregated value < lower band.
     - Previous bar's aggregated value ≥ lower band.

3. **Result Determination**: If the above conditions are met based on the specified direction, return `True`; otherwise, return `False`.

### Ladders

The `Ladders` class identifies upward and downward ladder patterns within the price data. Ladders represent sequences of consecutive local maxima or minima that indicate the strength and direction of a trend.

```python
def Ladders.compute(bars: pd.DataFrame, barAgg: BarAgg, min_stat_pts: int, max_stat_pts: int = 2**31 - 1) -> pd.DataFrame
```

#### Parameters:
- **bars** (`pd.DataFrame`): The source dataframe containing price bars.
- **barAgg** (`BarAgg`): A callable aggregator function to extract a specific value from each bar (e.g., the closing price).
- **min_stat_pts** (`int`): The minimum number of contiguous stationary points (local maxima or minima) that a ladder must contain.
- **max_stat_pts** (`int`, optional): The maximum number of contiguous stationary points that a ladder can contain. Defaults to the maximum integer value.

#### Returns:
- **pd.DataFrame**: A dataframe containing identified ladders with the following details:
  - **left maximum/minimum**: The starting point of the ladder.
  - **right maximum/minimum**: The ending point of the ladder.
  - **total contained maximums and minimums**: The number of stationary points within the ladder.
  - **direction**: Indicates whether the ladder is upward (`-1`) or downward (`1`).

#### Computation Steps:

1. **Local Extremes Identification**:
   - Utilize the `LocStat` class to identify all local maxima and minima within the price data.

2. **Direction-Based Ladder Computation**:
   - **Upward Ladders**: Start from a local minimum and look for a sequence of increasing local maxima and subsequent minima.
   - **Downward Ladders**: Start from a local maximum and look for a sequence of decreasing local minima and subsequent maxima.

3. **Iteration and Comparison**:
   - Iterate through the identified local extremes, comparing consecutive points to determine if they form a ladder based on the specified direction.
   - Ensure that each subsequent point in the ladder maintains the trend (e.g., each new maximum is higher than the previous for upward ladders).

4. **Ladder Validation**:
   - Only ladders that meet the `min_stat_pts` and `max_stat_pts` criteria are considered valid and included in the final result.

5. **Result Compilation**: Compile all valid ladders into a dataframe, sorted by their starting points.

#### Notes:

- **Upward vs. Downward Ladders**:
  - **Upward Ladders** indicate strengthening bullish trends, as each local maximum increases.
  - **Downward Ladders** indicate strengthening bearish trends, as each local minimum decreases.

- **Flexibility**:
  - The `min_stat_pts` and `max_stat_pts` parameters allow for flexibility in defining what constitutes a significant ladder, accommodating various trading strategies and timeframes.

- **Integration with Other Indicators**:
  - Ladders can be used in conjunction with other indicators like RSI or Bollinger Bands to enhance trading signals and confirm trend strength.


## TBC

