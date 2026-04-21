# Setup VS Code pour Minotaur (Arduboy)

Ce guide remet en place un environnement de travail simple pour coder et tester le jeu Arduboy sous Linux avec VS Code.

## 1) Prerequis Linux

Installer `arduino-cli`:

```bash
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
sudo mv bin/arduino-cli /usr/local/bin/
arduino-cli version
```

Donner les droits serie au user (une fois):

```bash
sudo usermod -aG dialout "$USER"
```

Puis se deconnecter/reconnecter a la session Linux.

## 2) Ouvrir le projet et configurer VS Code

Le depot contient deja des taches VS Code dans [../.vscode/tasks.json](../.vscode/tasks.json) et des parametres dans [../.vscode/settings.json](../.vscode/settings.json).

Pour une nouvelle machine, partir de [../.vscode/settings.template.json](../.vscode/settings.template.json) et adapter les valeurs locales (port COM, chemin CLI Windows, etc.).

Valeurs a verifier dans `settings.json`:

- `minotaur.fqbn`: carte cible (par defaut `arduino:avr:leonardo`)
- `minotaur.port`: port serie (par defaut `/dev/ttyACM0`)
- `minotaur.monitorBaudRate`: vitesse du moniteur serie

## 3) Initialiser l'outillage Arduino

Depuis VS Code:

1. `Ctrl+Shift+P`
2. `Tasks: Run Task`
3. Lancer `Arduino: Setup Toolchain`

Cette tache installe:

- le core `arduino:avr`
- la librairie `Arduboy2`

## 4) Build et upload

Avec l'Arduboy branche:

1. Lancer `Arduino: Detect Ports` pour confirmer le port
2. Mettre a jour `minotaur.port` si besoin
3. Lancer `Arduino: Build + Upload`

## 4bis) Workflow recommande sous WSL (plus fiable)

Sous WSL, le Leonardo peut se deconnecter/reconnecter pendant le reset bootloader, ce qui peut casser l'upload Linux.

Workflow recommande:


1. Build cote Windows avec la tache `Arduino: Build (Windows)`
2. Upload cote Windows avec la tache `Arduino: Upload (Windows COM)`
3. Ou directement `Arduino: Build + Upload (Windows COM)`
4. Option la plus simple: `Arduino: Flash Minotaur` (verification COM + build + upload)

Parametres associes dans [../.vscode/settings.json](../.vscode/settings.json):

- `minotaur.arduinoCliWindows` (par defaut `arduino-cli.exe`, ou chemin complet vers le binaire)
- `minotaur.portWindows` (par defaut `COM7`)
- `minotaur.usbBusIdWindows` (optionnel, vide par defaut)

Note: si `minotaur.usbBusIdWindows` est renseigne, la tache `Arduino: Upload (Windows COM)` detache automatiquement ce bus USB de WSL avant l'upload pour rendre le port COM accessible a Windows.

Prerequis Windows:

- `arduino-cli.exe` installe et accessible dans le PATH Windows, ou `minotaur.arduinoCliWindows` pointe vers son chemin complet
- Device visible dans `usbipd list` puis attache a WSL si besoin

Si la tache `Arduino: Verify Port (Windows COM)` echoue avec un message du type `arduino-cli.exe not found`, renseigner le chemin complet dans [../.vscode/settings.json](../.vscode/settings.json), par exemple:

`C:\\Program Files\\Arduino IDE\\resources\\app\\lib\\backend\\resources\\arduino-cli.exe`

## 5) Debug pratique sur Arduboy

Le debug pas-a-pas materiel n'est pas disponible nativement comme sur un MCU avec probe JTAG.

Workflow recommande:

- debug logique via logs `Serial.print` (si active dans le code)
- moniteur serie via la tache `Arduino: Monitor`
- debug gameplay/UX via tests rapides sur hardware (latence, collisions, rendu)

## 6) Points utiles pour la suite

- Sketch principal: [../src/src.ino](../src/src.ino)
- Tache build: `Arduino: Build`
- Tache upload: `Arduino: Upload`
- Tache monitor: `Arduino: Monitor`

Quand tu branches ton Arduboy, on peut faire ensemble un premier cycle complet `Build + Upload` puis ajouter une boucle de debug efficace (logs, toggles de debug, petits tests de scene).
