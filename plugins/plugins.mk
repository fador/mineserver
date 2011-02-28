include ../../config.mk

OBJS = $(SRCS:.cpp=.o)

all: $(OBJS) $(PLUGIN_NAME).so

install: $(OBJS) $(PLUGIN_NAME).so
	mkdir -p ../../bin/
	cp $(PLUGIN_NAME).so ../../bin/$(PLUGIN_NAME).so

$(OBJS):
	$(CXX) -fPIC $(INC) $(CXXFLAGS) $(BUILDFLAGS) $(ARCHFLAGS) $(PLDFLAGS) $(LDFLAGS) -c $(@:.o=.cpp) -o ./$@

$(PLUGIN_NAME).so:
	$(CXX) -shared $(CXXFLAGS) $(BUILDFLAGS) $(ARCHFLAGS) $(PLDFLAGS) $(LDFLAGS) $(OBJS) -o $(PLUGIN_NAME).so


clean:
	-rm $(OBJS)
	-rm $(PLUGIN_NAME).so
