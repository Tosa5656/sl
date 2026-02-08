# Simple Makefile for slpm
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

# Detect filesystem library
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CXXFLAGS += -lstdc++fs
endif

TARGET = slpm
SRCDIR = src
INCDIR = include
OBJDIR = obj

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(CXXFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

install: $(TARGET)
	@echo "Installing slpm to /usr/local/bin/"
	cp $(TARGET) /usr/local/bin/

uninstall:
	@echo "Uninstalling slpm from /usr/local/bin/"
	rm -f /usr/local/bin/$(TARGET)