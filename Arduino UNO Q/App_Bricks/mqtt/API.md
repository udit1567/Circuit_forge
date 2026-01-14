# mqtt API Reference

## Index

- Class `MQTT`

---

## `MQTT` class

```python
class MQTT(broker_address: str, broker_port: int, username: Optional[str], password: Optional[str], topics: List[str], client_id: str)
```

MQTT class for publishing and subscribing to MQTT topics.

### Parameters

- **broker_address** (*str*): The address of the MQTT broker.
- **broker_port** (*int*): The port of the MQTT broker.
- **username** (*str*), default=None: The username for MQTT authentication. Defaults to None.
- **password** (*str*), default=None: The password for MQTT authentication. Defaults to None.
- **topics** (*List[str]*) (optional), default=None: List of topics to subscribe to upon connection. Defaults to None.
- **client_id** (*str*) (optional), default=None: A unique client ID for the MQTT client. If None or empty, a random ID will be generated. Defaults to None.

### Methods

#### `start()`

Start the MQTT client and connect to the broker.

#### `stop()`

Stop the MQTT client and disconnect from the broker.

#### `publish(topic: str, message: str | dict)`

Publish a message to the MQTT topic.

##### Parameters

- **topic** (*str*): The topic to publish the message to.
- **message** (*str|dict*): The message to publish. Can be a string or a dictionary.

##### Raises

- **ValueError**: If the topic is an empty string.
- **RuntimeError**: If the publish operation fails.

#### `subscribe(topic: str)`

Subscribe to a specified MQTT topic.

##### Parameters

- **topic** (*str*): The topic to subscribe to.

##### Raises

- **ValueError**: If the topic is an empty string.
- **RuntimeError**: If the subscription fails.

#### `on_message(topic: str, fn: Callable[[mqtt.Client, object, mqtt.MQTTMessage], None])`

Set the callback function for handling incoming messages on a specific topic.

##### Parameters

- **topic** (*str*): The topic to set the callback for.
- **fn** (*Callable[[mqtt.Client, object, mqtt.MQTTMessage], None]*): The callback function to handle incoming messages.

##### Raises

- **ValueError**: If the topic is an empty string or if fn is not callable.
- **RuntimeError**: If setting the callback fails.

