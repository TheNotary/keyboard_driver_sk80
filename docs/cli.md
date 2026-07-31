# keylt-bin CLI Spec

You may find it conveinent to use the `keylt-bin` tool to control your keyboard from within scripts. The following commands are available:

```bash
# Switches off all keys, but leaving the listed --key-ids on
keylt set_on_lights --key-ids 1,2,3,4

# Switches off all keys
keylt off

# Switches on all keys to full brightness
keylt on

# Uploads gif to LCD
keylt lcd path/to/the.gif
```

Implement any missing driver functionality to the library (I don't think brightness is handled).  If there isn't enough documentation, tell me, but I think the documentation is in the comments.