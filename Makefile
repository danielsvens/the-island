# ===== Project =====
BIN      := mygame
OBJDIR   := build

# ===== Build mode =====
MODE     ?= debug            # use: make MODE=release
CSTD     := -std=c23
CWARN    := -Wall -Wextra
CDEPS    := -MMD -MP         # header dependency files
CINC     := -Iinclude

ifeq ($(MODE),release)
  COPT := -O2
else
  COPT := -O0 -g
endif

# ===== Toolchain =====
CC       := gcc
CPPFLAGS := $(CINC)          # preprocessor flags (-I, -D)
CFLAGS   := $(CSTD) $(CWARN) $(CDEPS) $(COPT)
LDFLAGS  :=
LDLIBS   :=

# ===== Raylib via pkg-config (fallback if missing) =====
RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS   := $(shell pkg-config --libs   raylib 2>/dev/null)
CPPFLAGS += $(RAYLIB_CFLAGS)
LDLIBS   += $(if $(strip $(RAYLIB_LIBS)),$(RAYLIB_LIBS),-lraylib -lm -ldl -lpthread -lGL)

# ===== Sources / Objects (auto-discovered) =====
SRC  := $(shell find src -name '*.c')
OBJ  := $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC))
DEPS := $(OBJ:.o=.d)

# ===== Targets =====
.PHONY: all clean run bear help
all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@ $(LDLIBS)

# compile rule (auto-creates subdirs under build/)
$(OBJDIR)/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN) $(ARGS)

clean:
	rm -rf $(OBJDIR) $(BIN)

# Generate compile_commands.json with Bear (force rebuild)
bear: clean
	bear -- make -B

help:
	@echo "make [MODE=debug|release]        # build"
	@echo "make run [ARGS=...]              # run the game"
	@echo "make clean                       # remove build artifacts"
	@echo "make bear                        # regenerate compile_commands.json"

# include header deps (safe if files don't exist yet)
-include $(DEPS)

