

# Technical Analysis

## Indicators

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


### Ladders

