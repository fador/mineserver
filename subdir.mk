SUBCLEAN = $(addsuffix .clean,$(SUBDIRS))
SUBINSTALL = $(addsuffix .install,$(SUBDIRS))

.PHONY: clean all install $(SUBDIRS) $(SUBCLEAN) $(SUBINSTALL)

all: $(SUBDIRS)

clean: $(SUBCLEAN)

install: $(SUBINSTALL)

$(SUBCLEAN): %.clean:
	$(MAKE) -C $* clean

$(SUBINSTALL): %.install:
	$(MAKE) -C $* install

$(SUBDIRS):
	$(MAKE) -C $@
