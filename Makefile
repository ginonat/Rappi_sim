CXX = g++
CXXFLAGS = -Wall -Werror -pedantic -std=c++14
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

SRCDIR = src
OBJDIR = obj

SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

rappi_sim: $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o rappi_sim
