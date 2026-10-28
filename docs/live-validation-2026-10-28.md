# Live validation (2026-10-28)

A bounded observation was collected while one player was connected. The server process was not restarted and no gameplay or save setting was changed.

## Result

- Async bridge was enabled for the entire window.
- 1,784 of 1,784 intercepted factory ticks were dispatched to workers and completed.
- No synchronous fallbacks occurred during the window.
- The bridge reported no in-flight backlog at each sample.
- API-reported server tick rate ranged from 29.71 to 29.88 TPS (mean 29.78 TPS).
- Journal counts during the window: 0 assertions, 0 fatal signals, 0 critical errors.
- The one connected player remained connected and the server stayed in a running, unpaused state.

## Interpretation

This confirms that the current guarded async path remains active and stable during a live multiplayer session. The window did not include a controlled sequence of machine placement, dismantling, or inventory transfers, so it cannot prove safety for every interaction. Engine inventory warnings observed outside the window are existing gameplay warnings and were not fatal errors.

The raw observation is kept on the host because it includes operational process metadata. The repository records only these anonymized aggregates.
