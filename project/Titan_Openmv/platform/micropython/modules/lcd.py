"""Legacy LCD helpers for the Titan Board 800x480 RGB565 panel."""

import display as _display

LCD_SHIELD = 1
LCD_DISPLAY = 2
LCD_RGB = LCD_DISPLAY
FWVGA = _display.FWVGA

# The module dictionary keeps the display object alive until deinit/soft reset.
_lcd = None


def init(type=LCD_RGB, *, width=800, height=480, framesize=FWVGA,
         refresh=56, bgr=False, portrait=False, triple_buffer=False):
    """Initialize the fixed RGB panel using two display buffers."""
    global _lcd
    # Reject unsupported options before disturbing an already running display.
    if type != LCD_RGB:
        raise ValueError("only the Titan Board RGB panel is supported")
    if width != 800 or height != 480 or framesize != FWVGA:
        raise ValueError("only 800x480 (FWVGA) is supported")
    if refresh != 56:
        raise ValueError("the panel refresh is fixed at approximately 56 Hz")
    if bgr or portrait or triple_buffer:
        raise ValueError("BGR, portrait and triple buffering are not supported")
    deinit()
    _lcd = _display.RGBDisplay(framesize=FWVGA, refresh=56,
                               triple_buffer=False)


def deinit():
    """Stop display output; repeated calls are harmless."""
    global _lcd
    if _lcd is not None:
        _lcd.deinit()
        _lcd = None


def _require_lcd():
    if _lcd is None:
        raise OSError("call lcd.init() first")
    return _lcd


def display(image, *args, **kwargs):
    """Display an image, optionally specifying position, ROI and scale."""
    target = _require_lcd()
    if args:
        # Support positional x, y and the old position/ROI tuple forms.
        if len(args) == 1 and isinstance(args[0], (tuple, list)):
            values = args[0]
            if len(values) == 4:
                names, values = ("roi",), args
            elif len(values) == 2:
                names = ("x", "y")
            else:
                raise TypeError("expected (x, y) or (x, y, w, h)")
        elif len(args) <= 2:
            names, values = ("x", "y"), args
        else:
            raise TypeError("use keyword arguments after x and y")
        for name, value in zip(names, values):
            if name in kwargs:
                raise TypeError("duplicate display argument: " + name)
            kwargs[name] = value
    if "scale" in kwargs:
        if "x_scale" in kwargs or "y_scale" in kwargs:
            raise TypeError("scale cannot be combined with x_scale or y_scale")
        kwargs["x_scale"] = kwargs["y_scale"] = kwargs.pop("scale")
    # Older OpenMV releases used 256 for fully opaque images.
    if kwargs.get("alpha") == 256:
        kwargs["alpha"] = 255
    return target.write(image, **kwargs)


def clear(off=False):
    return _require_lcd().clear(off)


def backlight(*args):
    return _require_lcd().backlight(*args)


def set_backlight(intensity):
    return _require_lcd().backlight(intensity)


def get_backlight():
    return _require_lcd().backlight()


def width():
    return _require_lcd().width()


def height():
    return _require_lcd().height()


def refresh():
    return _require_lcd().refresh()
