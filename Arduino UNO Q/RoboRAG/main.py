from arduino.app_utils import App
from arduino.app_utils import Bridge

from langchain_ollama import ChatOllama
from langchain_core.tools import tool



# RELAY

@tool
def relay_on():
    """Turn relay ON."""
    return str(Bridge.call("relay_on"))


@tool
def relay_off():
    """Turn relay OFF."""
    return str(Bridge.call("relay_off"))


@tool
def relay_status():
    """Get relay status."""
    return str(Bridge.call("relay_status"))


# BUZZER

@tool
def buzzer_on():
    """Turn buzzer on at 1000Hz."""
    return str(Bridge.call("buzzer_on", 1000))


@tool
def buzzer_off():
    """Turn buzzer off."""
    return str(Bridge.call("buzzer_off"))


# DISTANCE


@tool
def get_distance():
    """Read distance sensor in millimeters."""
    return str(Bridge.call("get_distance"))



# THERMO
@tool
def get_temperature():
    """Read temperature in Celsius."""
    return str(Bridge.call("get_temperature"))


@tool
def get_humidity():
    """Read humidity percentage."""
    return str(Bridge.call("get_humidity"))


@tool
def get_thermo():
    """Read temperature and humidity."""
    return str(Bridge.call("get_thermo"))


# ==========================================
# MOVEMENT
# ==========================================

@tool
def get_movement():
    """
    Read movement sensor.
    Returns:
    x,y,z,roll,pitch,yaw
    """
    return str(Bridge.call("get_movement"))


# ==========================================
# TOOL REGISTRY
# ==========================================

TOOLS = {
    "relay_on": relay_on,
    "relay_off": relay_off,
    "relay_status": relay_status,
    "buzzer_on": buzzer_on,
    "buzzer_off": buzzer_off,
    "get_distance": get_distance,
    "get_temperature": get_temperature,
    "get_humidity": get_humidity,
    "get_thermo": get_thermo,
    "get_movement": get_movement,
}

ALL_TOOLS = list(TOOLS.values())


# OLLAMA


llm = ChatOllama(
    model="granite4:350m",
    temperature=0
)

llm_with_tools = llm.bind_tools(ALL_TOOLS)


# MAIN LOOP
def loop():

    user = input("\nYou: ")

    if user.lower() in ["exit", "quit"]:
        raise SystemExit

    response = llm_with_tools.invoke(user)

    if response.tool_calls:

        tool_results = []

        for tool_call in response.tool_calls:

            tool_name = tool_call["name"]

            print(f"\n[TOOL] {tool_name}")

            result = TOOLS[tool_name].invoke({})

            print(f"[RESULT] {result}")

            tool_results.append(
                f"{tool_name}: {result}"
            )

        final = llm.invoke(
            f"""
User request:
{user}

Tool results:
{chr(10).join(tool_results)}

Answer naturally and briefly.
"""
        )

        print("\nAssistant:", final.content)

    else:
        print("\nAssistant:", response.content)



# APP ENTRY
App.run(user_loop=loop)