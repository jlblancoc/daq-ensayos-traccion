cmake_minimum_required(VERSION 3.1)
project(EnsayosTraccionUAL)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package(MRPT QUIET COMPONENTS hwdrivers)  # Find MRPT; needed for static libs, and also to forward that path to the external project

find_package(Qt5Widgets QUIET)  # Find the QtWidgets library
find_package(Qt5PrintSupport QUIET)

IF (NOT Qt5Widgets_FOUND)
	MESSAGE(STATUS "*WARNING* Qt5 not found. Cannot build this project. \n  Add the installation prefix of Qt5Widgets to CMAKE_PREFIX_PATH \n  or set Qt5Widgets_DIR to a directory containing one of the above files, typically: 'C:\\Qt\\5.X\\msvcXXXX_64\\lib\\cmake\\Qt5Widgets'")
ELSE()

	include(ExternalProject)

	# Search for "arduinodaq":
	ExternalProject_Add(arduino_daq_external
		PREFIX 3rdparty
#		GIT_REPOSITORY "https://github.com/ual-arm-ros-pkg/arduino-daq-ros-pkg.git"
#		GIT_TAG "master"
		URL "D:/code/arduino-daq-ros-pkg"
		CMAKE_ARGS 
		"-DBUILD_ROS=OFF" 
		"-DBUILD_STANDALONE_LIB=ON"
		"-DMRPT_DIR=${MRPT_DIR}"
		INSTALL_COMMAND ""
	)	

	# Define project:
	set(FORMS
		EnsayosTraccionUAL/mainwindow.ui
	)
	qt5_wrap_ui(FORMS_HEADERS ${FORMS})

	set(RESOURCES
		EnsayosTraccionUAL/${PROJECT_NAME}.qrc
	)
	qt5_add_resources(RESOURCES_RCC ${RESOURCES})

	link_directories("${CMAKE_BINARY_DIR}/3rdparty/src/arduino_daq_external-build/libarduinodaq")
	include_directories("${CMAKE_BINARY_DIR}/3rdparty/src/arduino_daq_external/include")
	include_directories("${CMAKE_BINARY_DIR}/3rdparty/src/arduino_daq_external/arduino_firmware/arduino_daq/arduino_daq")
	
	# Tell CMake to create the helloworld executable
	add_executable(${PROJECT_NAME} WIN32 
		EnsayosTraccionUAL/main.cpp
		EnsayosTraccionUAL/mainwindow.cpp 
		EnsayosTraccionUAL/mainwindow.h
		EnsayosTraccionUAL/qcustomplot.cpp
		EnsayosTraccionUAL/qcustomplot.h
		${FORMS_HEADERS}
		${RESOURCES_RCC}
		)

	# Use the Widgets module from Qt 5.
	target_link_libraries(${PROJECT_NAME}
		Qt5::Widgets
		Qt5::PrintSupport
		${MRPT_LIBS}
		arduinodaq
		)

	add_dependencies(${PROJECT_NAME} arduino_daq_external)

ENDIF() # if qt not found
