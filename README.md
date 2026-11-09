# netdemo – kleines ELF-/PE-Beispiel für Software Reverse Engineering

Dieses Repository enthält ein kleines C-Programm und einen GitHub-Actions-Workflow,
der daraus automatisch zwei native Executables baut:

- Linux x86-64: ELF mit GCC
- Windows x64: PE mit Microsoft Visual C++ (MSVC)

Die fertigen Dateien werden bei jedem Push automatisch gebaut und als Release
veröffentlicht. Sie können direkt heruntergeladen und beispielsweise in Ghidra oder
IDA geöffnet werden — siehe [Fertige Dateien herunterladen](#fertige-dateien-herunterladen).

## Was macht das Programm?

`netdemo.c` führt diese Schritte aus:

1. Liest `USER` (Linux) beziehungsweise `USERNAME` (Windows).
2. Löst standardmäßig `www.example.com` mit `getaddrinfo()` auf.
3. Versucht, eine TCP-Verbindung zu Port 443 aufzubauen.
4. Schließt den Socket und gibt die Adressliste frei.

**Wichtig:** Das Programm führt keinen TLS-Handshake aus und sendet keine HTTP- oder
HTTPS-Anwendungsdaten. Port 443 ist hier lediglich das Ziel des TCP-Verbindungsversuchs.

Der Hostname kann als erstes Kommandozeilenargument überschrieben werden:

```bash
./netdemo-linux www.example.org
```

Unter Windows:

```cmd
netdemo.exe www.example.org
```

## Fertige Dateien herunterladen

Jeder Push auf `main` baut die Executables neu und veröffentlicht sie im Release
[**latest**](https://github.com/ChrisDietrich/netdemo/releases/latest). Diese Links
bleiben dauerhaft gültig und zeigen immer auf den jeweils neuesten Build — sie müssen
nach einer Codeänderung also nicht angepasst werden:

| Datei | Direktlink |
| --- | --- |
| ELF (Linux x86-64) | [netdemo-linux](https://github.com/ChrisDietrich/netdemo/releases/latest/download/netdemo-linux) |
| PE (Windows x64) | [netdemo.exe](https://github.com/ChrisDietrich/netdemo/releases/latest/download/netdemo.exe) |
| ELF-Metadaten (`readelf`-Ausgaben) | [netdemo-elf-metadata.zip](https://github.com/ChrisDietrich/netdemo/releases/latest/download/netdemo-elf-metadata.zip) |
| PE-Metadaten (`dumpbin`-Ausgaben) | [netdemo-pe-metadata.zip](https://github.com/ChrisDietrich/netdemo/releases/latest/download/netdemo-pe-metadata.zip) |

Auf der Kommandozeile:

```bash
curl -LO https://github.com/ChrisDietrich/netdemo/releases/latest/download/netdemo-linux
curl -LO https://github.com/ChrisDietrich/netdemo/releases/latest/download/netdemo.exe
chmod +x netdemo-linux
```

Der Download aus dem Release funktioniert **ohne GitHub-Konto**.

### Alternative: Artefakte eines einzelnen Workflow-Runs

Unter [**Actions**](https://github.com/ChrisDietrich/netdemo/actions) liegt zu jedem
Lauf im Abschnitt **Artifacts** zusätzlich `linux-elf-gcc` und `windows-pe-msvc`.
Das ist nützlich, um einen *bestimmten* älteren Stand zu betrachten. Zwei
Einschränkungen gegenüber den Release-Links oben: der Download erfordert einen
GitHub-Login, und die Artefakte werden nach 30 Tagen automatisch gelöscht.

## Lokal kompilieren

### Linux (GCC)

```bash
gcc -std=c11 -D_POSIX_C_SOURCE=200112L \
    -Wall -Wextra -O0 -g -fno-pie -no-pie \
    -o netdemo-linux netdemo.c
```

ELF-Informationen anzeigen:

```bash
file netdemo-linux
readelf -h netdemo-linux
readelf -l netdemo-linux
readelf -S netdemo-linux
readelf -d netdemo-linux
readelf -Ws netdemo-linux
```

### Windows (MSVC)

In einer **x64 Native Tools Command Prompt for Visual Studio**:

```cmd
cl /nologo /std:c11 /W4 /Od /Zi /MD netdemo.c Ws2_32.lib /Fe:netdemo.exe
```

PE-Informationen anzeigen:

```cmd
dumpbin /headers netdemo.exe
dumpbin /imports netdemo.exe
```

## Hinweise für die Analyse

Mögliche Untersuchungsziele in Ghidra:

- ELF- beziehungsweise PE-Header und Architektur
- Entry Point im Vergleich zu `main()`
- Program Header / Segmente und Sections
- dynamische Symbole und Bibliotheksabhängigkeiten
- Aufrufe von `getenv()`, `getaddrinfo()`, `socket()`, `connect()`
- Unterschiede zwischen Linux-C-Laufzeit und Windows-Winsock

Nicht jeder Funktionsname muss in jeder Build-Konfiguration als dynamischer Import
sichtbar sein. Das hängt unter anderem von Compiler, Linker, Optimierung und
Laufzeitbibliothek ab.

## Netzwerk- und Laufzeithinweise

Für die DNS-Auflösung und den TCP-Verbindungsversuch benötigt das Programm eine
Netzwerkverbindung. DNS kann durch lokale Caches oder die Hosts-Datei beantwortet
werden. Firewalls, Proxies oder Netzrichtlinien können den Verbindungsversuch
blockieren. Ein fehlgeschlagener Verbindungsversuch bedeutet daher nicht automatisch,
dass das Programm fehlerhaft ist.

Das Programm kommuniziert nur mit dem angegebenen Host auf TCP-Port 443. Es sendet
keine Nutzdaten und führt keinen TLS-Handshake durch. Für Übungen sollte ein
freigegebener, kontrollierter Zielhost verwendet werden.

## Lizenz

Dieses Lehrbeispiel steht unter der MIT-Lizenz. Siehe `LICENSE`.
