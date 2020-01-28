CMAKE_MINIMUM_REQUIRED(VERSION 3.4.0)

PROJECT(Registration)

CMAKE_POLICY(SET CMP0020 NEW)
CMAKE_POLICY(SET CMP0028 NEW)
CMAKE_POLICY(SET CMP0053 NEW)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

#---设置自动生成moc文件,不可缺少---------
SET(CMAKE_AUTOMOC ON)

#------------------------------------------
FIND_PACKAGE(Qt5Widgets)
FIND_PACKAGE(Qt5Core)
FIND_PACKAGE(Qt5Gui)
FIND_PACKAGE(Qt5SerialPort)
FIND_PACKAGE(Qt5Xml)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

#--------查找当前文件中的所有源码----------

FILE(GLOB SRC_FILES "./*.cpp")
FILE(GLOB HEAD_FILES "./*.h")
FILE(GLOB UI_FILES "./*.ui")


SET(PROJECT_RESOURCE Registration.qrc)

#--------生成ui文件的头文件----------------
QT5_WRAP_UI(WRAP_FILES ${UI_FILES})
QT5_WRAP_CPP(WRAP_SRCS_H ${HEARD_FILES})
QT5_ADD_RESOURCES(PROJECT_RESOURCE_RCC ${PROJECT_RESOURCE})


#--------创建工程文件-----------------------
ADD_EXECUTABLE(${PROJECT_NAME}
               ${HEAD_FILES}
               ${SRC_FILES} 
			   
               ${UI_FILES}

			   ${WRAP_FILES}
               ${WRAP_SRCS_H}
			   ${PROJECT_RESOURCE_RCC}
			   ${PROJECT_RESOURCE}
               )

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
                      Qt5::Widgets
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Xml
                      ${VTK_LIBRARIES}) 
					  
					  
#---------添加筛选器----------------				  
source_group("Forms" FILES ${UI_FILES})
source_group("Forms\\WRAP_UI" FILES ${WRAP_FILES})
