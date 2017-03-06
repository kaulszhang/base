
# 提取宏定义
# argment1:     文件名
# argment2:	宏名称
# return:       宏定义的值

define get_macro_info
$(strip \
        $(shell PATH="$(PATH)" LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) $(CXX) -E $(addprefix -I,$(HEADER_DIRECTORYS)) -imacros $(1) -x c++ /dev/null -dM | \
		awk '$$2 == "$(2)" { gsub("\"","",$$3); print $$3; }') \
)
endef

# 判断宏是否定义
# argment1:     文件名
# argment2:	宏名称
# return:	有定义返回宏名称，否则返回空串

define macro_is_defined
$(strip \
        $(shell PATH="$(PATH)" LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) $(CXX) -E $(addprefix -I,$(HEADER_DIRECTORYS)) -imacros $(1) -x c++ /dev/null -dM | \
		awk '$$2 == "$(2)" { print $(2); }') \
)
endef

