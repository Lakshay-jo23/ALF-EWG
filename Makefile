CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -Isrc
SRCS     = src/main.cpp src/Sensors.cpp src/Nodes.cpp src/Alerts.cpp src/Persistence.cpp
OBJS     = $(SRCS:.cpp=.o)

alfewg: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: alfewg
	./alfewg

clean:
	rm -f $(OBJS) alfewg incidents.bin zone_report.txt
