# Source layout

| Directory | Product ownership |
| --- | --- |
| `src/op/` | OP.EXE title and menus |
| `src/main/` | MAIN.EXE gameplay |
| `src/maine/` | MAINE.EXE ending and staff roll |
| `src/zun/` | ZUN.COM payloads (ZUNINIT, ZUN_RES) |
| `src/shared/` | Code proved identical across TH02 products |
| `compat/rec98/` | One-line forwarders into the pinned ReC98 tree |

Place files by artifact and subsystem, never by reconstruction state. Record
translation-unit splits and linker order as build inputs. ReC98 shares many
TH02 units between products (`snd_*.c`, `pi_load.cpp`, `input_rs.cpp`, ...);
a shared unit moves to `src/shared/` only after each consuming product is
checked.
