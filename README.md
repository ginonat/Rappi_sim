# Rappi_sim

A delivery-courier simulation: runners random-walk a node-and-street city graph, shops
periodically spawn delivery requests, and requests are drawn as a package marker with an
arrow to their destination.

## Build & run

Requires SFML (`sudo apt-get install libsfml-dev`).

```
make
./rappi_sim
```

## Controls

| Key / input     | Effect                                          |
|------------------|--------------------------------------------------|
| `E`              | Toggle edit mode (pauses the simulation)          |
| Left click (edit mode) | Select the nearest node                     |
| `S` (edit mode, node selected) | Mark the selected node as a shop     |
| `G` (edit mode)  | Save the current city to `new_city.map`           |
| `L` (edit mode)  | Reload the city from `maps/lion_city.map`         |
| Mouse wheel      | Zoom in/out, centered on the cursor               |
| `+`              | Spawn a new runner at a random node               |

## Layout

```
src/        buildCity.cpp, draw.cpp, main.cpp, struct.cpp
include/    corresponding headers
maps/       city graphs (.map) plus the lion_city source files (.svg/.ods)
assets/     arial.ttf
```

See [ToDo.md](ToDo.md) for known gaps.
