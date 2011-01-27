SUBDIRS = commands filelog cursesui 

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

clean: TARGET := clean
clean:  $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(TARGET)


