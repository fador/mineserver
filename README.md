# Mineserver
*by Fador & Nredor and others*

More info on OFFICIAL MineServer :
https://github.com/fador/mineserver

This is a fork of the project aimed at allowing Python Plugins
for MineServer, lowering the bar for plugin devs.

General rules:

- Have python, Python.h and SWIG
 (sudo apt-get install python python-dev swig)

- "make" MineServer first
- Must run "make" in src/plugins/PyScript (any halp on other build systems?)

Usage:
In config.cfg add a new plugin, name it and point the file to a .py file you wish you use

API:
An example of callbacks are included in TriggTest.py which is bundled in src/plugins/PyScript/python
for a full list of functions see pydoc MineServer
