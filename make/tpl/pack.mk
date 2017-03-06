
LOCAL_TYPE			:= pack

## 该项目默认的配置

LOCAL_CONFIG			:= $(LOCAL_CONFIG)

## 生成目标名称（完整文件名称要根据配置PACKET_CONFIG、版本PACKET_VERSION增加前缀、后缀）

PACKET_TARGET			:=

## 该项目依赖的项目

PACKET_DEPENDS			:= \
				$(PACKET_DEPENDS)

## 该项目额外依赖的文件（相对于根目录ROOT_DIRECTORY）

PACKET_DEPEND_FILES		:= \
				$(PACKET_DEPEND_FILES)

## 项目版本定义文件

PACKET_VERSION_DEPEND		:=

## 该项目默认的配置类型

PACKET_CONFIG			:=
