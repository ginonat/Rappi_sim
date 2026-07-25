# Rappi_sim

A small delivery-courier simulation, named after [Rappi](https://www.rappi.com/), the
Latin American delivery app. Runners wander a node-and-street city graph. Shops
periodically place delivery requests; the nearest idle runner is dispatched to the shop,
picks up the package, and takes the shortest path (Dijkstra) to the destination.

![Rappi_sim screenshot](assets/screenshot.png)

## Features

- **Start menu** — pick which city to load from a "Load Map" button; every `.map` file in
  `maps/` shows up automatically.
- **A real map editor** — add nodes, connect/disconnect them, drag them around (individually
  or as a multi-select group via a drag-box), delete a selection, snap to a grid, or
  drag out a fresh NxM grid sized to two corners you define, all without leaving the app.
- **Tiered streets** — a connection can be Normal, Long (10x), or Very Long (100x) the
  distance it looks on screen. Drawing that to scale would blow the map up, so instead
  it's color-coded (white/orange/red) and crossed proportionally slower — and routing
  (`findPath`) treats it as genuinely farther, not just visually shorter.
- **Real routing** — deliveries follow an actual shortest path over the street graph
  (`src/pathfinding.cpp`), not a straight line or a random walk.
- **Live status at a glance** — idle runners are green, runners fulfilling an order are
  blue; a pending order shows as a dim dotted curve from the shop to its destination, and
  turns into a blue curve drawn live from the runner once picked up.
- **A HUD sidebar** with the control legend and live runner/shop/request counts.
- **A simple delivery economy** — every request rolls a random item cost (bell-curve
  distributed) and accrues a logistics fee proportional to the tiered distance the runner
  actually travels to fulfill it (pickup leg + delivery leg). Item cost mean/std dev, the
  fee rate, and both the runner-speed and simulation-speed dials live behind the
  **Settings** button so they can be tuned live without touching code.
- **A Stats panel** (**Stats** button) charting runner utilization (idle vs. active count)
  and cumulative spend (goods vs. logistics) over time, plus running totals.
- **Free camera** — scroll to zoom (anchored under the cursor), middle-click drag to pan.

## Build & run

Requires SFML 2.5:

```bash
sudo apt-get install libsfml-dev
make
./rappi_sim
```

## Controls

On launch, click **Load Map** and pick a city to start. In the simulation:

| Key / input                          | Effect                                              |
|----------------------------------------|--------------------------------------------------------|
| `E`                                   | Toggle edit mode (pauses the simulation)               |
| Left click a node (edit mode)         | Select just that node                                  |
| Left click empty space (edit mode)    | Add a new node there                                   |
| Drag empty space (edit mode)          | Selection box — selects every node inside it           |
| Shift+drag empty space (edit mode)    | Same, but adds to the current selection instead of replacing it |
| Drag a selected node (edit mode)      | Move the whole selection together, spacing preserved   |
| Shift+click a node (edit mode)        | Toggle a connection between it and every selected node |
| `1` / `2` / `3` (edit mode)           | Set the tier for new connections (Normal/Long/Very Long); if 2+ nodes are selected, also retiers every existing connection *within* that selection |
| `Delete` / `Backspace` (edit mode)    | Delete every selected node (and their connections)     |
| `N` (edit mode)                       | Toggle grid snap (25px) for placing/dragging nodes      |
| `Up`/`Down`/`Left`/`Right` (edit mode)| Adjust the pending grid tool's rows/cols                |
| `C` (edit mode)                       | Arm the grid tool — the next drag defines two corners and adds that NxM grid there (selected, ready to connect to the rest of the map), leaving the existing map intact (`Esc` or `C` again cancels) |
| `S` (edit mode)                       | Mark every selected node as a shop                      |
| `G` (edit mode)                       | Save the current city to `maps/new_city.map`            |
| `L` (edit mode)                       | Reload the current city from disk                       |
| Mouse wheel                           | Zoom in/out, centered on the cursor                     |
| Middle-click drag                     | Pan the view                                            |
| `+` (not in edit mode)                | Spawn a new runner at a random node                     |
| `Settings` button (sidebar)           | Open/close the settings panel: runner speed, simulation speed, item cost mean/std dev, fee per distance — each a draggable dial |
| `Stats` button (sidebar)              | Open/close the stats panel: runner utilization and cumulative spend charts, plus running totals |

Both panels capture the click that opens them; while one is open, clicking anywhere outside
it closes it again instead of falling through to the map underneath.

## How a delivery happens

1. A shop rolls the dice each frame (`requestProbability`, scaled by simulation speed) and,
   on a hit, creates a `Request` with a random destination node and a random item cost
   (`randomBellCurve(itemCostMean, itemCostStdDev)`, floored above zero) — the item is
   "bought" immediately, so its cost is added to the goods-spend total right away.
2. The nearest idle runner (straight-line distance) is dispatched: `findPath` computes the
   route to the shop, and the runner walks it one node at a time. The tiered distance of
   that leg (`pathDistance`), times the fee rate, is added to the request's logistics fee.
3. On arrival, the runner "picks up" the package, `findPath` computes a fresh route to the
   destination (its tiered distance again added to the fee), and the request's
   visualization switches to the blue in-transit style.
4. On arrival at the destination, the request is marked satisfied and pruned from the
   request list; its now-final logistics fee is added to the logistics-spend total shown
   in the Stats panel, and the runner goes back to idle wandering.

## Project layout

```
src/        buildCity.cpp, draw.cpp, main.cpp, pathfinding.cpp, struct.cpp
include/    corresponding headers
maps/       city graphs (.map) plus the lion_city source files (.svg/.ods)
assets/     arial.ttf, screenshot.png
```

See [ToDo.md](ToDo.md) for known gaps.
