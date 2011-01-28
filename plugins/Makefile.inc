CPP = g++

OBJS = $(SRCS:.cpp=.o)

all: $(OBJS) $(PLUGIN_NAME).so

install:
	cp $(PLUGIN_NAME).so ../../bin/$(PLUGIN_NAME).so

$(OBJS):
	$(CPP) -fPIC $(LDFLAGS) -c $(@:.o=.cpp) -o ./$@

$(PLUGIN_NAME).so:
	$(CPP) -shared $(OBJS) -o $(PLUGIN_NAME).so


clean:
	rm $(OBJS)
	rm $(PLUGIN_NAME).so
