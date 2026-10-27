# Native Engine-Task-Brücke

`src/satis_task_bridge.cpp` und `include/satis_task_bridge.h` bilden einen
experimentellen Integrationsbaustein für den geplanten Ersatz der Factory-
Simulationsphasen. Sie rufen das vorhandene Core-`AsyncTask` auf und planen
CPU-Arbeit auf Engine-Worker sowie Fortsetzungen auf dem Game-Thread.

## Build und Nachweis

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j2
pytest -v tests/
python3 tests/verify_runtime_evidence.py
```

Die Verifikationsskripte prüfen gespeicherte echte Laufzeitevidenzen und die Hashes der
getesteten Dateien gegen die Build-Manifeste.

## Bisherige Laufzeit-Proben im offiziellen Server

### 1. Task-Batch-Probe (`evidence/runtime/task-batch-probe.json`)
Protokolliert die Ausführung über REA/Frida im offiziellen Server:
- 32 angenommene, ausgeführte und freigegebene Jobs.
- Maximal 3 gleichzeitig aktive Worker-Jobs.
- Prüfsumme identisch zur seriellen Referenz derselben reinen Berechnung.
- Keine Worker-Aufgabe auf dem Game-Thread.
- Genau eine Fortsetzung auf dem Game-Thread nach Abschluss und Context-Freigabe aller Jobs.
- Zähler offener Aufgaben anschließend 0, keine gemeldeten Skript- oder Cleanupfehler.
- Server-HealthCheck nach Probe: healthy.

### 2. Graph-Event-Completion-Probe (`evidence/runtime/completion-probe.json`)
Protokolliert die Ausführung mit echter Unreal Engine `FBaseGraphTask`-Completion:
- 32 Worker-Aufgaben wurden erst nach Signalisierung des Completion-Events fortgesetzt.
- Genau 1 Watcher-Aufruf auf dem Game-Thread nach Signalisierung (`watcherCalls=1`, `watcherDisposals=1`).
- Kein verfrühtes Auslösen vor Signalisierung (`earlyWatcher=0`).
- Keine Ausführung auf falschem Thread (`wrongThread=0`).
- 0 gemeldete Fehler (`failures=0`).
- Identische Prüfsumme (`checksum == expectedChecksum`).

## Rekonstruierter ABI-Vertrag

Core `.debug`, CU `0x22`:

- `TUniqueFunction<void()>`: 48 Byte, 16-Byte-Ausrichtung.
- `Callable` bei 0, `Storage` bei 16.
- `FFunctionStorage`: 32 Byte, Inlinebereich bei 0, Heapzeiger bei 24.
- `IFunction_OwnedObject`: VTable-Slots clone=0, address=1, destroy=2.
- `ENamedThreads::GameThread=1`, `AnyNormalThreadNormalTask=255`.

Zusätzlich für Graph-Events rekonstruiert (`libFactoryServer-Core-Linux-Shipping.so`):
- `event_allocate` (Offset `0x4ac890`): Allokiert 168 Bytes für `FBaseGraphTask` / `FGraphEvent`.
- `event_construct` (Offset `0x4acb90`): Initialisiert das Graph-Event.
- `task_unlock` (Offset `0x4aa520`): Entsperrt das Event und weckt abhängige Tasks auf.
- `create_with_prerequisite` (Offset `0x4a38e0`): Erstellt Task mit Event-Voraussetzung.

Die Brücke verwendet stabile Heapobjekte und hält keine Referenzen auf den
Stack des aufrufenden Submitters zurück.

Initialisierung verweigert unbekannte Core-Versionen anhand des GNU-Build-ID
`36a6397531db8df2`, des Exportoffsets `0x47b060` und der Eintrittsinstruktionen.
Die ELF-COPY-Relocation des Hauptprogramms ist wesentlich: `GGameThreadId`
muss global aufgelöst werden. Core-lokaler Symbolspeicher enthielt in der
Laufzeitprobe 0, die globale Kopie die tatsächliche Thread-ID.

## API und Besitz

`satis_task_enqueue` übernimmt Context-Besitz ausschließlich bei Rückgabewert
0. Nach `run` wird optional `dispose` genau einmal ausgeführt.

`satis_task_batch` allokiert zunächst sämtliche Callback-Objekte. Bei einem
Fehler vor der Veröffentlichung bleibt der Besitz aller Contexts beim
Aufrufer. Nach erfolgreicher Veröffentlichung läuft die Fortsetzung erst,
wenn jeder Job und dessen Context-Freigabe abgeschlossen ist.

`satis_completion_create` erstellt ein neues Graph-Event mit externer Referenz.
`satis_completion_then` hängt einen Callback an das Event an, der erst nach
`satis_completion_signal` auf dem gewünschten Thread (z. B. GameThread) ausgeführt wird.
`satis_completion_release` gibt das Event und die Referenzen sauber frei.

