# Rappi_sim

A delivery-courier simulation: runners wander a node-and-street city graph, shops
periodically spawn delivery requests, and the nearest idle runner is dispatched to the
shop, picks up the package, and routes (shortest path, via Dijkstra) to the destination.

Idle runners are green, runners fulfilling a request are blue. A pending request is drawn
as a dim dotted curve from the shop to its destination; once picked up, that curve turns
blue and is drawn live from the runner's own position.

## Build & run

Requires SFML (`sudo apt-get install libsfml-dev`).

```
make
./rappi_sim
```

## Controls

| Key / input      | Effect                                            |
|-------------------|----------------------------------------------------|
| `E`               | Toggle edit mode (pauses the simulation)            |
| Left click (edit mode) | Select the nearest node                        |
| `S` (edit mode, node selected) | Mark the selected node as a shop        |
| `G` (edit mode)   | Save the current city to `new_city.map`             |
| `L` (edit mode)   | Reload the city from `maps/lion_city.map`           |
| Mouse wheel       | Zoom in/out, centered on the cursor                 |
| Middle-click drag | Pan the view                                        |
| `+`               | Spawn a new runner at a random node                 |
| Speed slider (sidebar) | Drag to scale every runner's movement speed live |

## Layout

```
src/        buildCity.cpp, draw.cpp, main.cpp, pathfinding.cpp, struct.cpp
include/    corresponding headers
maps/       city graphs (.map) plus the lion_city source files (.svg/.ods)
assets/     arial.ttf
```

See [ToDo.md](ToDo.md) for known gaps.
