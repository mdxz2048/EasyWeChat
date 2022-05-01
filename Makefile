
#最终bin文件的名字，可以更改为自己需要的
BIN=easyWechat
#输出文件夹
OUTFILE= .
#可执行程序文件夹
OUTBIN= .
#连接目标文件夹
OBJFILE= .
#记住当前工程的根目录路径
ROOT_DIR=$(shell pwd)

#目标文件所在的目录
OBJS_DIR= .
#bin文件所在的目录
BIN_DIR= .

#获取当前目录下的c文件集，放在变量CUR_SOURCE中
CUR_SOURCE=${wildcard *.c}
#将对应的c文件名转为o文件后放在下面的CUR_OBJS变量中
CUR_OBJS=${patsubst %.c, %.o, $(CUR_SOURCE)}
#删除
RM=rm -rf

#APP_COMPILE_TIME编译时间
APP_COMPILE_DATE = $(shell date +"%Y-%m-%d")
APP_COMPILE_TIME = $(shell date +"%H:%M:%S")

## get all include path
CFLAGS  += $(foreach dir, $(INCLUDE_PATH), -I$(dir)) 
CFLAGS  += -g -pthread 
##传入编译时间和版本号
CFLAGS  += -DAPP_COMPILE_DATE=\"$(APP_COMPILE_DATE)\" -DAPP_COMPILE_TIME=\"$(APP_COMPILE_TIME)\"
	
#注意这里的顺序，需要先执行SUBDIRS最后才能是DEBUG
all:$(clean) $(CUR_OBJS)

#将c文件编译为o文件，并放在指定放置目标文件的目录中即OBJS_DIR
$(CUR_OBJS):%.o:%.c
	$(CC) -c $^ -o $(ROOT_DIR)/$(OBJS_DIR)/${BIN} $(CFLAGS)

clean:
	@$(RM) $(BIN_DIR)/${BIN}
	@rm $(OBJS_DIR)/*.o
inc:
	@echo $(INCLUDE_PATH)


