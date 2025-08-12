TARGET = a.out

CC = clang
SRCS = $(wildcard *.cpp **/*.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
DEPS = $(patsubst %.cpp, %.d, $(SRCS))

# CXXFLAGS += -Wall -g -I/usr/lib/llvm-18/include -std=c++17   -fno-exceptions -funwind-tables -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
LLVM_CONFIG = llvm-config --link-static
CPPFLAGS += $(shell $(LLVM_CONFIG) --cppflags)
CFLAGS   += $(shell $(LLVM_CONFIG) --cflags) -g
CXXFLAGS += $(shell $(LLVM_CONFIG) --cxxflags) -g -Wall
LDFLAGS  += $(shell $(LLVM_CONFIG) --ldflags)

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -rf **/*.o *.o **/*.d *.d

-include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -lLLVM -lstdc++ -o $@ $(OBJS)

# -MMD: ファイルの依存関係を表した%.dファイルを生成する
# -MP : ヘッダファイルに対してダミーの依存関係を作ることでMakefileのエラー防止
%.o: %.cpp
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@
