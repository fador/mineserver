# Mineserver
*by Fador & Nredor and others*

More info on OFFICIAL MineServer :
https://github.com/fador/mineserver

This is a fork of the project aimed at allowing Python Plugins
for MineServer, lowering the bar for plugin devs.

General rules:

- all Python Plugins must be in bin/plugins
- "make" MineServer first
- Must run "make" in src/plugins/PyScript (any halp on other build systems?)

Usage:
run mineserver and type "/load PyScript ./PyScript.so"
this will load all python plugins in bin/plugins

API:
Right now, the API and callbacks are rather few and far between, work is in progress
