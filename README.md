# Pixels_1

# 🏰 Pixels Graphics · Tower Defense

Defend the castle across four lanes and three enemy waves. Spend coins to deploy units, dodge incoming shots, and collect coins from defeated enemies.

## 📸 Game screenshot

<img width="1026" height="607" alt="gameplay" src="https://github.com/user-attachments/assets/ebf213ea-e492-44f0-9748-6e61db6e7634" />

## 🚀 Build and run on Windows

You'll need Git, CMake 3.16 or newer, Ninja, a C++17 compiler, and graphics drivers that support OpenGL 3.3. These commands use GCC from the MSYS2 UCRT64 toolchain. Make sure `git`, `cmake`, `ninja`, and `g++` are available in PowerShell.

```powershell
git clone https://github.com/Ramez-Ihab/Pixels_graphics.git
cd Pixels_graphics
cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cmake --build build
.\build\game.exe
```

If you already have the repository, open PowerShell in the project folder and start with the first CMake command. The first configuration downloads GLFW and GLM, so it needs internet access.

## 🎮 How to play

1. Click **Start** and wait for the intro to finish.
2. Click a unit card, then click a lane to deploy it. The cards are **Melee**, **Ranged**, and **Defense**, from left to right.
3. Click a lane without a card selected to toggle it for dodge controls. Press **W** or **↑** to jump over low shots, or **S** or **↓** to duck under high shots. Defense units hold their position and block.
4. A friendly unit collects a dropped coin automatically when it gets close in the same lane.

Press **Esc** to pause or resume. Clear all three waves to win. If a living enemy reaches the castle, you lose.

## ⚙️ Change the speed and coins

| What to change | Where to edit | How it works |
| --- | --- | --- |
| Walking speed | [`src/Combat.cpp`](src/Combat.cpp) · `WALK_SPEED` and `HARD_ENEMY_SPEED` | Most moving units use `100.f`. Enemy melee units use `120.f`. Increase the number to move faster, or decrease it to move slower. |
| Projectile speed | [`src/Combat.cpp`](src/Combat.cpp) · `SHOT_SPEED` | Defaults to `320.f`. Higher values make shots travel faster. |
| Attack frequency | [`src/Combat.cpp`](src/Combat.cpp) · `ATTACK_COOLDOWN` | Defaults to `1.f` second. A lower value allows more frequent attacks. |
| Time between enemy spawns | [`src/Flow.cpp`](src/Flow.cpp) · `SPAWN_INTERVAL` | The three values are `{5.f, 5.f, 4.f}` seconds, one per wave. Lower values bring enemies out sooner. |
| Starting coins | [`src/Flow.cpp`](src/Flow.cpp) · `resetGame` | Change `g.coins = 30` to the amount you want, such as `g.coins = 100`. This applies when starting or restarting. |
| Coins earned per pickup | [`src/Flow.cpp`](src/Flow.cpp) · `flowUpdate` | Change `g.coins += 10` to adjust the value of each pickup. |
| Unit prices | [`src/Common.h`](src/Common.h) · `COST_MELEE`, `COST_RANGED`, and `COST_DEFENSE` | Change the individual constants to make each card cheaper or more expensive. |

After changing a setting, close the game, rebuild with `cmake --build build`, and run `.\build\game.exe` again.

## ✏️ Edit this README on GitHub

Open `README.md` in the repository and click the pencil icon. Use **Preview** to check your edits, then choose **Commit changes**. The screenshot uses a relative link, so include `assets/gameplay.jpg` in the same branch as the README.
