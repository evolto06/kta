# Makefile for kta: thin wrapper around MSBuild (the SFML 3 libs are MSVC x64 builds,
# so MinGW g++ cannot link them).
#
#   make            build Debug x64
#   make release    build Release x64
#   make run        build and run Debug
#   make run-release
#   make rebuild    clean + build Debug
#   make clean      remove Debug and Release build output
#
# Override the MSBuild path or platform if needed:
#   make MSBUILD="D:/VS/MSBuild/Current/Bin/MSBuild.exe" PLATFORM=x64
# SFML's bin folder (e.g. C:\libraries\SFML-3.0.0\bin) must be on PATH for `run`.

MSBUILD  ?= C:/Program Files/Microsoft Visual Studio/18/Community/MSBuild/Current/Bin/MSBuild.exe
PROJECT  := kta.vcxproj
PLATFORM ?= x64
FLAGS    := -m -nologo -v:minimal

.PHONY: all debug release run run-release rebuild clean

all: debug

debug:
	"$(MSBUILD)" $(PROJECT) -p:Configuration=Debug -p:Platform=$(PLATFORM) $(FLAGS)

release:
	"$(MSBUILD)" $(PROJECT) -p:Configuration=Release -p:Platform=$(PLATFORM) $(FLAGS)

# Keep these paths unquoted: under MSYS/Git Bash sh, a recipe line that ends in a
# double quote is mis-parsed ("unexpected EOF while looking for matching `"'").
run: debug
	Build/$(PLATFORM)/Debug/bin/kta.exe

run-release: release
	Build/$(PLATFORM)/Release/bin/kta.exe

rebuild:
	"$(MSBUILD)" $(PROJECT) -t:Rebuild -p:Configuration=Debug -p:Platform=$(PLATFORM) $(FLAGS)

clean:
	"$(MSBUILD)" $(PROJECT) -t:Clean -p:Configuration=Debug -p:Platform=$(PLATFORM) $(FLAGS)
	"$(MSBUILD)" $(PROJECT) -t:Clean -p:Configuration=Release -p:Platform=$(PLATFORM) $(FLAGS)
