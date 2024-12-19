# Project Room Ventilation

Group Nr: 3

## Students:  

Timur Ali Basnakajev
Tobias König
Jessica Marban
Patrick Neumann

## Description

When the CO2 level in a room reaches a certain level, a window should automatically
open and a fresh air fan should switch on. After a certain lower CO2 level is reached and
a certain run-on time has elapsed, the window is closed again and the fan is switched off
again.

Futher Details: https://github.com/apg1034/RoomVentilation/blob/main/IDP_Designdokument/start.pdf

## Basic documentation in German language

# Interdisziplinäres Projekt

## Einleitung

An der FH Campus Wien wurde im Rahmen eines Interdiszipliären Projekts ein selbst gewähltes Projekt umgesetzt.

Die Umsetzung erfolgte im wesentlichen in drei Schritten:

1. Finden einer Idee
2. Erstellung eines Design Dokumentes (Anforderungen, Data Flow Diagram, Hazards).
3. Umsetzung und Implementierung des Projektes

## Das Projektteam

Das Projekt Team setzt sich aus folgenden Personen zusammen:

- Tobias König
- Jessica Marban
- Basnakajev Timur Ali
- Patrick Neumann

## Die Hardware und Umsetzung

Die im Designdokument beschriebene Hardware wurde von der FH Campus Wien ausgeliehen.

Für die Messung der aktuellen Position beim Öffnen und Schließen festzustellen wurden zwei Zahrräder und einen Zahrriehmen aus eigenen Mitteln angekauft.

### Der Aufbau

In einem ersten Schritt wurde die Hardware laut Designdokument verdrahtet. Um den aktuelle Stellung beim Öffnen und Schließen des Fenster feststellen zu können mussten wir eine den Stellmotor mit einem Inkrementalgeber (ULN2003A Driver) verbunden werden. Beim Drehen des Motors werden die Impulse gezählt. Damit lässt sich die aktuelle Position (Fenster ist geschlossen, Fenster ist geöffnet) feststellen. Zusätzlich kann eine Sperre durch ein Hinderniss beim Öffnen und Schließen durch Feststellen eines "Schleppfehlers" fesgestellt werden.

## Programmierung des Adruino

### Software

Für die Programmierung des Adruino wurde die IDE auf Windows heruntergeladen und installiert:

https://www.arduino.cc/en/software 

#### Benötigte Libraries:

Zusätzlich werden folgende Bibliotheken benötigt.

- [MICS-VZ-89TE](https://https://github.com/HGrabas/MICS-VZ-89TE)
- Adruino BLE
- AES Lib
- Base64
- Crypto


Teile davon befindet sich auch in unserem GitHub Repository 
im RoomVentilation/Implementation/Arduino 

Sämtliche Software aus dem Git Repository muss mit Hilfe der IDE auf dem Arduino installiert und gebaut werden. Dazu muss der Arduino mit Hilfe eines Micro-USB Kabels angeschlossen werden. Dadurch wird der Arduino auch mit Strom versorgt. 

## Programmierung des Raspberry Pi

Auf dem Raspberry PI wurde wie empfohlen geflasht.
Danach muss SSH aktiviert werden und ein User mit einem Passwort eingerichtet werden.

Um die entsprechenden Devices richtig anzusprechen zu können werden folgende  Bibliotheken benötigt die installiert werden müssen.

- wiringPi
- gitlib
- gattlib
- openssl
- termio
- curl

Danach kann sämtliche Software aus dem Git Repository geladen werden und
mit folgendem Befehl das Hauptprogramm gebaut werden.

Build:

````
gcc -o main main.c motor_control.c led_control.c bluetooth_control_t.c action_control_t.c common.c mail_control.c fan_control.c crypto_control_t.c encoder_control.c -lwiringPi -lgattlib -lcurl -lcrypto
````

## Starten des Programms

Das Programm wird wie folgt gestartet:

>./main



