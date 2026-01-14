# streamlit_ui API Reference

## Index

- Function `addons.arduino_header`

---

## `addons.arduino_header` function

```python
def arduino_header(title: str)
```

Arduino custom header.

Render a minimal Arduino header: left-aligned title, right-aligned logo SVG, styled. SVG logo loaded by file.

---

Streamlit UI Brick

This module forwards the full [Streamlit](https://streamlit.io) API.

For detailed usage of Streamlit components such as buttons, sliders, charts, and layouts, refer to the official Streamlit documentation:
https://docs.streamlit.io/develop/api-reference

You can import this brick as:

    from arduino.app_bricks.streamlit_ui import st

Then use it just like native Streamlit:

    st.title("My App")
    st.button("Click me")

Additionally, custom components like `st.arduino_header()` are provided to streamline Arduino integration.

