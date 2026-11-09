# netdemo – kleines ELF-/PE-Beispiel für Software Reverse Engineering

Dieses Repository enthält ein kleines C-Programm und einen GitHub-Actions-Workflow,
der daraus automatisch zwei native Executables baut:

- Linux x86-64: ELF mit GCC
- Windows x64: PE mit Microsoft Visual C++ (MSVC)

Die fertigen Dateien werden als GitHub-Actions-Artefakte veröffentlicht und können
anschließend heruntergeladen und beispielsweise in Ghidra oder IDA geöffnet werden.

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

## GitHub-Repository einrichten

1. Lege auf GitHub ein neues Repository an.
2. Entpacke dieses Projekt und lade den gesamten Inhalt in das Repository hoch.
3. Achte darauf, dass `.github/workflows/build.yml` einschließlich des versteckten
   Verzeichnisses `.github` mit hochgeladen wird.
4. Öffne im Repository den Reiter **Actions**.
5. Der Workflow startet bei Push und Pull Request automatisch. Alternativ kann er
   über **Actions → Build ELF and PE examples → Run workflow** manuell gestartet werden.

Nach einem erfolgreichen Lauf findest du auf der Übersichtsseite des Workflow-Runs
den Abschnitt **Artifacts**. Dort liegen:

- `linux-elf-gcc`
- `windows-pe-msvc`

Jedes Artefakt enthält das Executable sowie Textdateien mit ausgewählten Header-,
Section-, Symbol- und Importinformationen. Die Artefakte sind in diesem Workflow
30 Tage verfügbar.

## Lokal kompilieren

### Linux (GCC)

```bash
gcc -std=c11 -Wall -Wextra -O0 -g -fno-pie -no-pie \
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
