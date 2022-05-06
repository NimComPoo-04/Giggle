TARGET = giggle
TARGET_OS = unix

DIR = server # don't include test

LIB = pthread lua m
LIBDIR = lua-5.3.6/src

INCS = lua-5.3.6/src

BUILD = debug
BUILD_DIR = build

RM = rm -rf
