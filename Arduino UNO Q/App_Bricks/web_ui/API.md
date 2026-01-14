# web_ui API Reference

## Index

- Class `WebUI`

---

## `WebUI` class

```python
class WebUI(addr: str, port: int, ui_path_prefix: str, api_path_prefix: str, assets_dir_path: str, certs_dir_path: str, use_tls: bool, use_ssl: bool | None)
```

Module for deploying a web server that can host a web application and expose APIs to its clients.

It uses FastAPI, Uvicorn, and Socket.IO to serve static files (e.g., HTML/CSS/JS), handle REST API endpoints,
and support real-time communication between the client and the server.

### Parameters

- **addr** (*str*) (optional), default="0.0.0.0" (all interfaces): Server bind address. Defaults to "0.0.0.0" (all interfaces).
- **port** (*int*) (optional), default=7000: Server port number. Defaults to 7000.
- **ui_path_prefix** (*str*) (optional), default="" (root): URL prefix for UI routes. Defaults to "" (root).
- **api_path_prefix** (*str*) (optional), default="" (root): URL prefix for API routes. Defaults to "" (root).
- **assets_dir_path** (*str*) (optional), default="/app/assets": Path to static assets directory. Defaults to "/app/assets".
- **certs_dir_path** (*str*) (optional), default="/app/certs": Path to TLS certificates directory. Defaults to "/app/certs".
- **use_tls** (*bool*) (optional), default=False: Enable TLS/HTTPS. Defaults to False.
- **use_ssl** (*bool*) (optional), default=None: Deprecated. Use use_tls instead. Defaults to None.

### Methods

#### `local_url()`

Get the locally addressable URL of the web server.

##### Returns

- (*str*): The server's URL (including protocol, address, and port).

#### `url()`

Get the externally addressable URL of the web server.

##### Returns

- (*str*): The server's URL (including protocol, address, and port).

#### `start()`

Start the web server asynchronously.

This sets up static file routing and WebSocket event handlers, configures TLS if enabled, and launches the server using Uvicorn.

##### Raises

- **RuntimeError**: If 'index.html' is missing in the static assets directory.
- **RuntimeError**: If TLS is enabled but certificates fail to generate.
- **RuntimeWarning**: If the server is already running.

#### `stop()`

Stop the web server gracefully.

Waits up to 5 seconds for current requests to finish before terminating.

#### `expose_api(method: str, path: str, function: Callable)`

Register a route with the specified HTTP method and path.

The path will be prefixed with the api_path_prefix configured during initialization.

##### Parameters

- **method** (*str*): HTTP method to use (e.g., "GET", "POST").
- **path** (*str*): URL path for the API endpoint (without the prefix).
- **function** (*Callable*): Function to execute when the route is accessed.

#### `on_connect(callback: Callable[[str], None])`

Register a callback for WebSocket connection events.

The callback should accept a single argument: the session ID (sid) of the connected client.

##### Parameters

- **callback** (*Callable[[str], None]*): Function to call when a client connects. Receives the session ID (sid) as its only argument.

#### `on_disconnect(callback: Callable[[str], None])`

Register a callback for WebSocket disconnection events.

The callback should accept a single argument: the session ID (sid) of the disconnected client.

##### Parameters

- **callback** (*Callable[[str], None]*): Function to call when a client disconnects. Receives the session ID (sid) as its only argument.

#### `on_message(message_type: str, callback: Callable[[str, Any], Any])`

Register a callback function for a specific WebSocket message type received by clients.

The client should send messages named as message_type for this callback to be triggered.

If a response is returned by the callback, it will be sent back to the client
with a message type suffix "_response".

##### Parameters

- **message_type** (*str*): The message type name to listen for.
- **callback** (*Callable[[str, Any], Any]*): Function to handle the message. Receives two arguments:
the session ID (sid) and the incoming message data.

#### `send_message(message_type: str, message: dict | str, room: str | None)`

Send a message to connected WebSocket clients.

##### Parameters

- **message_type** (*str*): The name of the message event to emit.
- **message** (*dict | str*): The message payload to send (dict or str).
- **room** (*str*): The target Socket.IO room (defaults to all clients).

