
# Makefile
#

CIRCLEHOME = ../..

OBJS    = kernel.o main.o st7735s.o ds1307.o veml7700.o

LIBS    = $(CIRCLEHOME)/lib/libcircle.a

include ../Rules.mk

-include $(DEPS)
