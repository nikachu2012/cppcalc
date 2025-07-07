TARGET = a.out

CC = g++
SRCS = $(wildcard *.cpp **/*.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SRCS))
DEPS = $(patsubst %.cpp, %.d, $(SRCS))

CXXFLAGS += -Wall

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -rf **/*.o *.o **/*.d *.d

-include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) $(CPPFLAGS) -o $@ $(OBJS)

# -MMD: ファイルの依存関係を表した%.dファイルを生成する
# -MP : ヘッダファイルに対してダミーの依存関係を作ることでMakefileのエラー防止
%.o: %.cpp
	$(CC) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@
