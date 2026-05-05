# Format Specifications

---

## Animation (ANM)

### File Header

| Offset | Size | Type         | Field               | Description                   |
|--------|------|--------------|---------------------|-------------------------------|
| 0x00   | 4    | `char[4]`    | Magic               | Must always be "ANIM"         |
| 0x04   | 2    | `u16`        | Version             | Must match ENGINE_VERSION     |
| 0x06   | 4    | `u32`        | Sprite Amount       | Number of sprites             |
| 0x0A   | 4    | `u32`        | Script Amount       | Number of scripts             |
| 0x0E   | 2    | `u16`        | Width               | Texture width                 |
| 0x10   | 2    | `u16`        | Height              | Texture height                |
| 0x12   | 4    | `SpriteTable*` | Sprite Table Offset | File offset to sprite table |
| 0x16   | 4    | `ScriptTable*` | Script Table Offset | File offset to script table |
| 0x1A   | 4    | `char*`      | Path Offset         | File offset to texture path   |
| 0x1E   | 4    | `char*`      | Alpha Path Offset   | File offset to alpha path     |

### Sprite Table Entry — 20 bytes

| Offset | Size | Type    | Field         | Description           |
|--------|------|---------|---------------|-----------------------|
| 0x00   | 4    | `s32`   | Sprite ID     | ID of sprite          |
| 0x04   | 4    | `float` | Sprite X      | X position in texture |
| 0x08   | 4    | `float` | Sprite Y      | Y position in texture |
| 0x0C   | 4    | `float` | Sprite Width  | Width in texture      |
| 0x10   | 4    | `float` | Sprite Height | Height in texture     |

### Script Table Entry — 8 bytes

| Offset | Size | Type      | Field         | Description           |
|--------|------|-----------|---------------|-----------------------|
| 0x00   | 4    | `s32`     | Script ID     | ID of script          |
| 0x04   | 4    | `Script*` | Script Offset | File offset to script |

### Script Instruction — 4 + Size bytes

| Offset | Size | Type       | Field     | Description                 |
|--------|------|------------|-----------|-----------------------------|
| 0x00   | 2    | `u16`      | Time      | Frame time of instruction   |
| 0x02   | 1    | `u8`       | Opcode    | Instruction opcode          |
| 0x03   | 1    | `u8`       | Size      | Size of arguments in bytes  |
| 0x04   | Size | `u8[Size]` | Arguments | Argument data               |

### Opcodes

| Opcode | Name               | Arguments                                 | Description                            |
|--------|--------------------|-------------------------------------------|----------------------------------------|
| 0      | NOP                | —                                         | No operation                           |
| 1      | SET_SPRITE         | `s32 id`                                  | Sets sprite                            |
| 2      | SET_RANDOM_SPRITE  | `s32 min_id, amp`                         | Picks random sprite in [m, m+a]        |
| 3      | SET_OFFSET         | `float ox, oy, oz`                        | Sets position offset                   |
| 4      | SET_SCALE          | `float sx, sy`                            | Sets scale                             |
| 5      | SET_ROTATION       | `float rx, ry, rz`                        | Sets rotation                          |
| 6      | SET_COLOR          | `u8 r, g, b`                              | Sets RGB color                         |
| 7      | SET_ALPHA          | `u8 alpha`                                | Sets alpha value                       |
| 8      | SET_VISIBILITY     | `u8 visible`                              | Sets visibility                        |
| 9      | SET_BLEND_MODE     | `u8 mode`                                 | Sets blend mode                        |
| 10     | SCROLL_TEXTURE_X   | `float dx`                                | Scrolls texture horizontally           |
| 11     | SCROLL_TEXTURE_Y   | `float dy`                                | Scrolls texture vertically             |
| 12     | FLIP_X             | —                                         | Toggles X flip                         |
| 13     | FLIP_Y             | —                                         | Toggles Y flip                         |
| 14     | ANCHOR_TOP_LEFT    | —                                         | Toggles anchor to top-left             |
| 15     | SET_OFFSET_SPEED   | `float osx, osy, osz`                     | Sets offset speed                      |
| 16     | SET_SCALE_SPEED    | `float ssx, ssy`                          | Sets scale speed                       |
| 17     | SET_ROTATION_SPEED | `float srx, sry, srz`                     | Sets rotation speed                    |
| 18     | Z_WRITE_DISABLE    | `u8 enabled`                              | Enables/disables Z-write               |
| 19     | STOP               | —                                         | Stops script execution                 |
| 20     | PAUSE              | —                                         | Pauses script                          |
| 21     | JUMP               | `u32 offset`                              | Jumps to another instruction           |
| 22     | FADE               | `u32 alpha, duration; u8 mode`            | Fades alpha over time                  |
| 23     | MOVE_TO            | `float x, y, z; u32 duration; u8 mode`   | Moves to position over time            |
| 24     | ROTATE_TO          | `float rx, ry, rz; u32 duration; u8 mode`| Rotates to angle over time             |
| 25     | SCALE_TO           | `float sx, sy; u32 duration; u8 mode`    | Scales over time                       |
| 26     | INTERRUPT_LABEL    | `s32 label`                               | Registers interrupt label              |

---

## Shot Table (SHT)

### File Header

| Offset | Size | Type    | Field                | Description                                      |
|--------|------|---------|----------------------|--------------------------------------------------|
| 0x00   | 2    | `s16`   | Shot Count           | Number of power-level entries                    |
| 0x02   | 1    | `u8`    | Bombs                | Number of bombs                                  |
| 0x03   | 4    | `u32`   | Death Bomb Window    | Frames after death where a bomb can still fire   |
| 0x07   | 4    | `float` | Hitbox               | Player collision radius                          |
| 0x0B   | 4    | `float` | Grazebox             | Graze detection radius                           |
| 0x0F   | 4    | `float` | Item Magnet Speed    | Speed at which items are attracted to the player |
| 0x13   | 4    | `float` | Item Box             | Item pickup radius                               |
| 0x17   | 4    | `float` | POC Line             | Y position of the Point of Collection line       |
| 0x1B   | 4    | `float` | Speed                | Normal movement speed                            |
| 0x1F   | 4    | `float` | Speed Diagonal       | Normal diagonal movement speed                   |
| 0x23   | 4    | `float` | Focus Speed          | Focused movement speed                           |
| 0x27   | 4    | `float` | Focus Speed Diagonal | Focused diagonal movement speed                  |

### Power Entry Table — 8 bytes per entry, Shot Count entries

| Offset | Size | Type    | Field            | Description                                    |
|--------|------|---------|------------------|------------------------------------------------|
| 0x00   | 4    | `s32*`  | Shot List Offset | File offset to the shot list for this power level |
| 0x04   | 4    | `s32`   | Power Level      | Power level this entry corresponds to          |

### Shot Data — 28 bytes per entry

List is terminated by a sentinel entry where both `interval` and `delay` equal −1.

| Offset | Size | Type    | Field      | Description                            |
|--------|------|---------|------------|----------------------------------------|
| 0x00   | 2    | `s16`   | Interval   | Frames between shots; −1 signals end of list |
| 0x02   | 2    | `s16`   | Delay      | Startup delay in frames; −1 signals end of list |
| 0x04   | 4    | `float` | Position X | Horizontal spawn offset relative to player |
| 0x08   | 4    | `float` | Position Y | Vertical spawn offset relative to player   |
| 0x0C   | 4    | `float` | Hitbox X   | Bullet collision width                 |
| 0x10   | 4    | `float` | Hitbox Y   | Bullet collision height                |
| 0x14   | 4    | `float` | Angle      | Fire angle in degrees                  |
| 0x18   | 4    | `float` | Speed      | Bullet speed                           |
| 0x1C   | 2    | `u16`   | Damage     | Damage dealt on hit                    |
| 0x1E   | 1    | `u8`    | Orb        | Orb slot index                         |
| 0x1F   | 1    | `u8`    | Type       | Bullet type identifier                 |
| 0x20   | 2    | `u16`   | Sprite     | Sprite ID for the bullet               |
| 0x22   | 2    | `u16`   | SFX        | Sound effect ID on fire                |