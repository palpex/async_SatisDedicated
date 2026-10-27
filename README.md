# async_SatisDedicated

Asynchroner und Multithreaded-Architektur-Umbau für den offiziellen **Satisfactory Dedicated Server** (Unreal Engine 5).

---

## 1. Projektübersicht & Motivation

Satisfactory (Coffee Stain Studios) führt in seinem Dedicated Server umfangreiche Simulationen aus (Fabrikgebäude, Förderbänder, Rohrnetze, Stromkreise, Fahrzeuge und Eisenbahnen). Standardmäßig läuft der Hauptsimulations-Tick (`FFactoryTickFunction::ExecuteTick` und `AFGBuildableSubsystem::TickFactory`) synchron auf dem Game-Thread. Obwohl interne Phasen teilweise `ParallelFor`-Aufrufe nutzen, blockieren diese den Game-Thread synchron bis zum Abschluss aller Substeps, was bei großen Fabriken zu starken FPS- und Tickrate-Einbrüchen führt.

Dieses Projekt analysiert die native Simulationsarchitektur des offiziellen Linux-Servers mittels Reverse Engineering (Ghidra, REA) und implementiert eine **native Task-Brücke (`satis_task_bridge`)**, die über die interne Unreal Engine 5 TaskGraph-ABI (`AsyncTask`, `FBaseGraphTask`, `FFunctionGraphTask`) echte asynchrone Multithreading-Verarbeitung ermöglicht.

---

## 2. Ausgangspunkt & Binärbasis (SteamCMD)

Da Coffee Stain Studios keinen Quellcode für Satisfactory bereitstellt, basiert dieses Projekt auf den offiziellen Linux Dedicated Server Binaries, bezogen über SteamCMD:

- **Steam App ID:** `1690800` (Satisfactory Dedicated Server)
- **Linux Depot ID:** `1690802`
- **Branch / Build:** `public` / Build `25782485`
- **FactoryGame ELF Binary:** `libFactoryServer-FactoryGame-Linux-Shipping.so` (SHA-256: `637c2c70e65f6386697a6a5f5746f64b187d2fcfeee56df1aeef3f172268c0c1`)
- **Core Engine Binary:** `libFactoryServer-Core-Linux-Shipping.so` (GNU Build-ID: `36a6397531db8df2`)
- **Werkzeuge:** REA 6.8.0, Ghidra 12.1.4, Clang / GCC C++17, Python 3.10.

---

## 3. Reverse-Engineering-Ergebnisse

Detaillierte Berichte finden sich in [`docs/investigation.md`](docs/investigation.md) und [`docs/threading_architecture_analysis.md`](docs/threading_architecture_analysis.md).

### 3.1 Tick-Ausführungskette
1. **`FFactoryTickFunction::ExecuteTick` (Offset `0x180cba0`):**
   - Wird vom UE-TickManager aufgerufen.
   - Übergibt in SysV-ABI `CurrentThread` (Register `edx`) und `TRefCountPtr<FBaseGraphTask> MyCompletionGraphEvent` (Register `rcx`).
   - Das Original ignoriert `MyCompletionGraphEvent` und ruft synchron `AFGBuildableSubsystem::TickFactory` auf.
2. **`AFGBuildableSubsystem::TickFactory` (Offset `0x187faf0`):**
   - Schreibt Substep-Zähler (`mCurrentSubStep`) und ruft pro Substep `TickFactoryActors` auf.
3. **`AFGBuildableSubsystem::TickFactoryActors` (Offset `0x187fc40`):**
   - Enthält drei `ParallelForInternal`-Aufrufstellen mit `TFunctionRefCaller`.
   - Verarbeitet Conveyor-Tick-Gruppen (`FConveyorTickGroup`).
4. **`AFGPipeSubsystem::SimulatePipeNetworks` (Offset `0x1b259b0`):**
   - Führt Flüssigkeitssimulationen aus und besitzt einen Parallelpfad via `CVarUseParallelSimulation`.

---

## 4. Native Task-Brücke (`satis_task_bridge`)

Die Brücke (`src/satis_task_bridge.cpp`, `include/satis_task_bridge.h`) bindet sich an den internen TaskGraph des Servers an:

- **Dynamische Symbolauflösung & Schutzprüfung:**
  - Verifiziert GNU Build-ID `36a6397531db8df2` von `libFactoryServer-Core-Linux-Shipping.so`.
  - Löst `AsyncTask` bei Offset `0x47b060` auf und verifiziert die Opcode-Signatur.
  - Löst `GGameThreadId` über globale ELF-COPY-Relocations auf.
  - Bindet `FBaseGraphTask`-Allokation (`0x4ac890`), Konstruktion (`0x4acb90`), Unlock/Signal (`0x4aa520`) und `create_with_prerequisite` (`0x4a38e0`).
- **Batching (`satis_task_batch`):**
  - Fächert $N$ Aufgaben auf Worker-Threads aus (`ENamedThreads::AnyNormalThreadNormalTask = 255`).
  - Löst genau eine Game-Thread-Continuation (`ENamedThreads::GameThread = 1`) aus, sobald alle Worker fertig sind und ihre Kontexte freigegeben wurden.
- **Graph Event Completion (`satis_completion_*`):**
  - Ermöglicht das Erzeugen von Graph-Events (`satis_completion_create`).
  - Hängt abhängige Tasks an (`satis_completion_then`), die erst nach explizitem Signal (`satis_completion_signal`) freigegeben werden.

---

## 5. Echte Laufzeit-Verifikation & Testergebnisse

Die Funktionalität wurde im laufenden Satisfactory Dedicated Server via Frida-Instrumentation und isolierten Unit-Tests nachgewiesen:

1. **Batch-Probe (`evidence/runtime/task-batch-probe.json`):**
   - 32 Aufgaben auf Worker-Threads ausgeführt.
   - Max Concurrency: 3 Kerne gleichzeitig aktiv.
   - Prüfsumme stimmt exakt mit serieller Referenz überein.
   - Kein Worker-Job auf dem Game-Thread ausgeführt (`wrongThread = 0`).
   - Genau eine Fortsetzung auf dem Game-Thread nach vollständigem Batch-Abschluss.
2. **Completion-Event-Probe (`evidence/runtime/completion-probe.json`):**
   - Worker-Aufgaben blieben vor Signalisierung blockiert.
   - Nach `satis_completion_signal` exakt 1 Ausführung der Watcher-Continuation.
   - `earlyWatcher = 0`, `failures = 0`.
3. **Standalone Safety & Invarianten (`tests/test_bridge_safety.py`):**
   - Bei Ausführung außerhalb des Servers verweigert die Bibliothek die Initialisierung sicher und stürzt niemals ab.

---

## 6. Bauen & Testen

### Voraussetzungen
- CMake 3.20+
- GCC / Clang mit C++17 Unterstützung
- Python 3.10+ mit `pytest`

### Kompilieren
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j$(nproc)
```

### Tests ausführen
```bash
# Unit Tests (Pytest)
pytest -v tests/

# Vollständige Evidenz- und Manifest-Validierung
python3 tests/verify_runtime_evidence.py
```

---

## 7. Projektstruktur

```
.
├── CMakeLists.txt              # CMake Build-Konfiguration
├── README.md                   # Dieses Dokument
├── docs/                       # Vertiefende Architektur- und Analyseberichte
│   ├── investigation.md        # REA- und Ghidra-Untersuchungsbericht
│   ├── threading_architecture_analysis.md # Analyse der Subsysteme und Ticks
│   ├── task-bridge.md          # Spezifikation der Task-Brücke & ABI
│   └── next-step.md            # Nächste Integrationsschritte
├── include/
│   └── satis_task_bridge.h     # Öffentlicher C-Header der Task-Brücke
├── src/
│   └── satis_task_bridge.cpp   # Implementierung der Unreal Engine TaskGraph-Brücke
├── instrumentation/            # Frida-Instrumentierungsskripte für Live-Proben
│   ├── task-bridge-probe.js    # Batching-Probe im Server
│   └── completion-probe.js     # Graph-Event-Completion-Probe
├── evidence/                   # Verifizierte JSON-Evidenzen und Disassemblies
│   ├── runtime/                # Echte Server-Messprotokolle und Build-Manifeste
│   └── rea-factorygame-session.bundle.json.gz # REA-Ghidra-Sitzungsbundle (komprimiert)
├── scripts/                    # Analyse- und REA-Hilfsskripte
└── tests/                      # Automatisierte Testsuite
    ├── test_bridge_safety.py   # Standalone- und Boundary-Tests
    ├── test_evidence_integrity.py # Prüfsummen- und Evidenzprüfungen
    └── verify_runtime_evidence.py # Validierung der Server-Laufzeitproben
```

---

## 8. Autoren & Danksagung

- **Lead:** palpex
- **Co-Author:** cj27022009-blip

