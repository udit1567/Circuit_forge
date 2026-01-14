# dbstorage_tsstore API Reference

## Index

- Class `TimeSeriesStoreError`
- Class `TimeSeriesStore`

---

## `TimeSeriesStoreError` class

```python
class TimeSeriesStoreError()
```

Custom exception raised for TimeSeriesStore database operation errors.


---

## `TimeSeriesStore` class

```python
class TimeSeriesStore(host: str, port: int, retention_days: int)
```

Time series database handler for storing and retrieving data using InfluxDB.

This class extends the base InfluxDB handler and provides methods for writing samples to the database.
It allows writing and reading individual measurements with their values and timestamps.

### Parameters

- **host** (*str*) (optional): The hostname of the InfluxDB server.
Defaults to "dbstorage-influx".
- **port** (*int*) (optional): The port number of the InfluxDB server.
Defaults to 8086.
- **retention_days** (*int*) (optional): The number of days to retain data in the
InfluxDB bucket. Defaults to 7.

### Methods

#### `write_sample(measure: str, value: Any, ts: int, measurement_name: str)`

Write a time series sample to the InfluxDB database.

Stores a single data point with the specified measurement field, value, and timestamp.
If no timestamp is provided, the current time is used automatically.

##### Parameters

- **measure** (*str*): The name of the measurement field (e.g., "temperature", "humidity").
This acts as the column name for the data point.
- **value** (*Any*): The numeric or string value to store. Supports int, float, str, and bool types.
- **ts** (*int*) (optional): The timestamp in milliseconds since epoch.
Defaults to 0 (current time).
- **measurement_name** (*str*) (optional): The measurement container name that groups
related fields together. Defaults to "arduino".

##### Raises

- **TimeSeriesStoreError**: If there is an error writing the sample to the InfluxDB database,
such as connection failures or invalid data types.

#### `read_last_sample(measure: str, measurement_name: str, start_from: str)`

Read the last sample of a specific measurement from the InfluxDB database.

Retrieves the latest data point for the specified measurement field within
the given time range.

##### Parameters

- **measure** (*str*): The name of the measurement field to query (e.g., "temperature").
- **measurement_name** (*str*) (optional): The measurement container name to search within.
Defaults to "arduino".
- **start_from** (*str*) (optional): The time range to search within. Supports relative
periods like "-1d" (1 day), "-2h" (2 hours), "-30m" (30 minutes) or
RFC3339 timestamps like "2024-01-01T00:00:00Z". Defaults to "-1d".

##### Returns

-: tuple | None: A tuple containing (field_name, timestamp_iso, value) where:
- field_name (str): The measurement field name
- timestamp_iso (str): ISO format timestamp string
- value (Any): The stored value
Returns None if no data is found in the specified time range.

##### Raises

- **TimeSeriesStoreError**: If the start_from value is invalid or if there is an error querying the InfluxDB database.

#### `read_samples(measure: str, measurement_name: str, start_from: str, end_to: str, aggr_window: str, aggr_func: str, limit: int, order: str)`

Read all samples of a specific measurement from the InfluxDB database.

Retrieves multiple data points for the specified measurement field with support
for time range filtering, data aggregation, and result ordering.

##### Parameters

- **measure** (*str*): The name of the measurement field to query (e.g., "temperature").
- **measurement_name** (*str*) (optional): The measurement container name to search within.
Defaults to "arduino".
- **start_from** (*str*) (optional): The start time for the query range. Supports relative
periods ("-7d", "-1h") or RFC3339 timestamps. Defaults to "-1d".
- **end_to** (*str*) (optional): The end time for the query range. Supports same formats
as start_from or "now()". Defaults to None (current time).
- **aggr_window** (*str*) (optional): Time window for data aggregation (e.g., "1h" for hourly,
"30m" for 30-minute intervals). Must be used with aggr_func. Defaults to None.
- **aggr_func** (*str*) (optional): Aggregation function to apply within each window.
Supported values: "mean", "max", "min", "sum". Must be used with aggr_window.
Defaults to None.
- **limit** (*int*) (optional): Maximum number of samples to return. Must be positive.
Defaults to 1000.
- **order** (*str*) (optional): Sort order for results by timestamp. Must be "asc"
(ascending, oldest first) or "desc" (descending, newest first). Defaults to "asc".

##### Returns

- (*list*): List of tuples, each containing (field_name, timestamp_iso, value) where:
- field_name (str): The measurement field name
- timestamp_iso (str): ISO format timestamp string
- value (Any): The stored or aggregated value
Empty list if no data found in the specified range.

##### Raises

- **TimeSeriesStoreError**: If any parameter is invalid, such as:
- Invalid time format in start_from or end_to
- Invalid order value (not "asc" or "desc")
- Invalid limit value (not positive integer)
- Invalid aggregation function
- Mismatched aggr_window and aggr_func (one specified without the other)
- Database query errors

