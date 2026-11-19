# Asset credits

The MIT license at the repository root covers source code and documentation
only. Files under `assets/` have their own terms, summarized below.

## Graphics (textures, maps, icons, splash logo)

The entire graphics layer — character and weapon sprites, map backgrounds,
UI icons, the splash logo, and other textures — was generated with AI tools
as an open test of those tools, then integrated into the project by
Rafał Ścibor.

Specific tools are not recorded for most files. Known exceptions from file
metadata:

- `textures/splash_logo.jpg` — generated with Google AI (metadata also
  mentions Photoshop).
- `textures/target_1.png` … `target_4.png` — include a GIMP “Created with
  GIMP” comment; edited/integrated in GIMP after generation.

## Font

- `fonts/default.ttf` — Bebas Neue Regular, licensed under the SIL Open
  Font License, Version 1.1. The license text is in `fonts/OFL.txt`.

## Music

- `music/main_menu.mp3`
- `music/game_music_1.mp3`
- `music/game_music_2.mp3`

These tracks were generated with [Suno](https://suno.com). Source links
embedded in the files:

- https://suno.com/song/919ef0ab-4be5-4465-a7e4-ad30737f108a
- https://suno.com/song/e478b671-a90a-4255-8120-edafae846c85
- https://suno.com/song/0bea76b8-caa6-4f8c-b1bb-952b5cfdee40

## Sounds

Sound files live in `assets/sounds/`. They are not tracked in git and ship
in the release zip. Ids match `assets/data/resources.json`.

Freesound files are CC0 1.0 (public domain dedication). No attribution is
required; authors are listed as a courtesy. Each one was trimmed, faded,
loudness-normalized, and converted to mono so the engine can position it
in space.

| File | Source | Author | License |
|---|---|---|---|
| `sounds/ak_single.mp3` | [Rifle Gun Shot AK47. close recording in a shooting range.](https://freesound.org/people/felix.blume/sounds/136399/) (one shot) | felix.blume | CC0 1.0 |
| `sounds/ak_fullauto.mp3` | [AK-47 Newgrounds Shooting Loop](https://freesound.org/people/fafasdsda/sounds/653542/) (1.7 s loop) | fafasdsda | CC0 1.0 |
| `sounds/pistol.mp3` | [Small pistol gunshot indoors](https://freesound.org/people/acidsnowflake/sounds/402789/) | acidsnowflake | CC0 1.0 |
| `sounds/shotgun.mp3` | [Shotgun Shot](https://freesound.org/people/LeMudCrab/sounds/163455/) | LeMudCrab | CC0 1.0 |
| `sounds/st1-footstep.mp3`, `sounds/st2-footstep.mp3`, `sounds/st3-footstep-sfx.mp3` | [walking indoors on concrete floor 1](https://freesound.org/people/FOSSarts/sounds/740419/) (three single steps) | FOSSarts | CC0 1.0 |
| `sounds/death_player.mp3` | [Male Death 4](https://freesound.org/people/scorpion67890/sounds/396801/) | scorpion67890 | CC0 1.0 |
| `sounds/death_1.mp3`, `sounds/death_2.mp3` | Generated with [ElevenLabs](https://elevenlabs.io) Sound Effects | ElevenLabs (generated for this project) | ElevenLabs free-plan output: non-commercial use, credit to ElevenLabs |
| `sounds/smg_burst.mp3` | Source unknown, original asset kept | unknown | unknown |

The earlier easter-egg death sounds (`death_amogus`, `death_lego_yoda`,
`death_scav`, and the never-shipped `death_scav_laugh`) were removed from
the game and the manifest.

Before any commercial use, regenerate or relicense `death_1` / `death_2` on
a paid ElevenLabs plan, and replace `smg_burst.mp3`.
