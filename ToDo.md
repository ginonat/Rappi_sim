# ToDo

- No way to delete a node from the editor (only add/connect/disconnect/move). Doable, but
  needs `nodes` to move from `std::deque` to something that stays safe on arbitrary erase
  (e.g. `std::list`, same reasoning as the `requests` fix earlier) - a `deque` only
  guarantees other elements survive `push_back`, not an erase from the middle.
