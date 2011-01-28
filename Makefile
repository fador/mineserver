SUBDIRS = src plugins 

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

clean: TARGET := clean
clean:  $(SUBDIRS)

install: TARGET := install
install: $(SUBDIRS)

all: TARGET := all
all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(TARGET)

dist-tar:
	tar cvzf mineserver-latest-`uname`-bin.tar.gz ./bin/ ./LICENSE
	
dist-zip:
	zip -j mineserver-latest-`uname`-bin.zip ./bin/* ./LICENSE