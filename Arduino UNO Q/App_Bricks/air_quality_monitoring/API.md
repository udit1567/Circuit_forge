# air_quality_monitoring API Reference

## Index

- Class `AQILevel`
- Class `AirQualityData`
- Class `AirQualityMonitoring`
- Class `AirQualityLookupError`

---

## `AQILevel` dataclass

```python
class AQILevel()
```

Data class to represent AQI levels.

### Attributes

- **min_value** (*int*): Minimum AQI value for the level.
- **max_value** (*int*): Maximum AQI value for the level.
- **description** (*str*): Description of the AQI level.
- **color** (*str*): Color associated with the AQI level in hex.


---

## `AirQualityData` dataclass

```python
class AirQualityData()
```

Data class to represent air quality data.

### Attributes

- **city** (*str*): Name of the city.
- **lat** (*float*): Latitude of the city.
- **lon** (*float*): Longitude of the city.
- **url** (*str*): URL for more information about the air quality data.
- **last_update** (*str*): Last update timestamp of the air quality data.
- **aqi** (*int*): Air Quality Index value.
- **dominantpol** (*str*): Dominant pollutant in the air.
- **iaqi** (*dict*): Individual AQI values for various pollutants.

### Methods

#### `pandas_dict()`

Return the data as a dictionary suitable for pandas DataFrame.


---

## `AirQualityMonitoring` class

```python
class AirQualityMonitoring(token: str)
```

Class to get air quality data from AQICN API.

### Parameters

- **token** (*str*): API token for AQICN service.

### Raises

- **ValueError**: If the token is not provided.

### Methods

#### `get_air_quality_by_city(city: str)`

Get air quality data by city name.

##### Parameters

- **city** (*str*): Name of the city.

##### Returns

- (*AirQualityData*): Air quality assembled data.

##### Raises

- **AirQualityLookupError**: If the API request fails.

#### `get_air_quality_by_coords(latitude: float, longitude: float)`

Get air quality data by coordinates.

##### Parameters

- **latitude** (*float*): Latitude.
- **longitude** (*float*): Longitude.

##### Returns

- (*AirQualityData*): Air quality assembled data.

##### Raises

- **AirQualityLookupError**: If the API request fails.

#### `get_air_quality_by_ip()`

Get air quality data by IP address.

##### Returns

- (*AirQualityData*): Air quality assembled data.

##### Raises

- **AirQualityLookupError**: If the API request fails.

#### `process(item: dict)`

Process the input dictionary to get air quality data.

##### Parameters

- **item** (*dict*): Input dictionary containing either 'city', 'latitude' and 'longitude', or 'ip'.

##### Returns

- (*dict*): Air quality data.

##### Raises

- **ValueError**: If the input dictionary is not valid.

#### `assemble_data(data: dict)`

Create a payload for the air quality data.

##### Parameters

- **data** (*dict*): Air quality data.

##### Returns

- (*dict*): Payload with relevant air quality information.

#### `map_aqi_level(aqi: int)`

Returns AQILevel class matching provided AQI.


---

## `AirQualityLookupError` class

```python
class AirQualityLookupError(message: str, status: str)
```

Custom exception for air quality lookup errors.

### Parameters

- **message** (*str*): Error message.
- **status** (*str*): Status of the error, defaults to None.

### Methods

#### `from_api_response(cls, data: dict)`

AirQualityLookupError error handling based on response provided by AQI API.

Documented errors:
- {"status": "error", "data": "Invalid key"}
- {"status": "error", "data": "Unknown station"}
- {"status": "error", "data": "Over quota"}
- {"status": "error", "data": "Invalid query"}
- {"status": "error", "data": "Too Many Requests"}
- {"status": "error", "data": "IP not allowed"}
- {"status": "error", "data": "Unknown error"}
- {"status": "error", "data": {"message": "..."}}

##### Parameters

- **data** (*dict*): Response data from the AQI API.

##### Returns

- (*AirQualityLookupError*): An instance of AirQualityLookupError with the error message and status.

