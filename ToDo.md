# ToDo

- Runners don't yet path toward a request's destination — `Runner::moveToNextNode` still
  picks a random neighbor regardless of any assigned `Request`. A designated runner should
  route toward `Request::destination` instead.
- Delivered packages are never cleared — `requests`/`packages`/`arrows` only grow; once a
  runner reaches a request's destination, the request should be marked `satisfied` and its
  package/arrow removed.
- `saveNodes` writes to the working directory root (`new_city.map`) instead of `maps/`.
