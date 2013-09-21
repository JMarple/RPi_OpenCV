cmake_minimum_required(VERSION 2.8)

project( camcv )
SET(COMPILE_DEFINITIONS -Werror)

#OPENCV
find_package( OpenCV REQUIRED )

include_directories(/opt/vc/userland)
include_directories(/opt/vc/userland/interface/vcos/pthreads)
include_directories(/opt/vc/userland/interface/vmcs_host/linux)

add_executable(camcv src/RaspiCLI.c src/RaspiPreview.c src/RaspiCamControl.c utils/mycamera.c)
target_link_libraries(camcv /opt/vc/lib/libmmal_core.so /opt/vc/lib/libmmal_util.so /opt/vc/lib/libmmal_vc_client.so /opt/vc/lib/libvcos.so /opt/vc/lib/libbcm_host.so ${OpenCV_LIBS})

