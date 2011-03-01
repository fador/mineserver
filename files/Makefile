FILES = commands.cfg \
        config.cfg \
        item_alias.cfg \
        banned.txt \
        motd.txt \
        permissions.txt \
        roles.txt \
        rules.txt \
        whitelist.txt 

DIRS = recipes

INSTALL_DIR = ../bin/

.PHONY: clean all install

install: $(FILES)
	mkdir -p $(INSTALL_DIR)
	cp -n $(FILES) $(INSTALL_DIR)
	cp -n -r $(DIRS) $(INSTALL_DIR)
