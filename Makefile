SUBDIRS = src plugins 

include subdir.mk

dist-tar:
	tar cvzf mineserver-latest-`uname`-bin.tar.gz ./bin/ ./LICENSE
	
dist-zip:
	zip -j mineserver-latest-`uname`-bin.zip ./bin/* ./LICENSE
  
dist: dist-tar dist-zip
