# keylt CLI

Control your keyboard LEDs and LCD from shell scripts.

## Quick Reference

```
> LED Commands:
  keylt on                              Turn all keys on (full brightness)
  keylt off                             Turn all keys off
  keylt set_on_lights --key-ids 1,2,3   Turn listed keys on, all others off

> LCD Commands:
  keylt lcd <path>                      Upload image or GIF to keyboard LCD

> General Commands:
  keylt list                            List connected keyboards
  keylt help                            Show usage information
```

## Options

| Option | Description |
|--------|-------------|
| `--keyboard VID:PID` | Target a specific keyboard when multiple are attached (see `keylt list`) |
| `--help` | Show usage information |

## Examples

```bash
# Turn off all keyboard LEDs
keylt off

# Light up only WASD keys (by key ID)
keylt set_on_lights --key-ids 1,2,3,4

# Upload a GIF to the SK80 LCD screen
keylt lcd ~/images/animation.gif

# List connected keyboards to find VID:PID
keylt list
# Output: SK80    05ac:024f

# Target a specific keyboard
keylt off --keyboard 05ac:024f
```

## Exit Codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Usage error (bad arguments, unknown command) |
| 2 | Device error (no keyboard found, connection failed) |
| 3 | I/O error (USB communication failure) |
