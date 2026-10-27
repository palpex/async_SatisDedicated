# Ausführung und Completion-Vertrag: verifizierter Stand

Dieser Bericht ersetzt die frühere pauschale Architekturbeschreibung.
Insbesondere sind Aussagen, Rohre, Strom, Fahrzeuge und Züge seien vollständig
seriell oder ein bestimmtes Subsystem sei der Performance-Flaschenhals,
**nicht** durch die vorhandenen Messungen belegt.

## Tatsächlich beobachteter Aufrufvertrag

`evidence/tick-completion-assembly.txt` disassembliert das unveränderte ELF.
`FFactoryTickFunction::ExecuteTick` beginnt bei `0x180cba0`, prüft das Target
bei TickFunction-Offset `0x28` und Objektflags/Objektarray. Es multipliziert
das Delta mit dem Target-Faktor bei `+0x70` und ruft `TickFactory` bei
`0x180cced` synchron auf. Davor und danach liest es die monotone Uhr und
aktualisiert `GTickFactoryTime`.

Für die SysV-ABI liegt `CurrentThread` im eingehenden `edx`, der Verweis auf
`MyCompletionGraphEvent` im eingehenden `rcx`. Der vollständige Funktionskörper
nutzt diese eingehenden Werte nicht, bevor die Register überschrieben werden.
Er übernimmt also im vorhandenen Code keine asynchrone Completion-Verkettung.
Das ist ein Integrationspunkt für einen Ersatz; das Event nur zu ignorieren
und Hintergrundarbeit nach Rückkehr weiterlaufen zu lassen wäre kein
korrekter Ersatzvertrag.

`RegisterFactoryTickFunction` bei `0x187f690` verwendet die eingebettete
TickFunction bei Subsystem-Offset `+0x6d8`, speichert das Target bei `+0x700`,
setzt den Enable-Zustand und registriert sie beim Actor-Level. Der eng
untersuchte Körper allein beweist weder die TickGroup noch die Konfiguration
von `bRunOnAnyThread`; diese stehen außerhalb dieses Nachweises.

## Abgleich mit REA

REA importiert dieses Shared-ELF mit Imagebase `0x100000`. Daher entspricht
ELF `0x187fc40` im Ghidra-Program `0x197fc40`. Die erste numerische Abfrage in
`rea-tick-factory-actors.json` analysierte stattdessen `SteerAvoidFloor` und
wird nicht für Factory-Aussagen verwendet. Symbolbasierte Folgeabfragen liefern:

- `TickFactoryActors`: `rea-corrected-tick-factory-pseudocode.json`,
  Evidence `ev_a43d8a6cbcfd31cab9e0c8c1a1d9716cf6d8ce8790a64cb0fc863fd81ecc4b43`.
- `TickFactory`: `rea-corrected-tick-substeps.json`,
  Evidence `ev_6eb823912cf018cd8b958b2c07ac6243db293d53d5d266190516cd038ac7c9e7`.
- `ExecuteTick`: `rea-corrected-execute-tick.json`,
  Evidence `ev_af97a30125ad8e6b6a1dc92b902ddf2225d9c2f1a4cc5a0465e92b168cc89f2c`.

Dekompilierer-Typen und Pseudocode sind Rekonstruktionen. So sind etliche
Integerfelder im Pseudocode fälschlich als `float` typisiert. ABI und
Zugriffsbreiten sind weiterhin anhand der Assembly zu prüfen.

## Parallelität innerhalb der Subsysteme

`TickFactoryActors` hat drei `ParallelForInternal`-Aufrufstellen. Die dritte
verwendet abhängig vom Konfigurationszweig Callback `$_2` **oder** `$_3`;
sie sind keine zwei immer nacheinander ausgeführten Phasen. Die
Server-/Client-Zweige unterscheiden sich ebenfalls.

`AFGPipeSubsystem::SimulatePipeNetworks` bei ELF `0x1b259b0` sammelt aktive
Netze, liest `CVarUseParallelSimulation` und gibt Netzarbeit an eine
ParallelFor-Instanz. REA-Beleg:
`rea-corrected-pipe-networks.json`, Evidence
`ev_c3d31ccc30cb682f27290560d7c0c3a032663475c1a4775d348225c5504ead3e`.
Das widerlegt die frühere Behauptung, Rohre seien komplett seriell.

## Laufzeitbelege und Grenzen

`runtime/baseline-tick-profile.json` enthält eine REA-Frida-Aufzeichnung der
sechs gewählten Tick-Einstiegspunkte im frischen Test-Save. Sie liefen auf
Thread 172161. Das zeigt ihren Aufrufthread in diesem Lauf, nicht die
Threadzuordnung aller inneren Funktionen, aller Konfigurationen oder Mods.
Der Save enthält keine vom Spieler gebaute Produktionsfabrik und ist kein
repräsentativer Skalierungsbenchmark.

## Noch fehlender Integrationsvertrag

Für den Rewrite fehlen weiterhin die Completion-Verlängerung des Engine-Ticks,
der Schutz gegen Änderungen/GC/Weltwechsel während ausstehender Aufgaben und
ein belegter Datenbesitzvertrag zwischen Gebäude-, Inventar-, Förder-, Strom-
und Flüssigkeitsphasen. Die neue native Task-Brücke prüft erst den Transport
von Aufgaben und Lebensdauer der Callback-Kontexte; sie ersetzt diese
Simulationsregeln nicht.
