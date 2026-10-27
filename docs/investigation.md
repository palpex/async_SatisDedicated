# Untersuchung und Nachweisanforderungen

Ziel bleibt der Umbau des offiziellen Servers zu einer asynchronen,
multithreaded ausgeführten Simulation. Installation, Decompilation oder ein
Thread-Pool allein erfüllen dieses Ziel nicht.

## Zielidentität

Der SteamCMD-Download wurde mit `validate` abgeschlossen. Autoritative Belege:

- `evidence/steamcmd-install-retry.log`: erfolgreicher Download und Validierung.
- `server/steamapps/appmanifest_1690800.acf`: installierter Build und Depots.
- `evidence/rea-factorygame-exports.bundle.json`: REA-Exportinventar mit
  Dateidigest und 50.511 Exporten. Evidence-ID:
  `ev_2443cff916beae8b8616e15d87aa960f0d8ecb04f19e71fad2fb7baab2444351`.

Die exportierte Funktionsliste belegt keine Ausführung oder Thread-Sicherheit.
Debugdateien enthalten Typ-/Zeileninformationen und Symbole, keinen
wiederhergestellten Originalquelltext. Die ersten Compiler-Metadaten nennen
Clang 18.1.0rc und C++14 als DWARF-Sprachkennung; daraus folgt kein vollständiger
Buildvertrag für einen Ersatz.

## Bisherige statische Befunde

Objdump-Belege sind getrennt von REA-Ergebnissen zu behandeln:

| Funktion oder Aufruf | relative ELF-Adresse | Beleg |
| --- | --- | --- |
| `AFGBuildableSubsystem::TickFactory` | `0x187faf0` | `tick-factory-assembly.txt` |
| Aufruf `TickFactoryActors` pro Substep | `0x187fc1b` | `tick-factory-assembly.txt` |
| `AFGBuildableSubsystem::TickFactoryActors` | `0x187fc40` | `tick-factory-assembly.txt` |
| Erster `ParallelForInternal`-Aufruf | `0x187fdf3` | `tick-factory-assembly.txt` |
| Zweiter `ParallelForInternal`-Aufruf | `0x18801da` | `tick-factory-assembly.txt` |
| Dritter `ParallelForInternal`-Aufruf | `0x1880f2d` | `tick-factory-assembly.txt` |
| Instanziiertes `ParallelForInternal` | `0x184eea0` | `parallel-for-assembly.txt` |

Alle Belegpfade beziehen sich auf `evidence/`. Adressen sind buildgebunden und
keine durch ASLR bestimmte Laufzeitadresse.

`TickFactory` schreibt `mCurrentSubStep` und `mCurrentSubStepMax` und ruft in
einer Schleife `TickFactoryActors` auf. Ein asynchrones Zurückkehren über diese
Substep-Grenze würde daher die zeitlichen Annahmen verändern. Welche Daten
zwischen Phasen gelesen und geschrieben werden, muss noch rekonstruiert werden.

`TickFactoryActors` liest unter anderem `CVarUseParallelTick`,
`CVarUseParallelBuildablesTick`, `CVarUseGroupedParallelConveyorsTick`,
`CVarUseParallelConveyorLoadBalancing`, `CVarRunFactoryTickUnbalanced` und
`CVarParallelGroupSize`. Das belegt Auswahlpfade; die tatsächlich zur Laufzeit
aktiven Einstellungen sind bisher nicht gemessen.

Vor der ersten parallelen Phase gibt es virtuelle Aufrufe auf einer Liste bei
`this+0x940`/`this+0x948`. Die Zuordnung zu einem konkreten Interface und dessen
Zugriffsregeln ist noch zu prüfen. Die parallelen Aufrufe bekommen Zeiger auf
lokale Stackdaten. Ein Ersatz darf diese Daten nicht nach Funktionsrückkehr
weiterverwenden.

## Laufender REA-Auftrag

Die CLI analysiert das echte `FactoryGame`-ELF und die Funktion `0x187fc40`.
Ausgabe: `evidence/rea-tick-factory-actors.json`; Fehlerausgabe:
`evidence/rea-tick-factory-actors.stderr.log`. Solange die JSON-Datei leer und
der Prozess aktiv ist, liegt noch kein Funktionsdossier vor. Ghidra meldet
Disassemblierungsprobleme in einigen Exception-Handler-Bereichen; diese müssen
bei der Bewertung der resultierenden Abdeckung erhalten bleiben.

## Nächste Rekonstruktionsschritte

1. Funktionsdossier auswerten und sämtliche drei Callback-Körper untersuchen.
2. Lese-/Schreibmengen von Gebäuden, Förderketten, Inventaren, Strom und
   Flüssigkeitsnetzen einschließlich Handlern bestimmen; Besitz und
   Lebensdauer für Bauen, Abreißen und Weltwechsel festhalten.
3. Synchronisationspunkte auf konkrete Datenabhängigkeiten zurückführen.
4. Tatsächliche Laufzeit mit einer Fabrik messen. Es liegt bisher kein Save
   für reproduzierbare Simulationstests vor.
5. Erst danach passende Phasen durch explizite Aufgabenabhängigkeiten,
   konsistente Zustandsübergänge und abgegrenzte Engine-Zugriffe ersetzen.

## Abschlussbelege für den vollständigen Auftrag

- Der Ersatz muss im offiziellen Server laufen und die bisher seriellen
  berechenbaren Phasen tatsächlich parallel ausführen.
- CPU-lastige Arbeit muss mehrere Kerne gleichzeitig nutzen; asynchrone
  Warteoperationen allein genügen nicht.
- Messung von Tickzeit, Wartezeit und Skalierung mit 1/2/4/... Workern auf
  demselben Save und denselben Eingaben.
- Vergleich von Produktionsmengen, Inventaren, Strom, Flüssigkeiten,
  Transport, Replikation und Save/Load mit dem unveränderten Server.
- Tests für konkurrierendes Bauen/Abreißen, Spieleraktionen, Autosave,
  Shutdown, Fehler und Weltwechsel sowie Race-/Deadlock-Prüfungen.
- Reproduzierbarer Build, Prüfung der unterstützten Serverversion und
  kompatibler Clientbetrieb.

Keiner dieser Abschlussbelege ist bisher erbracht. Der vollständige Rewrite
ist offen, und eine Performanceverbesserung wird nicht behauptet.

## Callback-Befunde aus dem ausgelieferten Binary

`evidence/factory-tick-callbacks-assembly.txt` enthält die vier lokalen
`TFunctionRefCaller`-Körper. Die ersten drei sind oben genannten Aufrufen
zugeordnet; der vierte benötigt weitere Einordnung:

- `$_0` bei `0x189dbe0` liest die Pointerliste `this+0x3c0` mit Länge
  `this+0x3c8` und ruft bei `0x189dc96` VTable-Slot `+0x930` mit dem
  Tick-Delta auf. Die genaue Slot-Signatur muss noch mit Typdaten abgeglichen
  werden; im Bericht wird sie nicht als vollständig rekonstruiert behandelt.
- `$_1` bei `0x189dcc0` liest Gruppen bei `this+0x490`/`this+0x498`.
  Bei `0x189dd24` ruft es explizit
  `AFGConveyorChainActor::Factory_Tick(float)` mit der Summe zweier lokal
  übergebener Zeitwerte auf.
- `$_2` bei `0x189dd80` liest Gruppen bei `this+0x468`/`this+0x470`.
  Ein vorhandener `ChainActor` wird bei `0x189ddd8` direkt getickt;
  andernfalls werden die Conveyor-Pointer bei Gruppenoffset `+8`/`+16`
  verarbeitet. Der Zweig bei `0x189de4e` bis `0x189de65` akkumuliert
  Tick-Delta im Objekt bei `+0x750` anstatt den virtuellen Tick auszuführen.
- `$_3` bei `0x189dec0` liest die Gruppenliste bei
  `this+0x3e8`/`this+0x3f0` und enthält ähnliche Prüfungen vor dem virtuellen
  Tick bei `0x189df1b`.

Die DWARF-Information im **mitgelieferten** `.debug`-Artefakt ordnet den
Code `0x187fc40` der Compilation Unit bei `0x314b779` zu
(`Module.FactoryGame.102.cpp`). Dort steht die vollständige Struktur
`FConveyorTickGroup` bei DIE `0x320bbee`: Größe 32 Byte,
`ForceIntoSharedByBuildable` bei 0, `Conveyors` bei 8 und `ChainActor` bei 24.
Das stützt die Fördergruppeninterpretation. Es belegt nicht, dass mehrere
solche Gruppen voneinander unabhängige Inventare besitzen.

## Fortsetzung: Code und reale Laufzeitbelege

Die erste Ghidra-Abfrage ist abgeschlossen, ihre numerische Adresse wählte
wegen der Imagebase `0x100000` jedoch eine andere Funktion. Korrigierte
symbolbasierte Abfragen und die Abweichung sind in
`docs/threading_architecture_analysis.md` dokumentiert. Das vollständige
REA-Sitzungsbundle ist `evidence/rea-factorygame-session.bundle.json`.

Ein isolierter unveränderter Server wurde auf Loopback gestartet, ein frischer
Save `AsyncRewriteBaseline` erstellt, unpausiert, per API gespeichert und der
Server anschließend heruntergefahren. Save und Einstellungen liegen unter
`.runtime/baseline-home/`; Identität und Tick-Messwerte stehen in
`evidence/runtime/baseline-summary.json`. Der Save enthält keine Fabriklast.

`src/satis_task_bridge.cpp` implementiert nun eine native, buildgebundene Brücke
zum vorhandenen Core-`AsyncTask`. Der echte Server hat 32 von 32 CPU-Aufgaben
ausgeführt und deren Kontexte freigegeben, mit maximal drei gleichzeitig
aktiven Jobs, passender Prüfsumme und ohne Jobs auf dem Game-Thread. Beleg:
`evidence/runtime/task-bridge-probe-second.json`. Der erste Probeversuch
verweigerte die Initialisierung korrekt: Der Symbolwert `GGameThreadId` muss
wegen einer ELF-COPY-Relocation global aufgelöst werden; der Core-lokale
Speicher ist im getesteten Prozess 0. Das wurde vor der erfolgreichen Probe
korrigiert. Ein Batch-/Continuation-Vertrag wird gesondert geprüft.

Der vollständige Rewrite ist weiterhin offen. Die Brücke ist ein konkreter
Integrationsbaustein, kein Beweis, dass die Spielsimulation bereits asynchron
umgebaut ist oder schneller läuft.
