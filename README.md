# Scarlet Skips — Nintendo 3DS Homebrew

**Scarlet Skips** is a small outdoor jump-rope arcade game for Nintendo 3DS homebrew.

## Game
- Outdoor sunset park setting with trees, benches, hills, skyline and water tower.
- Press **A** when the rope passes underneath the player.
- Successful skips increase the total counter, combo and coins.
- Near-perfect timing is rewarded with a PERFECT message.
- **B** opens the upgrade shop.
- **D-Pad Up/Down** selects an upgrade in the shop.
- **A** buys the selected upgrade.
- **START** saves and exits.
- Save file: `sdmc:/3ds/ScarletSkips/save.dat`.

## Upgrades
- **Rope Speed** — makes the rope rotate faster.
- **Coin Bonus** — increases coins per successful skip.
- **Scarlet Skips** — the signature red rope; increases speed and coin rewards.
- Levels can be purchased up to level 9.

## Art
`assets/icon.png` is a 48×48 HOME Menu icon. `assets/banner_art.png` is the promotional/banner artwork and `assets/gameplay_art.png` / `assets/shop_art.png` are reference artwork for the game presentation.

## Build
Install the standard devkitPro 3DS environment with devkitARM, libctru, citro2d, 3dsxtool, bannertool and makerom. Then run:

```sh
make
```

This project is configured to produce a `.3dsx` and a `.cia` using the available 3DS homebrew toolchain.

For a GitHub build, add a devkitPro/devkitARM runner or use a 3DS homebrew GitHub Actions workflow that installs the toolchain before running `make`.

## FBI
Copy the resulting `ScarletSkips.cia` to the SD card, open FBI, select the CIA, and choose **Install and delete CIA**.

## GitHub Actions

This repository includes `.github/workflows/build.yml`. GitHub Actions uses the maintained `devkitpro/devkitarm` container to build the project, then uploads `ScarletSkips.cia`, `ScarletSkips.3dsx`, and `ScarletSkips.smdh` as workflow artifacts.

After pushing the repository to GitHub, open **Actions → Build Scarlet Skips**, select a run, and download **ScarletSkips-3DS-build** from the Artifacts section.
