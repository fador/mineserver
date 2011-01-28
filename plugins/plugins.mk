include ../../config.mk

OBJS = $(SRCS:.cpp=.o)

all: $(OBJS) $(PLUGIN_NAME).so

install:
	cp $(PLUGIN_NAME).so ../../bin/$(PLUGIN_NAME).so

$(OBJS):
	$(CC) -fPIC $(INC) $(CFLAGS) $(ARCHFLAGS) $(LDFLAGS) -c $(@:.o=.cpp) -o ./$@

$(PLUGIN_NAME).so:
	$(CC) -shared $(CFLAGS) $(ARCHFLAGS) $(OBJS) -o $(PLUGIN_NAME).so


clean:
	-rm $(OBJS)
	-rm $(PLUGIN_NAME).so
