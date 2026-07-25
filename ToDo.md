# ToDo

- Satisfied requests are never pruned — `requests`/`packages` only grow over a long session
  (they're just skipped when rendering/counting once `satisfied` is true). Since runners hold
  a raw `Request*` into the `requests` deque, pruning needs to either null out/clear those
  pointers first or switch to something safer than raw pointers (e.g. an index or shared
  ownership) before elements can be erased.
- `saveNodes` writes to the working directory root (`new_city.map`) instead of `maps/`.
