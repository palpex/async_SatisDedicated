# Nächster notwendiger Schritt

Der vorangehende Zielturn hat Code und autoritative Laufzeitevidenz erzeugt:
Die native Task-Brücke und die Batch-Fortsetzung wurden im offiziellen Server
geprüft. Dies ist Fortschritt, aber kein vollständiger Simulationsrewrite.

Vor weiteren Änderungen `python3 tests/verify_runtime_evidence.py` lesen/laufen
lassen und die gespeicherten Hashes gegen den aktuellen Code vergleichen.
Der FactoryGame-Hash ist unverändert. Alle Testserver sind über ihre API
heruntergefahren; Shutdownlogs und `runtime-state.json` liegen unter evidence.

Die engste offene Integrationsfrage ist das Verlängern des vorhandenen
Engine-Completion-Events bei `FFactoryTickFunction::ExecuteTick`. `rcx` trägt
beim Eintritt den Verweis auf `TRefCountPtr<FBaseGraphTask>`. Der Originalcode
nutzt ihn nicht. Ein asynchroner Ersatz muss seine Arbeit daran binden,
inklusive sämtlicher Substeps und Pre-/Post-Handler. Ohne diesen Vertrag dürfen
keine existierenden UObject-Operationen über die originale Tick-Rückkehr hinweg
ausgelagert werden.

Core-DWARF: `FBaseGraphTask`/`UE::Tasks::Private::FTaskBase` Größe 168,
`AddNested`/`DontCompleteUntil` deklarierte Inline-Methoden. Vollständige
Memberdaten unter `evidence/core-task-dwarf.json`. `AsyncTask` ist exportiert,
`FFunctionGraphTask::CreateAndDispatchWhenReady` liegt lokal bei `0x4acf90`
(Array-Voraussetzungen) bzw. `0x4a38e0` (einzelne Voraussetzung).
Noch nicht als sichere ABI aufrufen, bis Parameter, sret, RefCounts und
Synchronisation nachgewiesen sind.

Die letzte REA-Sitzung hat folgende korrigierte Funktionen als standalone
Evidence gespeichert; die `.c`-Dateien sind nur abgeleitete Lesehilfen:
`TickFactoryActors`, `TickFactory`, `ExecuteTick`, `RegisterFactoryTickFunction`,
`SimulatePipeNetworks`, `ConveyorChainActor::Factory_Tick` und Callbacks 0/1/2.
Imagebase ist 0x100000. Symbolnamen bevorzugen. Das erste alte Dossier bei
numerisch 0x187fc40 gehört fälschlich SteerAvoidFloor und ist quarantänisiert
im Bericht; nicht als Factory-Beleg verwenden.

Nicht erneut stundenlang dieselben Programme analysieren. Weitere Queries
müssen dieselbe aktive REA-Sitzung teilen, oder benötigte Aufgaben gesammelt
im `scripts/rea_session.mjs`-Client ausführen. Der SDK v2 erwartet
`callTool(params, options)`; das frühere dritte Argument bewirkte einen
60-Sekunden-Clientabbruch und wurde korrigiert. Der Client legt alle
angekündigten Schemas ab und nutzt nur existierende Tools.

Die Subagent-Berichte enthielten mehrere unbewiesene Behauptungen und wurden
vom Parent auf konkrete Nachweise reduziert. Insbesondere sind Pipes nicht
vollständig seriell. Das frische Baseline-Save hat keine Produktionsfabrik;
Skalierungs- und Semantiktests brauchen noch tatsächliche Fabriklast.
